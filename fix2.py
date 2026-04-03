import re

with open("scheduler.ino", "r") as f:
    lines = f.read().split('\n')

state = -1
count = 0
for i in range(len(lines)):
    line = lines[i]
    
    # Catch simple #if SYSTEM == X or #elif SYSTEM == X (with or without spaces/tabs at start)
    m1 = re.search(r'#(?:if|elif)\s+SYSTEM\s*==\s*(\d)', line)
    if m1:
        state = int(m1.group(1))
    
    # Catch #if (SYSTEM == 1 || SYSTEM == 2)
    if '#if' in line or '#elif' in line:
        if 'SYSTEM == 1' in line or 'SYSTEM == 2' in line:
            if 'SYSTEM == 0' not in line:
                state = 1
        if 'SYSTEM == 0 || SYSTEM == 2' in line:
            state = 0

    if '#else' in line or '#endif' in line:
        state = -1

    if state in [1, 2]:
        if 'bat_val' in line and '%05.2f' in line:
            old = line
            line = line.replace('%05.2f', '%04.1f')
            if old != line:
                lines[i] = line
                count += 1
                print(f"Replaced Line {i+1}: {line.strip()}")

with open("scheduler.ino", "w") as f:
    f.write('\n'.join(lines))

print(f"Fixed {count} lines")
