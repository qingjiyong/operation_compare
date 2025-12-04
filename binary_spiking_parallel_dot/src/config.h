/*
spiking vector dot product accelerator, binary encoding and parallel processing version
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__


#include <stdint.h>
#include <hls_stream.h>
#include <ap_int.h>
#include <hls_vector.h>

#define DATA_BITS 4
#define WEIGHT_BITS 4
#define ACCUMULATE_BITS 16
#define ENTRIES 128
// 16 bits is enough: max result = (-8) * (-8) * 128 = 8192, min result = (-8) * 7 *128 = -7168 

// data type
using Data_t = ap_int<DATA_BITS>;
using Weight_t = ap_int<WEIGHT_BITS>;
using Accu_t = ap_int<ACCUMULATE_BITS>;
using Result_t = ap_int<ACCUMULATE_BITS>;

//packed data
#define Pack_Data ap_uint<ENTRIES * DATA_BITS>
#define Pack_Weight ap_uint<ENTRIES * WEIGHT_BITS>

// spiking representation constants
#define SPIKE_BITS 2
#define TRANS_LEN 4
#define Spike_t ap_int<SPIKE_BITS>
struct Spiking_Block {
    Spike_t data[ENTRIES][TRANS_LEN];
};




#endif // __CONFIG_H__