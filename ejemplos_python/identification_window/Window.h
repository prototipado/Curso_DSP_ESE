#ifndef _INC_WINDOW_H_
#define _INC_WINDOW_H_

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "math.h"
#include "esp_dsp.h"

bool transform(float *x, float *dest);
void clear(void);
#endif