from fastapi import APIRouter, Request, Response, Form, Depends
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
import secrets

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")

USERS = {
    "satishv": {"password": "friend", "role": "supervisor"},
    "spatika": {"password": "sit#2001", "role": "normal"}
}

SESSIONS = {}

@router.get("/login", response_class=HTMLResponse)
def login_page(request: Request):
    return templates.TemplateResponse("login.html", {"request": request})

@router.post("/login")
def login_submit(request: Request, username: str = Form(...), password: str = Form(...)):
    if username in USERS and USERS[username]["password"] == password:
        session_id = secrets.token_hex(16)
        SESSIONS[session_id] = {"username": username, "role": USERS[username]["role"]}
        
        resp = RedirectResponse(url="/dashboard", status_code=302)
        resp.set_cookie(key="session_id", value=session_id, max_age=86400 * 7, httponly=True)
        return resp
    
    return templates.TemplateResponse("login.html", {
        "request": request, 
        "error": "Invalid username or password"
    })

@router.get("/logout")
def logout(request: Request):
    session_id = request.cookies.get("session_id")
    if session_id in SESSIONS:
        del SESSIONS[session_id]
        
    resp = RedirectResponse(url="/login", status_code=302)
    resp.delete_cookie("session_id")
    return resp
