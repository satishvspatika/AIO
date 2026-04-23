import os
from fastapi.templating import Jinja2Templates
from app.services.health_eval import ist_filter

# Centralized Template Engine to prevent Jinja2 cache collisions and signature mismatches
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
# Try /app/app/templates or /app/templates
TEMPLATE_DIR = os.path.join(BASE_DIR, "templates")
if not os.path.exists(TEMPLATE_DIR):
    # Fallback to parent dir's templates if we are inside a routers/ or services/ folder
    TEMPLATE_DIR = os.path.join(os.path.dirname(BASE_DIR), "templates")

if not os.path.exists(TEMPLATE_DIR):
    # Absolute default fallback
    TEMPLATE_DIR = "/app/app/templates"

# print(f"DEBUG: Using templates from {TEMPLATE_DIR}")
templates = Jinja2Templates(directory=TEMPLATE_DIR)

# Register common filters and globals here once
templates.env.filters["ist"] = ist_filter
import datetime
templates.env.globals["datetime"] = datetime.datetime
