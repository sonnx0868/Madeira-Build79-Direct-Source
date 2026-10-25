/* Fixed diagnostic labels only. Never return an argv value, URL, profile path,
 * extension ID, pipe name or authentication token from a browser command. */
#ifndef MADEIRA_IOS_BROWSER_CHILD_ROLE_H
#define MADEIRA_IOS_BROWSER_CHILD_ROLE_H
#include <string.h>

static const char *ios_browser_child_role( int argc, char *const argv[] )
{
    const char *type = NULL;
    int browser = 0, crashpad = 0, i;
    if (!argv || argc <= 0) return NULL;
    for (i = 0; i < argc; ++i)
    {
        const char *arg = argv[i], *name;
        if (!arg) continue;
        name = strrchr( arg, '\\' );
        name = name ? name + 1 : arg;
        { const char *slash = strrchr( name, '/' ); if (slash) name = slash + 1; }
        if (!strcmp( name, "chrome.exe" ) || !strcmp( name, "msedge.exe" ) ||
            !strcmp( name, "brave.exe" )) browser = 1;
        if (!strncmp( arg, "--type=", 7 )) type = arg + 7;
        else if (!strcmp( arg, "--type" ) && i + 1 < argc) type = argv[i + 1];
        if (!strcmp( arg, "--monitor-self" )) crashpad = 1;
    }
    if (!browser) return NULL;
    if (!type || !*type) return crashpad ? "crashpad" : "browser";
    if (!strcmp( type, "renderer" )) return "renderer";
    if (!strcmp( type, "gpu-process" )) return "gpu";
    if (!strcmp( type, "utility" )) return "utility";
    if (!strcmp( type, "crashpad-handler" )) return "crashpad";
    return "other-browser-child";
}
#endif
