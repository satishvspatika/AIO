import re

with open('gprs_ftp.ino', 'r') as f:
    text = f.read()

# We look for lines that just start with `", ` or `");` preceded by a newline that broke a string literal.
text = text.replace('\n",', '\\n",')
text = text.replace('\n");', '\\n");')

with open('gprs_ftp.ino', 'w') as f:
    f.write(text)
