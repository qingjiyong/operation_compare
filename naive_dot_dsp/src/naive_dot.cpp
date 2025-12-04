#include "naive_dot.h"

void load_and_unpack(
    const int rounds,
    Pack_Data *data,
    Pack_Weight *weight,
    hls::stream<hls::vector<Data_t, ENTRIES>> &data_stream,
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
        hls::vector<Data_t, ENTRIES> data_vec;
        hls::vector<Weight_t, ENTRIES> weight_vec;
        unpack_data_loop:
        for (int i = 0; i < ENTRIES; i++) {
            #pragma HLS UNROLL
            data_vec[i] = packed_data.range((i+1)*DATA_BITS-1, i*DATA_BITS);
            weight_vec[i] = packed_weight.range((i+1)*WEIGHT_BITS-1, i*WEIGHT_BITS);
        }

        // push to stream
        data_stream.write(data_vec);
        weight_stream.write(weight_vec);
    }
}

void compute(
    const int rounds,
    hls::stream<hls::vector<Data_t, ENTRIES>> &data_stream,
    hls::stream<hls::vector<Weight_t, ENTRIES>> &weight_stream,
    hls::stream<Result_t> &result_stream
) {
    round_compute:
    for(int r = 0; r < rounds; r++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
        #pragma HLS PIPELINE II=1
        // read from stream
        hls::vector<Data_t, ENTRIES> data_vec = data_stream.read();
        hls::vector<Weight_t, ENTRIES> weight_vec = weight_stream.read();

        // compute dot product
        Result_t acc = 0;
        dot_product_loop:
        for (int i = 0; i < ENTRIES; i++) {
            #pragma HLS UNROLL
            acc += data_vec[i] * weight_vec[i];
        }
        result_stream.write(acc);
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


void naive_dot(
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
    hls::stream<hls::vector<Data_t, ENTRIES>> data_stream;
    #pragma HLS STREAM variable=data_stream depth=2
    #pragma HLS BIND_STORAGE variable=data_stream type=fifo impl=srl

    hls::stream<hls::vector<Weight_t, ENTRIES>> weight_stream;
    #pragma HLS STREAM variable=weight_stream depth=2
    #pragma HLS BIND_STORAGE variable=weight_stream type=fifo impl=srl

    hls::stream<Result_t> result_stream;
    #pragma HLS STREAM variable=result_stream depth=2
    #pragma HLS BIND_STORAGE variable=result_stream type=fifo impl=srl

    #pragma HLS DATAFLOW
    
    load_and_unpack(rounds, data, weight, data_stream, weight_stream);
    compute(rounds, data_stream, weight_stream, result_stream);
    store_result(rounds, result, result_stream);
}