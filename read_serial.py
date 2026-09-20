import serial
import time
import sys

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/cu.usbserial-A5069RR4'
baud = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
duration = int(sys.argv[3]) if len(sys.argv) > 3 else 600

print(f"--- Monitoring {port} at {baud} baud for {duration} seconds ---", flush=True)

start_time = time.time()
ser = None

if len(sys.argv) > 4 and sys.argv[4] == 'reset':
    try:
        s = serial.Serial()
        s.port = port
        s.baudrate = baud
        s.dtr = False
        s.rts = False
        s.open()
        print("[RESET] Toggling DTR/RTS to pulse ESP32 EN line...", flush=True)
        s.dtr = False
        s.rts = True
        time.sleep(0.1)
        s.rts = False
        s.dtr = False
        time.sleep(0.3)
        s.close()
    except Exception as e:
        print(f"Reset error: {e}", flush=True)

while time.time() - start_time < duration:
    try:
        if ser is None or not ser.is_open:
            ser = serial.Serial()
            ser.port = port
            ser.baudrate = baud
            ser.dtr = False
            ser.rts = False
            ser.dsrdtr = False
            ser.rtscts = False
            ser.open()
            ser.dtr = False
            ser.rts = False
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='replace').rstrip()
            if line:
                print(line, flush=True)
        else:
            time.sleep(0.05)
    except (serial.SerialException, OSError) as e:
        if ser:
            try:
                ser.close()
            except Exception:
                pass
            ser = None
        time.sleep(0.5)
    except Exception as e:
        print(f"Serial Monitor Exception: {e}", flush=True)
        time.sleep(0.5)

if ser and ser.is_open:
    ser.close()
print("--- Serial monitoring complete ---", flush=True)
