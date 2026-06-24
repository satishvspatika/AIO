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
    base_dir = os.path.dirname(os.path.abspath(__file__))
    html_path = os.path.join(base_dir, 'factory_tool.html')
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

if __name__ == '__main__':
    main()
