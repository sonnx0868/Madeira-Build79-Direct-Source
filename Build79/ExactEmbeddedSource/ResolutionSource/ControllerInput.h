#ifndef MADEIRA_CONTROLLER_INPUT_H
#define MADEIRA_CONTROLLER_INPUT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* All entrypoints must be called on the iOS main thread. Zero means success.
 * path: absolute Unix path to Wine's drive_c/Games/MadeiraController.bin.
 * The writer creates its parent directory and only writes this exact file. */
int MadeiraControllerStart(const char *path);
void MadeiraControllerStop(void);
/* Additional deadzones, [0,.95], default zero: retain GameController filtering. */
void MadeiraControllerSetDeadzones(float left, float right, float triggers);
/* Allows the integration to suppress game input while its own menu is open.
 * UIApplication inactive/background state independently suppresses input. */
void MadeiraControllerSetInputEnabled(int enabled);
/* Touch contributes to virtual slot zero only. Axes are finite-clamped to
 * [-1,1]; strongest absolute axis wins, with physical input winning ties.
 * The legacy entrypoint replaces touch axes with zero. Gate loss clears touch
 * ownership; callers must submit fresh state after input resumes. */
void MadeiraControllerSetTouchState(uint32_t buttons, uint8_t left_trigger,
                                   uint8_t right_trigger, int connected);
void MadeiraControllerSetTouchStateWithStick(uint32_t buttons, uint8_t left_trigger,
                                            uint8_t right_trigger, float lx,
                                            float ly, int connected);
unsigned MadeiraControllerConnectedCount(void);
/* Direct iOS detection and live physical input, independent of guest/menu gating.
 * Diagnostic UTF-8 pointer is retained until the next main-thread call. */
unsigned MadeiraControllerDetectedCount(void);
const char *MadeiraControllerDiagnostics(void);
#ifdef __cplusplus
}
#endif
#endif
