
import sys

def audit_depth_per_line(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    depth = 0
    line_no = 1
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    escaped = False
    
    line_depths = []
    
    for i, char in enumerate(content):
        if char == '\n':
            line_depths.append((line_no, depth))
            line_no += 1
            in_line_comment = False
            
        if escaped:
            escaped = False
            continue
            
        if in_line_comment:
            continue
            
        if in_block_comment:
            if char == '/' and i > 0 and content[i-1] == '*':
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
            
        if char == '/' and i + 1 < len(content):
            if content[i+1] == '/':
                in_line_comment = True
                continue
            if content[i+1] == '*':
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
            
    # Final depth
    line_depths.append((line_no, depth))
    
    # Only print interesting lines (where depth changed)
    last_d = -1
    for ln, d in line_depths:
        if d != last_d:
            print(f"Line {ln}: Depth {d}")
            last_d = d

if __name__ == "__main__":
    audit_depth_per_line(sys.argv[1])
