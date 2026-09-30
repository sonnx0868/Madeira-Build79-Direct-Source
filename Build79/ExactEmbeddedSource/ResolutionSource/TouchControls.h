#ifndef MADEIRA_TOUCH_CONTROLS_H
#define MADEIRA_TOUCH_CONTROLS_H
/* Main-thread backend. Original UI/editor/layout remain the single owner. */
int MadeiraTouchLegacyInput(unsigned key, unsigned value, unsigned flags);
int MadeiraTouchIsEditing(void);
void MadeiraTouchRefresh(UIWindow *overlay, UIWindow *game, UIView *metal, int full, int allowed);
void MadeiraTouchReset(void);
void MadeiraTouchLayoutChanged(UIWindow *overlay);
int MadeiraTouchGlassEnabled(void);
UIView *MadeiraTouchCreateGlass(void);
const char *MadeiraTouchDiagnostics(void);
/* Implemented by the existing UI module, using inspected guest-only bridges. */
void MadeiraTouchPostKey(int vk, int down);
void MadeiraTouchPostMouse(unsigned flags);
void MadeiraTouchToggleKeyboard(void);
int MadeiraTouchWineRunning(void);
void madeira_resolution_trace(NSString *message);
#endif
