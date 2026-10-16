/* Test-only XInput API around the production host snapshot/query. */
static unsigned caps_queries,state_queries;
DWORD WINAPI XInputGetState(DWORD i,XINPUT_STATE *s) { ++state_queries; return ios_gamepad_query(i,0,s)?ERROR_SUCCESS:ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputGetStateEx(DWORD i,XINPUT_STATE *s) { return XInputGetState(i,s); }
DWORD WINAPI XInputSetState(DWORD i,XINPUT_VIBRATION *v) { return i?ERROR_DEVICE_NOT_CONNECTED:ERROR_SUCCESS; }
DWORD WINAPI XInputGetCapabilities(DWORD i,DWORD f,XINPUT_CAPABILITIES *c) { ++caps_queries; return ios_gamepad_query(i,1,c)?ERROR_SUCCESS:ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputGetCapabilitiesEx(DWORD u,DWORD i,DWORD f,XINPUT_CAPABILITIES_EX *c) { ZeroMemory(c,sizeof(*c));DWORD ret=XInputGetCapabilities(i,f,&c->Capabilities);c->VendorId=0x045e;c->ProductId=0x028e;c->VersionNumber=0x0114;return ret; }
DWORD WINAPI XInputGetBatteryInformation(DWORD i,BYTE type,XINPUT_BATTERY_INFORMATION *b) { b->BatteryType=1;b->BatteryLevel=3;return i?ERROR_DEVICE_NOT_CONNECTED:ERROR_SUCCESS; }
void __cdecl ProbeState(unsigned buttons,short x,short y) { struct winios_gamepad s={0};s.connected=1;s.buttons=buttons;s.lx=x;s.ly=y;winios_gamepad_set_state(0,&s); }
unsigned __cdecl ProbeCount(int caps) {return caps?caps_queries:state_queries;}
