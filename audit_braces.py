
import sys

def audit_braces(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    depth = 0
    in_block_comment = False
    for i, line in enumerate(lines):
        line_no = i + 1
        
        # Remove single line comments
        line = line.split('//')[0]
        
        # Handle block comments
        while True:
            if not in_block_comment:
                start = line.find('/*')
                if start != -1:
                    end = line.find('*/', start + 2)
                    if end != -1:
                        line = line[:start] + line[end+2:]
                        continue
                    else:
                        line = line[:start]
                        in_block_comment = True
                        break
                else:
                    break
            else:
                end = line.find('*/')
                if end != -1:
                    line = line[end+2:]
                    in_block_comment = False
                    continue
                else:
                    line = ""
                    break
        
        # Remove strings
        line = '"'.join(re.split(r'(?<!\\)".*?(?<!\\)"', line)[::2]) if '"' in line else line

        for char in line:
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
        
        if depth < 0:
            print(f"ERROR: Negative depth at line {line_no}: {depth}")
            depth = 0
        
    print(f"Final depth: {depth}")

if __name__ == "__main__":
    import re
    audit_braces(sys.argv[1])
