
def check_braces(filename):
    level = 0
    with open(filename, 'r') as f:
        for i, line in enumerate(f, 1):
            if i > 90: break
            for char in line:
                if char == '{':
                    level += 1
                elif char == '}':
                    level -= 1
            print(f"[{filename}] Line {i} Level: {level}")

check_braces('gprs_ftp.ino')
