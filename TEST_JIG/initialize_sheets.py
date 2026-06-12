import sys
import json
import urllib.request

def main():
    print("==================================================")
    print("        Spatika Google Sheet Initializer          ")
    print("==================================================")
    print("This script will initialize or reset headers in your Google Sheet")
    print("by sending a signal to your Google Apps Script Web App.")
    print("==================================================")
    
    url = input("Please enter your Google Apps Script Web App URL: ").strip()
    if not url:
        print("Error: URL cannot be empty.")
        sys.exit(1)
        
    print(f"\nSending initialization request to: {url}")
    
    payload = json.dumps({"initOnly": True}).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=payload,
        headers={"Content-Type": "application/json"},
        method="POST"
    )
    
    try:
        # Google Web App handles redirection, urllib handles it automatically
        with urllib.request.urlopen(req) as response:
            res_body = response.read().decode("utf-8")
            # Deployed Apps Script returns the response body (redirect resolution is automatic)
            print(f"Response: {res_body}")
            print("\n✓ Google Sheet initialization command sent successfully!")
            print("Please open your Google Sheet to verify that the 'Test_Logs' and 'Master_Status'")
            print("tabs are present and formatted correctly.")
    except Exception as e:
        print(f"\nError sending request: {e}")
        print("Please check that the Web App URL is correct, deployed as 'Anyone', and has access permissions.")

if __name__ == "__main__":
    main()
