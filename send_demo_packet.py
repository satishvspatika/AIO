import base64
import requests
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

# Configuration
AES_KEY = b"1234567890123456"
TARGET_URL = "https://account-surround-dreamlike.ngrok-free.dev/demo_ksndmc"

# Dummy Payload matching AIO9 format
payload_data = "stn_id=001952&rec_time=2026-04-18,15:15&rainfall=00.00&signal=-073&key=pse2420&bat_volt=04.1&bat_volt1=04.1"

def encrypt_payload(data):
    # AES-128-ECB PKCS7 Padding
    cipher = AES.new(AES_KEY, AES.MODE_ECB)
    padded_data = pad(data.encode('utf-8'), AES.block_size, style='pkcs7')
    encrypted_bytes = cipher.encrypt(padded_data)
    encoded_str = base64.b64encode(encrypted_bytes).decode('utf-8')
    return encoded_str

def main():
    print("="*60)
    print("🚀 AIO9 SECURE TELEMETRY DUMMY TEST")
    print("="*60)
    print(f"Original Data : {payload_data}")
    
    encrypted_payload = encrypt_payload(payload_data)
    print(f"Encrypted (B64): {encrypted_payload}")
    print(f"Target URL    : {TARGET_URL}")
    print("-" * 60)
    
    # Send POST request exactly like the ESP32
    # Form-data format: payload=<base64_data>
    try:
        response = requests.post(
            TARGET_URL, 
            data=f"payload={encrypted_payload}",
            headers={"Content-Type": "application/x-www-form-urlencoded"}
        )
        print(f"Status Code   : {response.status_code}")
        print(f"Server Resp   : {response.text}")
        print("="*60)
        if response.status_code == 200:
            print("✅ SUCCESS: Packet received and acknowledged by Contabo!")
        else:
            print("❌ FAILED: Server returned an error.")
    except Exception as e:
        print(f"❌ ERROR: Could not connect to Ngrok: {e}")

if __name__ == "__main__":
    main()
