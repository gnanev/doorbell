#ifndef _COMMON_H_
#define _COMMON_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "avrlibtypes.h"
#include "avrlibdefs.h"

typedef void (*voidFunc)();
typedef void (*voidFunc_8)(u08);
typedef void (*voidFunc_16)(u16);

#define BOOL u08
#define ON 1
#define OFF 0
#define YES 1
#define NO 0

#endif
