def get_numeric_ver(ver_str: str) -> float:
    """
    Safely extracts a comparable numeric version from version strings.
    Handles both "CODE-VERSION" and "VERSION-CODE" formats.
    """
    if not ver_str:
        return -1.0
    
    parts = str(ver_str).split("-")
    
    # Try the last part (traditional "TWSRF9-GEN-5.79")
    try:
        return float(parts[-1])
    except (ValueError, IndexError):
        pass
        
    # Try the first part (new "5.79-S-AIO")
    try:
        return float(parts[0])
    except (ValueError, IndexError):
        pass
        
    return -1.0


def needs_ota(device_ver: str, target_ver: str) -> bool:
    """
    Returns True if device version is different from target version.
    Uses numeric comparison to handle float edge cases (e.g. 5.9 vs 5.10).
    """
    return get_numeric_ver(device_ver) != get_numeric_ver(target_ver)
