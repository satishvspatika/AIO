import sys
import re

def find_unclosed(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    stack = []
    
    content = "".join(lines)
    # Remove multi-line comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove single line comments
    content = re.sub(r'//.*', '', content)
    # Remove strings
    content = re.sub(r'".*?"', '""', content)
    content = re.sub(r"'.*?'", "''", content)
    
    stack = []
    for match in re.finditer(r'[\{\}]', content):
        char = match.group()
        pos = match.start()
        # Find line number for pos
        line_no = content.count('\n', 0, pos) + 1
        if char == '{':
            stack.append(line_no)
        else:
            if stack:
                stack.pop()
            else:
                print(f"{filename}: Extra closing brace at line {line_no}")
    
    for line_no in stack:
        print(f"{filename}: Unclosed opening brace at line {line_no}")

    
    for line_no in stack:
        print(f"{filename}: Unclosed opening brace at line {line_no}")

find_unclosed(sys.argv[1])
