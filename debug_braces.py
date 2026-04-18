
import sys

def audit_file(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    depth = 0
    for i, line in enumerate(lines):
        # Remove comments to avoid false counts
        clean_line = line.split('//')[0]
        # Very simple quote handling
        in_string = False
        escaped = False
        for char in clean_line:
            if char == '"' and not escaped:
                in_string = not in_string
            if not in_string:
                if char == '{':
                    depth += 1
                elif char == '}':
                    depth -= 1
            if char == '\\':
                escaped = True
            else:
                escaped = False
        
        if depth < 0:
            print(f"DEPTH CRASH at line {i+1}: {line.strip()}")
            return
            
    print(f"Final Depth for {filename}: {depth}")

if __name__ == "__main__":
    audit_file(sys.argv[1])
