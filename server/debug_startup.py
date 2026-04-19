import sys
import traceback
import os

print("--- STATION STARTUP DEBUGGER ---")
print(f"CWD: {os.getcwd()}")
# Ensure the current directory is in the path so it can find the 'app' folder
sys.path.append(os.getcwd())

try:
    print("Testing 'app' package import...")
    import app
    print("SUCCESS: 'app' package found.")
    
    print("Testing 'app.main' import (The most common crash point)...")
    from app import main
    print("SUCCESS: app.main loaded without crash.")
    
except Exception as e:
    print("\n!!! CRASH DETECTED !!!")
    print(f"Error Type: {type(e).__name__}")
    print(f"Error Message: {e}")
    print("\nFull Traceback:")
    traceback.print_exc()
    sys.exit(1)

print("\nAll systems nominal. If the container is still restarting, it is a Docker issue, not a Python issue.")
