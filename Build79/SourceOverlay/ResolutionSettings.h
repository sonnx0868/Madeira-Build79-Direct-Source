#define MADEIRA_RESOLUTION_WIDTH_KEY @"MadeiraIPad.customDesktopWidth"
#define MADEIRA_RESOLUTION_HEIGHT_KEY @"MadeiraIPad.customDesktopHeight"
#define MADEIRA_RESOLUTION_MATCH_KEY @"MadeiraIPad.matchDesktopAspect"
#define MADEIRA_RESOLUTION_NATIVE_KEY @"MadeiraIPad.nativeDesktopResolution"

int madeira_resolution_parse_even(const char *text, int minimum, int maximum);
int madeira_resolution_saved(int *width, int *height, int *match);
int madeira_resolution_effective(int *width, int *height);
int madeira_resolution_match_height(int width);
int madeira_resolution_native(int *width, int *height);
int madeira_resolution_is_native(void);
