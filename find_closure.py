
import sys

def find_premature_closure(filename, start_line):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    depth = 0
    in_function = False
    for i, line in enumerate(lines):
        line_num = i + 1
        if line_num < start_line:
            continue
            
        if not in_function:
            if '{' in line:
                in_function = True
                depth = 0 # reset at function start
        
        # Remove comments
        clean_line = line.split('//')[0]
        # Skip strings
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
        
        if in_function and depth == 0:
            print(f"Function CLOSED at line {line_num}: {line.strip()}")
            return

if __name__ == "__main__":
    find_premature_closure(sys.argv[1], int(sys.argv[2]))
