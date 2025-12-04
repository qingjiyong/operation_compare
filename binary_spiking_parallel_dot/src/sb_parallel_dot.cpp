#include "sb_parallel_dot.h"

void load_and_unpack(
    const int rounds,
    Pack_Data *data,
    Pack_Weight *weight,
    hls::stream<Spiking_Block> &spike_stream,
    hls::stream<hls::vector<Weight_t, ENTRIES>> &weight_stream
) {
    round_data_load:
    for(int r = 0; r < rounds; r++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
        #pragma HLS PIPELINE II=1
        // read packed data
        Pack_Data packed_data = data[r];
        Pack_Weight packed_weight = weight[r];

        // unpack data
        //hls::vector<Data_t, ENTRIES> data_vec;
        Spiking_Block spike_block;
        hls::vector<Weight_t, ENTRIES> weight_vec;
        unpack_data_loop:
        for (int i = 0; i < ENTRIES; i++) {
            #pragma HLS UNROLL
            weight_vec[i] = packed_weight.range((i+1)*WEIGHT_BITS-1, i*WEIGHT_BITS);
            Data_t tmp_data = packed_data.range((i+1)*DATA_BITS-1, i*DATA_BITS);
            // convert to spiking representation
            // if tmp_data > 0, directly map to spikes
            // else convert negative integer to sign-magnitude then map to negative spikes
            /**************************************************
             * example: integer 5(0101) maps to spikes [0,1,0,1], every 2 bits represent a spike
             *          integer -5(1011) maps to sign-magnitude 1101, then maps to spikes [0,-1,0,-1]
             **************************************************/
            if(tmp_data >= 0) { // positive integer
                spike_block.data[i][3] = (tmp_data[3] == 0) ? 0 : 1;
                spike_block.data[i][2] = (tmp_data[2] == 0) ? 0 : 1;
                spike_block.data[i][1] = (tmp_data[1] == 0) ? 0 : 1;
                spike_block.data[i][0] = (tmp_data[0] == 0) ? 0 : 1;
            } else {
                Data_t sign_magnitude = ((~tmp_data) + 1); // two's complement to sign-magnitude
                spike_block.data[i][3] = (sign_magnitude[3] == 0) ? 0 : -1;
                spike_block.data[i][2] = (sign_magnitude[2] == 0) ? 0 : -1;
                spike_block.data[i][1] = (sign_magnitude[1] == 0) ? 0 : -1;
                spike_block.data[i][0] = (sign_magnitude[0] == 0) ? 0 : -1;
            }
        }

        // push to stream
        spike_stream.write(spike_block);
        weight_stream.write(weight_vec);
    }
}

void compute(
    const int rounds,
    hls::stream<Spiking_Block> &spike_stream,
    hls::stream<hls::vector<Weight_t, ENTRIES>> &weight_stream,
    hls::stream<Result_t> &result_stream
) {
    round_compute:
    for(int r = 0; r < rounds; r++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
        #pragma HLS PIPELINE II=1
        // read from stream
        Spiking_Block spiking_data = spike_stream.read();
        hls::vector<Weight_t, ENTRIES> weight_vec = weight_stream.read();

        // compute dot product
        Result_t partial_sum[TRANS_LEN] = {0};
        Result_t acc_result = 0;
        dot_product_loop:
        for (int i = 0; i < ENTRIES; i++) {
            #pragma HLS UNROLL
            for(int j = 0; j < TRANS_LEN; j++) {
                #pragma HLS UNROLL
               
                /*if (spiking_data.data[i][j] == 1) {
                    partial_sum[j] += weight_vec[i];
                    #pragma HLS BIND_OP variable=partial_sum op=add impl=fabric
                }
                else if (spiking_data.data[i][j] == -1)
                {
                    partial_sum[j] -= weight_vec[i];
                    #pragma HLS BIND_OP variable=partial_sum op=add impl=fabric
                    continue;
                }
                else continue; // skip zero spike to save computation*/
                if(spiking_data.data[i][j] != 0) {
                    partial_sum[j] += spiking_data.data[i][j] * weight_vec[i];
                    #pragma HLS BIND_OP variable=partial_sum op=mul impl=fabric
                }
                else continue; // skip zero spike to save computation
                //partial_sum[j] += spiking_data.data[i][j] * weight_vec[i];
                //#pragma HLS BIND_OP variable=partial_sum op=add impl=fabric
            }
        }
        // accumulate partial sums
        for(int j = 0; j < TRANS_LEN; j++) {
            #pragma HLS UNROLL
            acc_result += (partial_sum[j]*(1 << j));

        }

        result_stream.write(acc_result);
    }
}

void store_result(
    const int rounds,
    Result_t *result,
    hls::stream<Result_t> &result_stream
) {
    round_store:
    for(int r = 0; r < rounds; r++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
        #pragma HLS PIPELINE II=1
        result[r] = result_stream.read();
    }
}


void sb_parallel_dot(
    const int rounds,   // number of rounds to perform
    Pack_Data *data,  // 128 packed data entries
    Pack_Weight *weight,  // 128 packed weight entries
    Result_t *result    // dot product result
) {

    #pragma HLS INTERFACE mode=s_axilite port=rounds bundle=CTRL_BUS
    #pragma HLS INTERFACE mode=m_axi port=data bundle=DATA_BUS depth=1024 offset=slave
    #pragma HLS INTERFACE mode=m_axi     port=weight  offset=slave bundle=WEIGHT_BUS depth=1024
    #pragma HLS INTERFACE mode=m_axi     port=result  offset=slave bundle=RESULT_BUS depth=1024

    // streams for data and weight
    hls::stream<Spiking_Block> spike_stream;
    #pragma HLS STREAM variable=spike_stream depth=2
    #pragma HLS BIND_STORAGE variable=spike_stream type=fifo impl=srl

    hls::stream<hls::vector<Weight_t, ENTRIES>> weight_stream;
    #pragma HLS STREAM variable=weight_stream depth=2
    #pragma HLS BIND_STORAGE variable=weight_stream type=fifo impl=srl

    hls::stream<Result_t> result_stream;
    #pragma HLS STREAM variable=result_stream depth=2
    #pragma HLS BIND_STORAGE variable=result_stream type=fifo impl=srl

    #pragma HLS DATAFLOW
    
    load_and_unpack(rounds, data, weight, spike_stream, weight_stream);
    compute(rounds, spike_stream, weight_stream, result_stream);
    store_result(rounds, result, result_stream);
}