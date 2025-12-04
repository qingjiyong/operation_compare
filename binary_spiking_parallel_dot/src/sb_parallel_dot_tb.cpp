/*
test bench for sb_parallel_dot
*/

#include <iostream>
#include <fstream>
#include <string>
#include "config.h"
#include "sb_parallel_dot.h"

const std::string parent_dir = "/home/percy/data/HDL/sb_parallel_dot/src/testbench";
const std::string data_file = parent_dir + "/data.bin";
const std::string weight_file = parent_dir + "/weights.bin";
const std::string result_file = parent_dir + "/result.bin";

void load_data(const std::string &filename, Pack_Data *data, int rounds){
    std::ifstream data_file(filename, std::ios::binary);
    if(!data_file.is_open()){ 
        std::cerr << "Error opening data file: " << filename << std::endl;
        return;
    }
    //buffer to hold one entry
    const int bytes_per_entry = 64; 
    uint8_t buffer[bytes_per_entry];
    /*for(int r = 0; r < rounds; r++){
        data_file.read(reinterpret_cast<char*>(&data[r]), sizeof(Pack_Data));
    }*/

    for(int r = 0; r < rounds; r++){
        data_file.read(reinterpret_cast<char*>(buffer), bytes_per_entry);
        
        Pack_Data temp = 0;
        for(int i = 0; i < bytes_per_entry; i++) {
            temp |= (Pack_Data(buffer[i]) << (i * 8));
        }
        data[r] = temp;
    }
    data_file.close();
}

void load_weight(const std::string &filename, Pack_Weight *weight, int rounds){
    std::ifstream weight_file(filename, std::ios::binary);
    if(!weight_file.is_open()){ 
        std::cerr << "Error opening weight file: " << filename << std::endl;
        return;
    }
    
    /*for(int r = 0; r < rounds; r++){
        weight_file.read(reinterpret_cast<char*>(&weight[r]), sizeof(Pack_Weight));
    }*/
    //buffer to hold one entry
    const int bytes_per_entry = 64; 
    uint8_t buffer[bytes_per_entry];
    for(int r = 0; r < rounds; r++){
        weight_file.read(reinterpret_cast<char*>(buffer), bytes_per_entry);
        
        Pack_Weight temp = 0;
        for(int i = 0; i < bytes_per_entry; i++) {
            temp |= (Pack_Weight(buffer[i]) << (i * 8));
        }
        weight[r] = temp;
    }
    weight_file.close();
}

void load_result(const std::string &filename, Result_t *result, int rounds){
    std::ifstream result_file(filename, std::ios::binary);
    if(!result_file.is_open()){ 
        std::cerr << "Error opening result file: " << filename << std::endl;
        return;
    }
    
    for(int r = 0; r < rounds; r++){
        result_file.read(reinterpret_cast<char*>(&result[r]), sizeof(Result_t));
    }
    result_file.close();
}

int main(){
    const int rounds = 1024; // number of test rounds
    const int MAX_DEPTH = rounds<1024?1024:rounds;
    // Allocate memory for data, weight, and result
    Pack_Data data[MAX_DEPTH];
    Pack_Weight weight[MAX_DEPTH];
    Result_t result[MAX_DEPTH];
    Result_t expected_result[MAX_DEPTH];
    /*Pack_Data* data = new Pack_Data[rounds];
    Pack_Weight* weight = new Pack_Weight[rounds];
    Result_t* result = new Result_t[rounds];
    Result_t* expected_result = new Result_t[rounds];*/

    // Load input data and weights from binary files
    load_data(data_file, data, rounds);
    load_weight(weight_file, weight, rounds);
    load_result(result_file, expected_result, rounds);

    // Call the sb_parallel_dot function
    sb_parallel_dot(rounds, data, weight, result);

    // Load expected results from binary file
    //Result_t expected_result[rounds];
    

    // Verify results
    bool all_passed = true;
    for(int r = 0; r < rounds; r++){
        if(result[r] != expected_result[r]){
            std::cout << "Mismatch at round " << r << ": expected " 
                      << expected_result[r] << ", got " << result[r] << std::endl;
            all_passed = false;
        }
    }

    if(all_passed){
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << "Some tests failed." << std::endl;
    }

    return all_passed ? 0 : 1;
}