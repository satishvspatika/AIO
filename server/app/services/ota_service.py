import re

def get_numeric_ver(ver_str: str) -> tuple:
    """
    Safely extracts a comparable numeric version tuple from version strings.
    Handles both "CODE-VERSION" and "VERSION-CODE" formats.
    Returns (major, minor) so (5, 10) correctly evaluates > (5, 1)
    """
    if not ver_str:
        return (-1, -1)
    
    # Extract the first matching integer.integer pattern from the string
    match = re.search(r'(\d+)\.(\d+)', str(ver_str))
    if match:
        return (int(match.group(1)), int(match.group(2)))
        
    return (-1, -1)


def needs_ota(device_ver: str, target_ver: str) -> bool:
    """
    Returns True if device version is strictly older than target version.
    Uses semantic tuple comparison to handle edge cases accurately.
    """
    if not target_ver:
        return False
    return get_numeric_ver(device_ver) < get_numeric_ver(target_ver)
