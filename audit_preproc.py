
import sys

def audit_preproc(filename, defines):
    with open(filename, 'r') as f:
        lines = f.readlines()
        
    depth = 0
    active_stack = [True] # Stack of boolean
    
    line_no = 0
    for line in lines:
        line_no += 1
        stripped = line.strip()
        
        # Handle Preprocessor
        if stripped.startswith('#if'):
            # Simple evaluation: check if define is in line
            is_active = False
            for d in defines:
                if d in stripped:
                    is_active = True
                    break
            # If it's a "defined()" check or complex, this script might fail
            # But let's assume simple #if SYSTEM == 0
            active_stack.append(active_stack[-1] and is_active)
            continue
        elif stripped.startswith('#else'):
            # Flip the last state
            prev_active = active_stack.pop()
            parent_active = active_stack[-1]
            active_stack.append(parent_active and not prev_active)
            continue
        elif stripped.startswith('#elif'):
            # Pop and push new
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
            
        # Audit braces in active line
        # (Ignoring strings/comments for simplicity since I suspect structural drift)
        for char in line:
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
                if depth < 0:
                    print(f"DEPTH CRASH at line {line_no}")
                    return
                    
    print(f"Final Depth for {filename} with {defines}: {depth}")

if __name__ == "__main__":
    audit_preproc(sys.argv[1], sys.argv[2:])
