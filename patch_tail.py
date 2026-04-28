import re

with open('gprs_ftp.ino', 'r') as f:
    content = f.read()

replacement = """
    if (SerialSIT.available() > 0) {
        char peek = SerialSIT.peek();
        if (peek == '+' || peek == 'S' || peek == 'e' || SerialSIT.available() > 20) {
            int avail = SerialSIT.available();
            char tail_buf[64] = {0};
            int i = 0;
            while(SerialSIT.available() && i < 63) {
                tail_buf[i++] = SerialSIT.read();
            }
            tail_buf[i] = '\\0';
            
            // Clean non-printable chars for safe logging
            for(int j=0; j<i; j++) {
                if(tail_buf[j] < 32 && tail_buf[j] != '\\r' && tail_buf[j] != '\\n') tail_buf[j] = '.';
            }
            
            Serial.printf("[OTA] [ERR] Tail-Guard triggered! Data: %d bytes.\\n[OTA] Dump: %s\\n", 
                          avail, tail_buf);
            
            if (peek == 'S') { 
               Serial.println("[OTA] Network Drop Detected. Forcing Bearer Reset.");
               session_terminated = true; 
            }
            flushSerialSIT();
            chunk_retries++;
            consecutive_fails++;
            continue;
        }
    }
"""

# Find the tail guard section and replace it
pattern = r"if \(SerialSIT\.available\(\) > 0\) \{.*?char peek = SerialSIT\.peek\(\);.*?continue;\n        \}\n    \}"
content = re.sub(pattern, replacement.strip(), content, flags=re.DOTALL)

with open('gprs_ftp.ino', 'w') as f:
    f.write(content)
