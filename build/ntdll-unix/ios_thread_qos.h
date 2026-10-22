/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once
#include <pthread/qos.h>
int madeira_guest_qos_self(qos_class_t cls, int relative_priority);
void ios_thread_qos_exit(unsigned tid);
