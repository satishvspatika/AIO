with open('gprs_ftp.ino', 'r') as f:
    lines = f.readlines()

brace_level = 0
for i, line in enumerate(lines):
    # strip comments and strings to not count braces inside them
    # a bit hacky but rough estimate
    clean_line = line.split('//')[0]
    brace_level += clean_line.count('{') - clean_line.count('}')
    if brace_level < 0:
        print(f"Negative brace level at line {i+1}: {line.strip()}")
        brace_level = 0
