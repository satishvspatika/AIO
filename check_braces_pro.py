import sys
import re

def count_braces(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Remove single line comments
    content = re.sub(r'//.*', '', content)
    # Remove multi-line comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove strings
    content = re.sub(r'".*?"', '""', content)
    content = re.sub(r"'.*?'", "''", content)
    
    open_b = content.count('{')
    close_b = content.count('}')
    return open_b, close_b

for filename in sys.argv[1:]:
    o, c = count_braces(filename)
    if o != c:
        print(f"{filename}: {{={o}, }}={c} DIFF={o-c}")
