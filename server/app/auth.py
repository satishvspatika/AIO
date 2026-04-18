from fastapi import APIRouter, Request, Response, Form, Depends
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from app.services.health_eval import ist_filter
import secrets

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")
templates.env.filters["ist"] = ist_filter

import os
import sys

ADMIN_PASS = os.getenv("ADMIN_PASS", "")
GUEST_PASS = os.getenv("GUEST_PASS", "")

if not ADMIN_PASS or not GUEST_PASS:
    print("FATAL ERROR: ADMIN_PASS and GUEST_PASS must be strictly defined in the .env file.")
    print("Refusing to start server with empty passwords. This prevents the '1-Click' 0-day bypass.")
    sys.exit(1)

USERS = {
    os.getenv("ADMIN_USER", "admin"): {
        "password": ADMIN_PASS, 
        "role": "supervisor"
    },
    os.getenv("GUEST_USER", "guest"): {
        "password": GUEST_PASS, 
        "role": "normal"
    }
}

import sqlite3
import json

class SqliteSessionStore:
    def __init__(self, db_path="app/SpatikaSessions.db"):
        self.db_path = db_path
        try:
            os.makedirs(os.path.dirname(db_path), exist_ok=True)
            with sqlite3.connect(self.db_path, timeout=5.0) as conn:
                conn.execute('''
                    CREATE TABLE IF NOT EXISTS sessions (
                        session_id TEXT PRIMARY KEY,
                        data TEXT
                    )
                ''')
        except Exception as e:
            print(f"[FATAL] Session store init failed: {e}. Falling back to memory.")
            self.db_path = ":memory:"
            with sqlite3.connect(self.db_path, timeout=5.0) as conn:
                conn.execute('''
                    CREATE TABLE IF NOT EXISTS sessions (
                        session_id TEXT PRIMARY KEY,
                        data TEXT
                    )
                ''')

    def purge_expired(self):
        with sqlite3.connect(self.db_path, timeout=5.0) as conn:
            conn.execute("DELETE FROM sessions WHERE json_extract(data, '$.expires_at') < ?", (time.time(),))
    
    def __setitem__(self, key, value):
        with sqlite3.connect(self.db_path, timeout=5.0) as conn:
            conn.execute("INSERT OR REPLACE INTO sessions (session_id, data) VALUES (?, ?)", (key, json.dumps(value)))
            
    def get(self, key, default=None):
        with sqlite3.connect(self.db_path, timeout=5.0) as conn:
            cur = conn.execute("SELECT data FROM sessions WHERE session_id = ?", (key,))
            row = cur.fetchone()
            return json.loads(row[0]) if row else default

    def __contains__(self, key):
        return self.get(key) is not None

    def __delitem__(self, key):
        with sqlite3.connect(self.db_path, timeout=5.0) as conn:
            conn.execute("DELETE FROM sessions WHERE session_id = ?", (key,))
            
    def items(self):
        with sqlite3.connect(self.db_path, timeout=5.0) as conn:
            cur = conn.execute("SELECT session_id, data FROM sessions")
            return [(row[0], json.loads(row[1])) for row in cur.fetchall()]

# Phase 7 Fix: Drop fragile Python-Memory Dict for Multi-Worker SQLite Auth Store
SESSIONS = SqliteSessionStore()

import time

@router.get("/login", response_class=HTMLResponse)
def login_page(request: Request):
    return templates.TemplateResponse(request=request, name="login.html", context= {"request": request})

@router.post("/login")
def login_submit(request: Request, username: str = Form(...), password: str = Form(...)):
    if username in USERS and USERS[username]["password"] == password:
        session_id = secrets.token_hex(16)
        
        # OOM Memory Fix: Reaper to remove expired sessions natively via SQLite
        now = time.time()
        SESSIONS.purge_expired()
            
        # 7-day expiry
        SESSIONS[session_id] = {
            "username": username, 
            "role": USERS[username]["role"],
            "expires_at": now + (86400 * 7)
        }
        
        resp = RedirectResponse(url="/dashboard", status_code=302)
        # Phase 7 Fix: Stop CSRF logic exploits on supervisor POST commands
        resp.set_cookie(key="session_id", value=session_id, max_age=86400 * 7, httponly=True, samesite="strict")
        return resp
    
    return templates.TemplateResponse(request=request, name="login.html", context= {
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
