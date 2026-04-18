
import sys

def audit_final(filename, defines):
    with open(filename, 'r') as f:
        content = f.read()
    
    depth = 0
    active_stack = [True]
    
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    escaped = False
    
    lines = content.split('\n')
    line_no = 0
    
    for line in lines:
        line_no += 1
        stripped = line.strip()
        
        # Preprocessor check (only evaluate if NOT in block comment/string)
        # (Assuming #if are not inside strings/comments which is 99% true for this codebase)
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
            
        # Parse characters in line
        for i, char in enumerate(line):
            if escaped:
                escaped = False
                continue
            
            if in_line_comment:
                break # Rest of line is comment
                
            if in_block_comment:
                if char == '*' and i + 1 < len(line) and line[i+1] == '/':
                    # End of block comment handled by next char or skip
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
                
            # Starters
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
                    print(f"DEPTH CRASH at line {line_no}")
                    return
        
        # End of line resets
        in_line_comment = False
        
    print(f"Final Depth for {filename} with {defines}: {depth}")

if __name__ == "__main__":
    audit_final(sys.argv[1], sys.argv[2:])
