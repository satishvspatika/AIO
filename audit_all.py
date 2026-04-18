
import sys
import os

def audit_full_project(directory, main_file, defines):
    files = [f for f in os.listdir(directory) if f.endswith('.ino')]
    files.remove(main_file)
    files.sort()
    all_files = [main_file] + files
    
    depth = 0
    active_stack = [True]
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    escaped = False
    
    for filename in all_files:
        path = os.path.join(directory, filename)
        with open(path, 'r') as f:
            content = f.read()
        
        lines = content.split('\n')
        line_no = 0
        for line in lines:
            line_no += 1
            stripped = line.strip()
            
            if stripped.startswith('#if'):
                is_active = False
                for d in defines:
                    if d in stripped:
                        is_active = True
                        break
                active_stack.append(active_stack[-1] and is_active)
                continue
            elif stripped.startswith('#else'):
                prev = active_stack.pop()
                active_stack.append(active_stack[-1] and not prev)
                continue
            elif stripped.startswith('#elif'):
                active_stack.pop()
                is_active = False
                for d in defines:
                    if d in stripped:
                        is_active = True
                        break
                active_stack.append(active_stack[-1] and is_active)
                continue
            elif stripped.startswith('#endif'):
                active_stack.pop()
                continue
                
            if not active_stack[-1]:
                continue
                
            for i, char in enumerate(line):
                if escaped:
                    escaped = False
                    continue
                if in_line_comment:
                    break
                if in_block_comment:
                    if char == '*' and i + 1 < len(line) and line[i+1] == '/':
                        pass
                    if char == '/' and i > 0 and line[i-1] == '*':
                        in_block_comment = False
                    continue
                if in_string:
                    if char == '"':
                        in_string = False
                    elif char == '\\':
                        escaped = True
                    continue
                if in_char:
                    if char == "'":
                        in_char = False
                    elif char == '\\':
                        escaped = True
                    continue
                if char == '/' and i + 1 < len(line):
                    if line[i+1] == '/':
                        in_line_comment = True
                        break
                    if line[i+1] == '*':
                        in_block_comment = True
                        continue
                if char == '"':
                    in_string = True
                    continue
                if char == "'":
                    in_char = True
                    continue
                if char == '{':
                    depth += 1
                elif char == '}':
                    depth -= 1
                    if depth < 0:
                        print(f"DEPTH CRASH in {filename} at line {line_no}")
                        return
            in_line_comment = False
            
        print(f"After {filename}: Depth {depth}")
    
    print(f"Final Project Depth: {depth}")

if __name__ == "__main__":
    audit_full_project(".", "AIO9_5.0.ino", sys.argv[1:])
