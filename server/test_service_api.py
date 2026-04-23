import requests
import base64
import json
import time

# Simulation of v5.87 Service Report
SERVER_URL = "http://localhost:8000/api/v2/service_report"

# Dummy image: Red dot
dummy_b64 = "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAABAAEDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqjpKWmp6ipqr1qzYS91uLu3u7u7t7v5+fr7/xAAfAQADAQEBAQEBAQEBAAAAAAAAAQCAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElIUVVWV1hZWmNkZWZnaGlqc3R1dnd4eXq0hYaHiImKkpOUlZaXmJmqsrO0obX2l34l5tkfX256onl8eY2dkhifn5+njs7W19vh4e/ieH17fX19f7a1ubX19f321n5+mf/aAAwDAQACEBase64"

payload = {
    "stn": "9000",
    "com": "Verified solar panel cleaning. Voltage normalized to 13.8V.",
    "ts": int(time.time()),
    "img1": dummy_b64,
    "img2": None
}

print(f"Sending test report to {SERVER_URL}...")
try:
    response = requests.post(SERVER_URL, json=payload)
    print(f"Status: {response.status_code}")
    print(f"Response: {response.text}")
except Exception as e:
    print(f"Error: {e}")
