import urllib.request
import re

url = "http://rtdas.spatika.net/update/FW_S1_SPATIKA_GEN.bin"
try:
    req = urllib.request.Request(url, method='HEAD')
    with urllib.request.urlopen(req) as response:
        content_length = response.headers.get('Content-Length')
        print(f"Content length: {content_length}")
except Exception as e:
    print(f"Error: {e}")
