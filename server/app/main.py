from fastapi import FastAPI, Request
from fastapi.responses import RedirectResponse
from fastapi.staticfiles import StaticFiles
from starlette.middleware.base import BaseHTTPMiddleware
from app.database import engine
from app.models import Base
from app.routers import health, dashboard, ota, commands, summary, diagnostics
from app.auth import router as auth_router, SESSIONS

# Ensure all DB tables exist at startup
Base.metadata.create_all(bind=engine)

app = FastAPI(title="Spatika Health API v3.0")

class AuthMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        # 1. Skip auth for APIs: /health, /trg_gprs, /tws_gprs (if handled differently)
        # But wait, these API routes are usually just public without auth for the IoT devices.
        # We need to protect UI routes: /, /dashboard, /station, /cmd, /ota, /delete
        
        # Determine if path is protected UI route
        protected_prefixes = ("/dashboard", "/station", "/cmd", "/ota", "/delete", "/clear-queue", "/clear-ota-queue", "/toggle-ota-lock", "/summary", "/diagnostics", "/csv")
        
        # Skip auth for builds (so ESP32 can download binaries)
        if request.url.path.startswith("/builds"):
            return await call_next(request)

        # Is the requested URL one of the protected ones?
        is_protected = any(request.url.path.startswith(p) for p in protected_prefixes)
        
        # 2. Get User from Session
        session_id = request.cookies.get("session_id")
        user = SESSIONS.get(session_id)
        
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
app.include_router(diagnostics.router)

# Serve the firmware builds directory statically
# This makes binaries available at http://server-ip/builds/FW_S6_SPATIKA_GEN.bin
import os
os.makedirs("/app/builds", exist_ok=True)
app.mount("/builds", StaticFiles(directory="/app/builds"), name="builds")


@app.get("/")
async def root():
    from fastapi.responses import RedirectResponse
    return RedirectResponse(url="/dashboard")
