
import sys

def audit_file_pro(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    depth = 0
    line_no = 1
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    escaped = False
    
    for i, char in enumerate(content):
        if char == '\n':
            line_no += 1
            in_line_comment = False
            
        if escaped:
            escaped = False
            continue
            
        if in_line_comment:
            continue
            
        if in_block_comment:
            if char == '*' and i + 1 < len(content) and content[i+1] == '/':
                # Handled by next char check or skip
                pass
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
            
        # Not in string or comment
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
            if depth < 0:
                print(f"DEPTH CRASH at line {line_no}: Negative depth")
                # print snippet
                start = max(0, i - 20)
                end = min(len(content), i + 20)
                print(f"Context: ...{content[start:end]}...")
                return
                
    print(f"Final Depth for {filename}: {depth}")

if __name__ == "__main__":
    audit_file_pro(sys.argv[1])
