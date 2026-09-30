#ifndef MADEIRA_XINPUT_ABI_H
#define MADEIRA_XINPUT_ABI_H
#include <stdint.h>
#include <stddef.h>
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int16_t SHORT;
typedef int32_t BOOL;
typedef struct { WORD wButtons; BYTE bLeftTrigger, bRightTrigger;
    SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY; } XINPUT_GAMEPAD;
typedef struct { DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad; } XINPUT_STATE;
typedef struct { WORD wLeftMotorSpeed, wRightMotorSpeed; } XINPUT_VIBRATION;
typedef struct { BYTE Type, SubType; WORD Flags; XINPUT_GAMEPAD Gamepad;
    XINPUT_VIBRATION Vibration; } XINPUT_CAPABILITIES;
typedef struct { BYTE BatteryType, BatteryLevel; } XINPUT_BATTERY_INFORMATION;
typedef struct { WORD VirtualKey, Unicode, Flags; BYTE UserIndex, HidCode; } XINPUT_KEYSTROKE;
_Static_assert(sizeof(XINPUT_GAMEPAD) == 12, "XInput gamepad ABI");
_Static_assert(sizeof(XINPUT_STATE) == 16, "XInput state ABI");
_Static_assert(offsetof(XINPUT_STATE, Gamepad) == 4, "XInput state offset");
_Static_assert(sizeof(XINPUT_CAPABILITIES) == 20, "XInput capabilities ABI");
_Static_assert(sizeof(XINPUT_VIBRATION) == 4, "XInput vibration ABI");
_Static_assert(sizeof(XINPUT_KEYSTROKE) == 8, "XInput keystroke ABI");
#define ERROR_SUCCESS 0u
#define ERROR_NOT_SUPPORTED 50u
#define ERROR_INVALID_PARAMETER 87u
#define ERROR_DEVICE_NOT_CONNECTED 1167u
#define ERROR_EMPTY 4306u
#endif
