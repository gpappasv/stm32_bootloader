from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
import sys

def pem_to_raw_coordinates(pem_file_path):
    with open(pem_file_path, "rb") as pem_file:
        public_key = serialization.load_pem_public_key(pem_file.read())
    
    public_numbers = public_key.public_numbers()
    x = public_numbers.x.to_bytes(32, 'big')
    y = public_numbers.y.to_bytes(32, 'big')
    
    return x + y

def raw_to_c_array(raw_bytes, output_header_path):
    hex_array = ', '.join(f'0x{byte:02X}' for byte in raw_bytes)
    
    header_content = f"""
/**
 * @file ecdsa_pub_key.h
 * @brief ECDSA public key in C array format
 * @version 0.1
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef ECDSA_PUB_KEY_H
#define ECDSA_PUB_KEY_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>

// --- constants -------------------------------------------------------------------------------------------------------
const uint8_t ecdsa_public_key[] = {{
    {hex_array}
}};

#endif // ECDSA_PUB_KEY_H
"""
    with open(output_header_path, "w") as header_file:
        header_file.write(header_content)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python extract_public_key.py <public_key.pem> <ecdsa_pub_key.h>")
        sys.exit(1)

    pem_file_path = sys.argv[1]
    output_header_path = sys.argv[2]

    raw_key = pem_to_raw_coordinates(pem_file_path)
    raw_to_c_array(raw_key, output_header_path)