from fastapi import FastAPI, Request
from fastapi.responses import RedirectResponse
from starlette.middleware.base import BaseHTTPMiddleware
from app.database import engine
from app.models import Base
from app.routers import health, dashboard, ota, commands, summary
from app.auth import router as auth_router, SESSIONS
import aiofiles

# Ensure all DB tables exist at startup
Base.metadata.create_all(bind=engine)


app = FastAPI(title="Spatika Health API v3.0")

class AuthMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        # 1. Skip auth for APIs: /health, /trg_gprs, /tws_gprs (if handled differently)
        # But wait, these API routes are usually just public without auth for the IoT devices.
        # We need to protect UI routes: /, /dashboard, /station, /cmd, /ota, /delete
        
        # Determine if path is protected UI route
        protected_prefixes = ("/dashboard", "/station", "/cmd", "/ota", "/delete", "/clear-queue", "/clear-ota-queue", "/toggle-ota-lock", "/summary", "/csv")
        
        # Skip auth for builds (so ESP32 can download binaries)
        if request.url.path.startswith("/builds"):
            return await call_next(request)

        # Is the requested URL one of the protected ones?
        is_protected = any(request.url.path.startswith(p) for p in protected_prefixes)
        
        # 2. Get User from Session
        import time
        session_id = request.cookies.get("session_id")
        user = SESSIONS.get(session_id)
        
        if user and user.get("expires_at", 0) < time.time():
            if session_id in SESSIONS:
                del SESSIONS[session_id]
            user = None
        
        # Inject into state so Jinja templates can use it via request.state.user
        request.state.user = user

        # 3. Secure Route
        if is_protected and not user:
            return RedirectResponse("/login")
            
        # 4. Supervisor Only Actions (Delete, Edit, OTA locks)
        supervisor_prefixes = ("/delete", "/clear-queue", "/clear-ota-queue", "/toggle-ota-lock", "/cmd", "/station/*/ota")
        # Crude check for destructive action
        is_supervisor_action = any(request.url.path.startswith(p) for p in ("/delete", "/clear-queue", "/clear-ota-queue", "/toggle-ota-lock", "/cmd"))
        if request.url.path.endswith("/ota") and "station" in request.url.path:
            is_supervisor_action = True
            
        if is_supervisor_action and user and user["role"] != "supervisor":
            from fastapi.responses import HTMLResponse
            return HTMLResponse("<h1>403 Forbidden</h1><p>Supervisor privileges required.</p>", status_code=403)

        response = await call_next(request)
        return response

app.add_middleware(AuthMiddleware)

# Register all modular routers
app.include_router(auth_router)
app.include_router(health.router)
app.include_router(dashboard.router)
app.include_router(ota.router)
app.include_router(commands.router)
app.include_router(summary.router)

# Serve the firmware builds directory via a Range-aware endpoint instead of StaticFiles
import os
import mimetypes
from fastapi.responses import Response, StreamingResponse

os.makedirs("/app/builds", exist_ok=True)

@app.api_route("/builds/{filename}", methods=["GET", "HEAD"])
async def serve_firmware(filename: str, request: Request):
    """Range-aware firmware file server for ESP32 OTA downloads."""
    # Security: only serve .bin files, no path traversal
    if not filename.endswith(".bin") or "/" in filename or ".." in filename:
        from fastapi import Response
        return Response(status_code=403)

    filepath = os.path.realpath(os.path.join(BUILDS_DIR, filename))
    if not filepath.startswith(os.path.realpath(BUILDS_DIR)):
        from fastapi import Response
        return Response(status_code=403)
    if not os.path.exists(filepath):
        from fastapi import Response
        return Response(status_code=404)

    file_size = os.path.getsize(filepath)
    if request.method == "HEAD":
        from fastapi import Response
        return Response(
            status_code=200,
            headers={
                "Content-Length": str(file_size),
                "Accept-Ranges": "bytes",
                "Content-Type": "application/octet-stream",
            }
        )

    range_header = request.headers.get("Range")

    if range_header:
        # Parse "bytes=start-end"
        try:
            range_val = range_header.replace("bytes=", "")
            start_str, end_str = range_val.split("-")
            start = int(start_str)
            end = int(end_str) if end_str else file_size - 1
        except Exception:
            return Response(status_code=400)

        end = min(end, file_size - 1)
        chunk_size = end - start + 1

        async def iter_file():
            async with aiofiles.open(filepath, "rb") as f:
                await f.seek(start)
                remaining = chunk_size
                while remaining > 0:
                    import asyncio
                    data = await f.read(min(2048, remaining)) # Pull small 2KB chunks
                    if not data:
                        break
                    remaining -= len(data)
                    yield data
                    await asyncio.sleep(0.5) # Force 500ms delay per 2KB = 4KB/s throttle!

        return StreamingResponse(
            iter_file(),
            status_code=206,
            headers={
                "Content-Range": f"bytes {start}-{end}/{file_size}",
                "Content-Length": str(chunk_size),
                "Accept-Ranges": "bytes",
                "Content-Type": "application/octet-stream",
            }
        )
    else:
        # Full file request (e.g. for size check via HEAD/GET)
        async def iter_full():
            async with aiofiles.open(filepath, "rb") as f:
                while True:
                    import asyncio
                    data = await f.read(2048)
                    if not data:
                        break
                    yield data
                    await asyncio.sleep(0.5)

        return StreamingResponse(
            iter_full(),
            status_code=200,
            headers={
                "Content-Length": str(file_size),
                "Accept-Ranges": "bytes",
                "Content-Type": "application/octet-stream",
            }
        )


@app.get("/")
async def root():
    from fastapi.responses import RedirectResponse
    return RedirectResponse(url="/dashboard")
