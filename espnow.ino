// ESP-NOW has been permanently removed from this firmware (v5.40+).
// All field units use wired LCD (wired=1). No wireless UI sync is needed.
// ENABLE_ESPNOW is set to 0 in globals.h.
//
// The esp_now_h handle, peer_added, send_espnow, destinationAddress, and
// newMACAddress variables are retained in globals.h for backward compatibility
// with older configs, but no functionality is compiled or linked.
//
// To re-enable in future: Set ENABLE_ESPNOW=1 in globals.h and
// restore the implementation from git history (tag: pre-espnow-removal).
