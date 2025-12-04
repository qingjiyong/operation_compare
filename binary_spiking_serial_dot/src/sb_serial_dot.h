#ifndef __SB_SERIAL_DOT_H__
#define __SB_SERIAL_DOT_H__
#include "config.h"

void sb_serial_dot(
    const int rounds,   // number of rounds to perform
    Pack_Data *data,  // 128 packed data entries
    Pack_Weight *weight,  // 128 packed weight entries
    Result_t *result    // dot product result
);
#endif // __SB_SERIAL_DOT_H__