import numpy as np

def generate_data(entries, data_bits, weight_bits, num_samples):
    data = np.random.randint(-2**(data_bits-1), 2**(data_bits-1), size=(num_samples, entries))
    weights = np.random.randint(-2**(weight_bits-1), 2**(weight_bits-1), size=(num_samples, entries))
    result = np.sum(data * weights, axis=1)
    return data, weights, result

def convert_to_int(value, bits):
    if value < 0:
        value = (1 << bits) + value
    return value

def save_to_file(filename, data, bits):
    """Packs data into bytes and saves to a binary file."""
    if bits >= 8:
        # For data types of 8 bits or more, we can write them directly.
        byte_len = (bits + 7) // 8
        with open(filename, 'wb') as f:
            for val in data.flatten():
                f.write(int(val).to_bytes(byte_len, byteorder='little', signed=True))
    else:
        # For data types less than 8 bits (e.g., 4-bit), we need to pack them.
        packed_bytes = bytearray()
        buffer = 0
        buffer_len = 0
        
        for val in data.flatten():
            # Convert to two's complement for packing
            mask = (1 << bits) - 1
            val_bits = int(val) & mask
            
            buffer |= (val_bits << buffer_len)
            buffer_len += bits
            
            while buffer_len >= 8:
                packed_bytes.append(buffer & 0xFF)
                buffer >>= 8
                buffer_len -= 8
        
        # Write any remaining bits in the buffer
        if buffer_len > 0:
            packed_bytes.append(buffer)
            
        with open(filename, 'wb') as f:
            f.write(packed_bytes)


if __name__ == "__main__":
    ENTRIES = 128
    DATA_BITS = 4
    WEIGHT_BITS = 4
    RESULT_BITS = 16
    NUM_SAMPLES = 1024

    data, weights, result = generate_data(ENTRIES, DATA_BITS, WEIGHT_BITS, NUM_SAMPLES)
    print("Generated Data:\n", data[0])
    print("Generated Weights:\n", weights[0])
    print("Expected Result:\n", result[0])
    save_to_file('./data.bin', data, DATA_BITS)
    save_to_file('./weights.bin', weights, WEIGHT_BITS)
    save_to_file('./result.bin', result.reshape(-1, 1), RESULT_BITS)
    print("all results: ", result)

