"""This script is responsible for creating a DFU image from a binary file.
    The final binary file will be used to flash the firmware on the target device.
    The script will create a DFU image with the following structure:
    - DFU Image Prefix
    - Firmware Image

    The DFU Image Prefix is a 40-byte header that contains the following information:
    - CRC32 of the firmware image (4-bytes)
    - Firmware version (Major, Minor, Patch) (1-byte, 2-bytes, 1-byte)
    - SHA256 of the firmware image (32-bytes)

    The header of the DFU image is used by the bootloader to verify the integrity of the firmware image.
    The header of the DFU image does not participate in the CRC and SHA256 calculation and is not included in the size
    calculation.
"""
import re
import sys

'''
This function computes the CRC32 checksum of the given data.
The CRC32 checksum is calculated using the polynomial 0xEDB88320.
The function returns the CRC32 checksum of the data.
'''
def compute_crc32(data):
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xEDB88320
            else:
                crc >>= 1
    return crc ^ 0xFFFFFFFF

'''
This function calculates the CRC32 checksum of the given file.
The function returns the CRC32 checksum of the file.
'''
def calculate_file_crc32(file_path):
    with open(file_path, "rb") as file:
        data = bytearray(file.read())
        crc = compute_crc32(data)
        return crc

'''
This function calculates the size of the image using the linker script.
The function returns the size of the image.
'''
def calculate_img_size(linker_script_content):
    symbols = ["__flash_app_start__", "__flash_app_end__"]
    symbol_values = {}

    for symbol in symbols:
        match = re.search(rf"{symbol}\s*=\s*(0x[0-9A-Fa-f]+|\w+);", linker_script_content)
        if match:
            value = match.group(1)
            try:
                symbol_values[symbol] = int(value, 16) if value.startswith('0x') else value
            except ValueError:
                symbol_values[symbol] = value
        else:
            symbol_values[symbol] = None

    # Calculate the size of the image
    if symbol_values["__flash_app_start__"] is not None and symbol_values["__flash_app_end__"] is not None:
        size = symbol_values["__flash_app_end__"] - symbol_values["__flash_app_start__"]
        size += 1
    else:
        raise ValueError("Invalid linker script")

    return size

'''
This function appends padding to the data to match the given size.
'''
def append_padding(data, size):
    if len(data) < size:
        padding_size = size - len(data)
        padding = bytearray([0xFF] * padding_size)
        data.extend(padding)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <binary_file> <linker_script>")
        sys.exit(1)

    binary_file_path = sys.argv[1]
    linker_script_path = sys.argv[2]

    # calculate the binary file size, before adding the padding
    with open(binary_file_path, "rb") as binary_file:
        binary_file.seek(0, 2)
        binary_file_size = binary_file.tell()
        print(f"Binary file size: {binary_file_size} bytes")

    try:
        # Read and parse the linker script, to determine the size of the image
        with open(linker_script_path, "r") as linker_file:
            linker_script_content = linker_file.read()

        # Calculate the desired size of the image
        size = calculate_img_size(linker_script_content)

        # Append padding to the binary file, to match the desired size
        with open(binary_file_path, "r+b") as binary_file:
            data = bytearray(binary_file.read())
            append_padding(data, size - 40)
            binary_file.seek(0)
            binary_file.write(data)

        # calculate the new binary file size
        with open(binary_file_path, "rb") as binary_file:
            binary_file.seek(0, 2)
            binary_file_size = binary_file.tell()
            print(f"New binary file size: {binary_file_size} bytes")

        # Calculate the CRC32 checksum of the binary file
        crc = calculate_file_crc32(binary_file_path)
        print(f"\nCRC-32: {crc:08X}")

        # Append the calculated CRC32 checksum to the binary file (in the end)
        with open(binary_file_path, "a+b") as binary_file:
            binary_file.write(crc.to_bytes(4, "little"))

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
