from fastapi import APIRouter, Request, HTTPException
import logging
from Crypto.Cipher import AES
import base64
from app.database import engine
from app.models import HealthReport
from sqlalchemy.orm import Session
import os

router = APIRouter()

AES_KEY = b"SpatikaDemoAIO09"

def unpad(s):
    return s[:-ord(s[len(s)-1:])]

@router.post("/demo_ksndmc")
async def demo_ksndmc(request: Request):
    try:
        # Standard x-www-form-urlencoded
        form_data = await request.form()
        encrypted_b64 = form_data.get("payload")
        
        if not encrypted_b64:
            print("[DEMO] [ERROR] No payload found in request")
            raise HTTPException(status_code=400, detail="Missing payload")

        print("--------------------------------------------------")
        print(f"[DEMO] Received (AES 128 Encrypted): {encrypted_b64}")
        
        # Decode Base64
        encrypted_data = base64.b64decode(encrypted_b64)
        
        # Decrypt AES-128-ECB
        cipher = AES.new(AES_KEY, AES.MODE_ECB)
        decrypted_padded = cipher.decrypt(encrypted_data)
        decrypted_text = unpad(decrypted_padded).decode('utf-8')
        
        print(f"[DEMO] AES 128 Decrypted: {decrypted_text}")
        print("--------------------------------------------------")
        
        # v5.85: Persistence - Append to local file for verification
        try:
            with open("demo_records.txt", "a") as f:
                f.write(f"{decrypted_text}\n")
            print(f"[DEMO] Record Saved to: {os.getcwd()}/demo_records.txt")
        except Exception as fe:
            print(f"[DEMO] [ERROR] Failed to write to demo_records.txt: {str(fe)}")

        # Return the decrypted text for device confirmation
        return {"status": "success", "decrypted": decrypted_text}
        
    except Exception as e:
        print(f"[DEMO] [ERROR] Decryption failed: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Decryption error: {str(e)}")
