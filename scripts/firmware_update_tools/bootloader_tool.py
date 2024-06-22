import argparse
import serial
import time
import struct

# Constants
FIRMWARE_UPDATE_PACKET_SIZE = 128
COM_PROTO_MSG_TYPE_FWUG_START = 0x01
COM_PROTO_MSG_TYPE_FWUG_DATA = 0x02
COM_PROTO_MSG_TYPE_FWUG_CANCEL = 0x04

def compute_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF  # Ensure CRC remains 16-bit
    return crc

class FirmwareUpdateFactory:
    def __init__(self):
        self.buffer = bytearray(256)
    
    def create_fwug_start_msg(self):
        msg_len = 2 + 2  # Total length: 2 bytes header + 2 bytes footer
        msg_header = struct.pack('BB', COM_PROTO_MSG_TYPE_FWUG_START, msg_len)
        msg_footer = struct.pack('H', 0)
        message = msg_header + msg_footer
        crc16 = compute_crc16(message[:-2])
        message = msg_header + struct.pack('>H', crc16)
        self.buffer[:len(message)] = message
        return self.buffer[:len(message)]
    
    def create_fwug_data_msg(self, packet_number, payload):
        if len(payload) != FIRMWARE_UPDATE_PACKET_SIZE:
            raise ValueError(f"Payload must be {FIRMWARE_UPDATE_PACKET_SIZE} bytes")
        
        msg_len = 2 + 2 + FIRMWARE_UPDATE_PACKET_SIZE + 2  # Header + packet number + payload + footer
        msg_header = struct.pack('BB', COM_PROTO_MSG_TYPE_FWUG_DATA, msg_len)
        packet_num = struct.pack('<H', packet_number)  # Big-endian packet number
        msg_footer = struct.pack('H', 0)  # Placeholder for CRC16
        
        message = msg_header + packet_num + payload + msg_footer
        crc16 = compute_crc16(message[:-2])
        message = msg_header + packet_num + payload + struct.pack('>H', crc16)
        
        self.buffer[:len(message)] = message
        return self.buffer[:len(message)]
    
    def create_fwug_cancel_msg(self):
        msg_len = 2 + 2  # 2 bytes header + 2 bytes footer
        msg_header = struct.pack('BB', COM_PROTO_MSG_TYPE_FWUG_CANCEL, msg_len)
        msg_footer = struct.pack('H', 0)
        message = msg_header + msg_footer
        crc16 = compute_crc16(message[:-2])
        message = msg_header + struct.pack('>H', crc16)
        
        self.buffer[:len(message)] = message
        return self.buffer[:len(message)]

def send_message_via_serial(message, port='COM9', baudrate=115200):
    # Open serial port
    ser = serial.Serial(port, baudrate)
    
    # Create the buffer of 256 bytes
    buffer_size = 256
    buffer = bytearray(buffer_size)
    
    # Copy message into the buffer
    buffer[:len(message)] = message
    
    # Fill the rest of the buffer with 0xFF
    for i in range(len(message), buffer_size):
        buffer[i] = 0xFF
    
    # Convert buffer to hex string
    hex_buffer = buffer.hex()
    
    # Print hex buffer for verification
    #print("Hex Buffer:", hex_buffer)
    ser.write(bytes.fromhex(hex_buffer))
    
    # Initialize variables for response handling
    max_wait_time = 15  # Maximum wait time in seconds
    check_interval = 0.001  # Interval to check for response in seconds
    total_wait_time = 0
    
    # Wait for the response
    response = bytearray()
    while total_wait_time < max_wait_time:
        if ser.in_waiting > 0:
            response.extend(ser.read(ser.in_waiting))
            break
        time.sleep(check_interval)
        total_wait_time += check_interval
    
    # Print the response in hex format for verification
    # if response:
    #     print("Response (Hex):", response.hex())
    # else:
    #     print("No response received within the maximum wait time")
    
    # Close serial port
    ser.close()

# Function to send firmware update data in chunks
def send_firmware_update(file_path, factory):
    with open(file_path, 'rb') as f:
        packet_number = 0
        while True:
            # Read 128 bytes of data
            data_chunk = f.read(FIRMWARE_UPDATE_PACKET_SIZE)
            if not data_chunk:
                break  # No more data to send
            
            # Create FWUG_DATA message
            data_msg = factory.create_fwug_data_msg(packet_number, data_chunk)
            print(f"Sending FWUG_DATA Message {packet_number}:")
            send_message_via_serial(data_msg)
            
            packet_number += 1

if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description='Send firmware update via serial.')
    parser.add_argument('file', metavar='FILE', help='Path to binary file')
    args = parser.parse_args()

    # Example binary file path
    binary_file_path = args.file

    # Create FirmwareUpdateFactory instance
    factory = FirmwareUpdateFactory()

    # Create FWUG_START message
    start_msg = factory.create_fwug_start_msg()
    print("FWUG_START Message:", start_msg)

    # Send FWUG_START message via serial
    send_message_via_serial(start_msg)

    # Send firmware update data in chunks
    send_firmware_update(binary_file_path, factory)

    # Create FWUG_CANCEL message
    cancel_msg = factory.create_fwug_cancel_msg()
    print("FWUG_CANCEL Message:", cancel_msg)

    # Send FWUG_CANCEL message via serial
    send_message_via_serial(cancel_msg)
