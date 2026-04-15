import os

files = [f for f in os.listdir('.') if f.endswith('.ino') or f.endswith('.cpp') or f.endswith('.h')]
files.sort()

total_open = 0
total_close = 0

for filename in files:
    with open(filename, 'r') as f:
        content = f.read()
        open_b = content.count('{')
        close_b = content.count('}')
        if open_b != close_b:
            print(f"{filename}: {{={open_b}, }}={close_b} DIFF={open_b - close_b}")
        total_open += open_b
        total_close += close_b

print(f"TOTAL: {{={total_open}, }}={total_close} DIFF={total_open - total_close}")
