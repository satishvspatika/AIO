import subprocess

def main():
    # Run git diff
    res = subprocess.run(['git', 'diff', 'TEST_JIG/factory_tool.html'], capture_output=True, text=True)
    diff = res.stdout
    
    hunks = diff.split('@@')
    print(f"Analyzing {len(hunks)//2} hunks...")
    
    for i in range(1, len(hunks), 2):
        if i + 1 >= len(hunks):
            break
        header = hunks[i]
        content = hunks[i+1]
        
        # Count braces in added lines (starting with +)
        added_lines = [line[1:] for line in content.split('\n') if line.startswith('+') and not line.startswith('+++')]
        added_text = '\n'.join(added_lines)
        
        opens = added_text.count('{')
        closes = added_text.count('}')
        
        # Count braces in removed lines (starting with -)
        removed_lines = [line[1:] for line in content.split('\n') if line.startswith('-') and not line.startswith('---')]
        removed_text = '\n'.join(removed_lines)
        
        rem_opens = removed_text.count('{')
        rem_closes = removed_text.count('}')
        
        net_opens = opens - rem_opens
        net_closes = closes - rem_closes
        
        if net_opens != net_closes:
            print(f"\n⚠️ Hunk at index {i//2 + 1} ({header.strip()}) is unbalanced:")
            print(f"  Net added {{: {net_opens}")
            print(f"  Net added }}: {net_closes}")
            
            # Print lines
            print("--- Added lines in this hunk ---")
            for line in added_lines:
                if '{' in line or '}' in line:
                    print(line)

if __name__ == '__main__':
    main()
