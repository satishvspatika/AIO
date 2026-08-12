import html.parser
import sys

class HTMLValidator(html.parser.HTMLParser):
    def __init__(self):
        super().__init__()
        self.tags_stack = []
        self.errors = []

    def handle_starttag(self, tag, attrs):
        # We ignore self-closing tags in HTML5
        self_closing = ['area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 
                        'link', 'meta', 'param', 'source', 'track', 'wbr']
        if tag not in self_closing:
            self.tags_stack.append((tag, self.getpos()))

    def handle_endtag(self, tag):
        self_closing = ['area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 
                        'link', 'meta', 'param', 'source', 'track', 'wbr']
        if tag in self_closing:
            return

        if not self.tags_stack:
            self.errors.append(f"Unexpected closing tag </{tag}> at line {self.getpos()[0]}, col {self.getpos()[1]}")
            return

        expected_tag, pos = self.tags_stack.pop()
        if expected_tag != tag:
            self.errors.append(f"Mismatched tag: expected </{expected_tag}> (from line {pos[0]}, col {pos[1]}), but found </{tag}> at line {self.getpos()[0]}, col {self.getpos()[1]}")
            # Put expected tag back to attempt recovery
            self.tags_stack.append((expected_tag, pos))

def main():
    import os
    import shutil
    base_dir = os.path.dirname(os.path.abspath(__file__))
    web_html_path = os.path.join(base_dir, 'WEB_FLASH_FILES', 'factory_tool.html')
    jig_html_path = os.path.join(base_dir, 'factory_tool.html')

    # Use TEST_JIG/factory_tool.html as primary source
    html_path = jig_html_path if os.path.exists(jig_html_path) else web_html_path

    with open(html_path, 'r', encoding='utf-8') as f:
        html_content = f.read()

    parser = HTMLValidator()
    parser.feed(html_content)

    print(f"Finished parsing HTML. Stack size: {len(parser.tags_stack)}")
    if parser.errors:
        print("\n❌ HTML Structure Errors Found:")
        for err in parser.errors[:10]:
            print(f"  - {err}")
        if len(parser.errors) > 10:
            print(f"  ... and {len(parser.errors) - 10} more errors.")
        sys.exit(1)
    
    if parser.tags_stack:
        print("\n❌ Unclosed Tags Remaining:")
        for tag, pos in reversed(parser.tags_stack):
            print(f"  - <{tag}> opened at line {pos[0]}, col {pos[1]}")
        sys.exit(1)

    print("\n✅ HTML is structurally valid!")
    
    # Sync from jig_html_path to web_html_path and Downloads
    shutil.copy2(jig_html_path, web_html_path)
    
    # Sync Windows & Mac launcher scripts
    web_dir = os.path.join(base_dir, 'WEB_FLASH_FILES')
    for script_name in ['launch_windows.bat', 'server.ps1', 'launch_mac.command']:
        src = os.path.join(web_dir, script_name)
        dst = os.path.join(base_dir, script_name)
        if os.path.exists(src):
            shutil.copy2(src, dst)
        elif os.path.exists(dst):
            shutil.copy2(dst, src)

    downloads_path = os.path.expanduser('~/Downloads/factory_tool.html')
    if os.path.exists(os.path.dirname(downloads_path)):
        shutil.copy2(web_html_path, downloads_path)
        print("✅ Synced physical file copy to ~/Downloads/factory_tool.html")
    print("✅ Synced physical file copies and launch scripts (launch_windows.bat / launch_mac.command / server.ps1) between TEST_JIG/ and WEB_FLASH_FILES/")

if __name__ == '__main__':
    main()
