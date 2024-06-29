import argparse
import serial
import time
import struct

# Constants
FIRMWARE_UPDATE_PACKET_SIZE = 128
COM_PROTO_MSG_TYPE_FWUG_START = 0x01
COM_PROTO_MSG_TYPE_FWUG_DATA = 0x02
COM_PROTO_MSG_TYPE_FWUG_STATUS = 0x03
COM_PROTO_MSG_TYPE_FWUG_CANCEL = 0x04

COM_PROTO_MSG_TYPE_OP_RESULT = 0x08

# Operation results
COM_PROTO_OP_RESULT_NO_ERR = 0x00
COM_PROTO_OP_RESULT_GENERIC_ERR = 0xE1
COM_PROTO_OP_RESULT_CRC_ERR = 0xE2
COM_PROTO_OP_RESULT_AUTH_ERR = 0xE3
COM_PROTO_OP_RESULT_UNKNOWN_MSG_ERR = 0xE4

def send_message_via_serial(message, port='COM9', baudrate=115200, interval=0.01, max_wait_time=3):
    # Open serial port
    ser = serial.Serial(port, baudrate)

    # Create the buffer of 256 bytes. This depends on the packet size that we have to send to the bootloader.
    # Currently only fixed size packets are supported (256 - Depends on bootloader firmware).
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
    total_wait_time = 0

    # Wait for the response
    response = bytearray()
    while total_wait_time < max_wait_time:
        if ser.in_waiting > 0:
            response.extend(ser.read(ser.in_waiting))
            break
        time.sleep(interval)
        total_wait_time += interval

    # Print the response in hex format for verification
    if response:
        print("Response (Hex):", response.hex())
    else:
        print("No response received within the maximum wait time")

    # Close serial port
    ser.close()
    return response

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
    def __init__(self, com_port='COM9', baud_rate=115200, file_path=None):
        self.buffer = bytearray(256)
        self.com_port = com_port
        self.baud_rate = baud_rate
        self.file_path = file_path

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

    def parse_fwug_response(self, response, packet_number_sent):
        # Handle any exceptions
        if not response:
            print("No response received")
            return False
        # Check the message type
        if response[0] == COM_PROTO_MSG_TYPE_FWUG_STATUS:
            if len(response) != 8:
                print("Invalid response length")
                return False
            # Parse the response
            op_result, is_active, packets_received = struct.unpack('>BBH', response[2:6]) # we care about bytes: 2, 3, 4, 5
            # Reverse endianess of packets_received
            packets_received = (packets_received >> 8) | ((packets_received & 0xFF) << 8)
            print(f"FWUG_STATUS: op_result={op_result}, is_active={is_active}, packets_received={packets_received}")
            if op_result == COM_PROTO_OP_RESULT_NO_ERR and packets_received == packet_number_sent + 1:
                print("Firmware update message successful")
            else:
                print("Firmware update message failed")
            # Return True if the operation result is no error and the packets received is equal to the packet number sent + 1
            return (op_result == COM_PROTO_OP_RESULT_NO_ERR and packets_received == packet_number_sent + 1)
        elif response[0] == COM_PROTO_MSG_TYPE_OP_RESULT:
            # Parse the response
            op_result = response[2]
            print(f"OP_RESULT: op_result={op_result}")
            print("Firmware update message failed")
            # Return False if the operation result message was returned instead of the firmware update status message
            return False
        
    # Function to perform firmware update
    def perform_firmware_update(self):
        packet_number = -1
        # Start firmware update
        start_msg = self.create_fwug_start_msg()
        print("FWUG_START Message:", start_msg)
        response = send_message_via_serial(start_msg, self.com_port, self.baud_rate, 5, 10)
        if not self.parse_fwug_response(response, packet_number):
            # If firmware update start failed, send a cancel message and return
            cancel_msg = self.create_fwug_cancel_msg()
            send_message_via_serial(cancel_msg, self.com_port, self.baud_rate)
            print("Firmware update failed. Attempting to cancel... (and exiting)")
            return
        else:
            packet_number += 1
            print("Firmware update started")
        
        # Open the binary file and send the data in chunks of FIRMWARE_UPDATE_PACKET_SIZE bytes
        with open(self.file_path, 'rb') as f:
            while True:
                data_chunk = f.read(FIRMWARE_UPDATE_PACKET_SIZE)
                if not data_chunk:
                    break
                if len(data_chunk) < FIRMWARE_UPDATE_PACKET_SIZE:
                    data_chunk += b'\xFF' * (FIRMWARE_UPDATE_PACKET_SIZE - len(data_chunk))
                data_msg = self.create_fwug_data_msg(packet_number, data_chunk)
                for i in range(3):
                    print(f"Sending packet {packet_number}...")
                    response = send_message_via_serial(data_msg, self.com_port, self.baud_rate)
                    if self.parse_fwug_response(response, packet_number):
                        packet_number += 1
                        break
                    
                    if i < 3:
                        print(f"Retrying packet {packet_number}...")

                    # After 3 retries, return
                    if i == 2:
                        print("Firmware update failed. Exiting...")
                        return

if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description='Send firmware update via serial.')
    parser.add_argument('file', metavar='FILE', help='Path to binary file')
    parser.add_argument('--port', default='COM9', help='Serial port')
    parser.add_argument('--baudrate', type=int, default=115200, help='Baud rate')
    args = parser.parse_args()

    # Example binary file path
    binary_file_path = args.file
    com_port = args.port
    baud_rate = args.baudrate

    # --- Initiate firmware update ---
    # Create the firmware update factory
    fwug_factory = FirmwareUpdateFactory(com_port, baud_rate, binary_file_path)
    # Perform firmware update
    fwug_factory.perform_firmware_update()
