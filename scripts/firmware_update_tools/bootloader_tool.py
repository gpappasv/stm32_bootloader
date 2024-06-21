import serial

# Open serial port
ser = serial.Serial('COM9', 115200)

# Send data
message = "Hello world!"
ser.write(message.encode())

# Close serial port
ser.close()