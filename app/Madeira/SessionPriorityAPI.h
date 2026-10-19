#pragma once
#include <stdint.h>
#include <stddef.h>
void madeira_session_priority_begin(void);
void madeira_session_priority_end(void);
void madeira_session_priority_foreground(int active);
void madeira_session_priority_thread(unsigned pid,unsigned tid,unsigned parent,void *peb);
void madeira_session_priority_image(unsigned pid,const uint16_t *image,size_t length);
void madeira_session_priority_select(unsigned pid);
void madeira_session_priority_presented(unsigned pid);
void madeira_session_priority_rendering(void);
void madeira_session_priority_retired(unsigned pid,void *peb);
const uint64_t *madeira_session_priority_epoch_address(void);
void madeira_session_priority_poll_current(int base_changed);
