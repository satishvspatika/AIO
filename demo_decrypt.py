import base64
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad
import sys

# The 16-byte key configured in user_config.h
DEMO_KEY = b"1234567890123456"

def decrypt_ksndmc_payload(encrypted_b64_string: str) -> str:
    try:
        # Decode Base64 back into raw encrypted bytes
        encrypted_bytes = base64.b64decode(encrypted_b64_string)
        
        # Initialize AES ECB Cipher
        cipher = AES.new(DEMO_KEY, AES.MODE_ECB)
        
        # Decrypt bytes
        decrypted_padded_bytes = cipher.decrypt(encrypted_bytes)
        
        # Remove PKCS#7 Padding
        decrypted_bytes = unpad(decrypted_padded_bytes, AES.block_size, style='pkcs7')
        
        # Decode bytes to normal UTF-8 string
        return decrypted_bytes.decode('utf-8')
        
    except Exception as e:
        return f"Decryption Failed: {str(e)}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python demo_decrypt.py '<BASE64_PAYLOAD>'")
        sys.exit(1)
        
    payload = sys.argv[1]
    
    # Strip 'payload=' if the user accidentally copied it
    if payload.startswith("payload="):
        payload = payload[8:]
        
    recovered_string = decrypt_ksndmc_payload(payload)
    print(f"\n--- SUCCESS: FastAPI Server Recovers ---")
    print(f"Plaintext Payload: {recovered_string}\n")
