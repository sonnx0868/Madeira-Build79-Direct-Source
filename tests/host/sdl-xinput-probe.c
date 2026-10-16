/* SPDX-License-Identifier: GPL-3.0-or-later WITH Madeira-Converter-Exception */
#include <windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef void* Controller;
#define FN(ret,name,args) typedef ret (__cdecl *name##_fn) args; name##_fn name=(name##_fn)GetProcAddress(sdl,#name);assert(name)
int main(int argc,char**argv) {
 int negative=argc>1?atoi(argv[1]):0;
 HMODULE host=LoadLibraryA("XInput1_4.dll");assert(host);
 void (__cdecl *state)(unsigned,short,short)=(void*)GetProcAddress(host,"ProbeState");assert(state);
 unsigned (__cdecl *count)(int)=(void*)GetProcAddress(host,"ProbeCount");assert(count);
 state(0,0,0);
 HMODULE sdl=LoadLibraryA("SDL2.dll");assert(sdl);
 FN(int,SDL_Init,(unsigned));FN(void,SDL_Quit,(void));FN(int,SDL_NumJoysticks,(void));FN(int,SDL_IsGameController,(int));
 FN(Controller,SDL_GameControllerOpen,(int));FN(unsigned char,SDL_GameControllerGetButton,(Controller,int));FN(short,SDL_GameControllerGetAxis,(Controller,int));
 FN(void,SDL_GameControllerUpdate,(void));FN(void,SDL_PumpEvents,(void));FN(void*,SDL_CreateWindow,(const char*,int,int,int,int,unsigned));
 FN(const char*,SDL_GetError,(void));
 int init=SDL_Init(0x20|0x2000);if(init){fprintf(stderr,"SDL error: %s\n",SDL_GetError());return 3;}
 assert(SDL_CreateWindow("Madeira input canary",0,0,32,32,8));
 int n=SDL_NumJoysticks();Controller controller=NULL;
 for(int i=0;i<n;i++)if(SDL_IsGameController(i)){controller=SDL_GameControllerOpen(i);if(controller)break;}
 if(negative==1){assert(!controller);printf("PASS: real SDL RawInput path suppresses the host-only XInput controller (queries caps=%u state=%u)\n",count(1),count(0));SDL_Quit();return 0;}
 assert(controller&&n==1&&count(1)>0);
 state(0x1000,20000,-15000);
 for(int i=0;i<10;i++){SDL_PumpEvents();SDL_GameControllerUpdate();Sleep(10);}
 if(negative==2){assert(!SDL_GameControllerGetButton(controller,0));puts("PASS: real SDL rejects a background-focus controller without the profile's background-events hint");SDL_Quit();return 0;}
 assert(SDL_GameControllerGetButton(controller,0));assert(SDL_GameControllerGetAxis(controller,0)>19000);
 state(0,0,0);for(int i=0;i<3;i++){SDL_PumpEvents();SDL_GameControllerUpdate();Sleep(10);}assert(!SDL_GameControllerGetButton(controller,0));
 state(0x3004,-30000,0);for(int i=0;i<3;i++){SDL_PumpEvents();SDL_GameControllerUpdate();Sleep(10);}
 assert(SDL_GameControllerGetButton(controller,0)&&SDL_GameControllerGetButton(controller,1)&&SDL_GameControllerGetButton(controller,13));
 assert(SDL_GameControllerGetAxis(controller,0)<-29000);
 printf("PASS: real SDL2 x86 detects one early host controller and receives buttons/sticks/releases through production snapshot/query (caps=%u state=%u)\n",count(1),count(0));
 SDL_Quit();return 0;
}
