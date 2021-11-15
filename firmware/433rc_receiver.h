#ifndef _433RC_RECEIVER_H_
#define _433RC_RECEIVER_H_

#include "common.h"

void setup433Receiver();
BOOL get433Data(u32* data);
void dump433Data();

BOOL getDiagnosticsBuffer(u08* buff);

#endif 