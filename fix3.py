with open("scheduler.ino", "r") as f:
    lines = f.read().split('\n')

state = -1
count = 0
for i in range(len(lines)):
    line = lines[i]
    
    if '#if SYSTEM == 1' in line or '#elif SYSTEM == 1' in line:
        state = 1
    elif '#if SYSTEM == 2' in line or '#elif SYSTEM == 2' in line:
        state = 2
    elif '#if' in line and 'SYSTEM == 1 || SYSTEM == 2' in line:
        state = 1
    elif '#if SYSTEM == 0 || SYSTEM == 2' in line:
        state = 0 # Safe limit
    elif '#if SYSTEM == 0' in line or '#elif SYSTEM == 0' in line:
        state = 0
        
    if '#else' in line or '#endif' in line:
        state = -1

    if state in [1, 2]:
        if '%05.2f\\r\\n' in line:
            old = line
            line = line.replace('%05.2f\\r\\n', '%04.1f\\r\\n')
            if old != line:
                lines[i] = line
                count += 1
                print(f"Replaced Line {i+1}: {line.strip()}")
        if '%05.2f\\n' in line:
            old = line
            line = line.replace('%05.2f\\n', '%04.1f\\n')
            if old != line:
                lines[i] = line
                count += 1
                print(f"Replaced Line {i+1}: {line.strip()}")

with open("scheduler.ino", "w") as f:
    f.write('\n'.join(lines))

print(f"Fixed {count} lines")
