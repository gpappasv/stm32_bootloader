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
import hashlib

CRC_SIZE_BYTES     = 4
VERSION_SIZE_BYTES = 4
SHA256_SIZE_BYTES  = 32
FOOTER_SIZE_BYTES = CRC_SIZE_BYTES + VERSION_SIZE_BYTES + SHA256_SIZE_BYTES

class CRC:
    @staticmethod
    def compute_crc32(data):
        """
        Computes the CRC32 checksum for the given data.
        
        Args:
            data (bytearray): The binary data to compute the checksum for.

        Returns:
            int: The CRC32 checksum.
        """
        crc = 0xFFFFFFFF
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 1:
                    crc = (crc >> 1) ^ 0xEDB88320
                else:
                    crc >>= 1
        return crc ^ 0xFFFFFFFF

class BinaryAnalyzer:
    def __init__(self, binary_data, linker_script_content):
        """
        Initializes the BinaryAnalyzer with the given binary data and linker script content.
        Appends padding and an empty footer to the binary data.
        
        Args:
            binary_data (bytearray): The binary data to analyze and modify.
            linker_script_content (str): The content of the linker script.
        """
        self.binary_data = binary_data
        self.linker_script_content = linker_script_content
        self.footer_size = FOOTER_SIZE_BYTES  # Footer size includes CRC32, version (4 bytes)
        self.version = bytearray([0, 0, 0, 0])  # Example version (Major, Minor (2 bytes), Patch)

        # Perform padding and footer appending during initialization
        self._calculate_img_size()
        self._append_padding()
        self._append_footer()

    def _calculate_img_size(self):
        """
        Calculates the image size using the linker script content.
        
        Raises:
            ValueError: If the linker script is invalid.
        """
        symbols = ["__flash_app_start__", "__flash_app_end__"]
        symbol_values = {}

        for symbol in symbols:
            match = re.search(rf"{symbol}\s*=\s*(0x[0-9A-Fa-f]+|\w+);", self.linker_script_content)
            if match:
                value = match.group(1)
                try:
                    symbol_values[symbol] = int(value, 16) if value.startswith('0x') else value
                except ValueError:
                    symbol_values[symbol] = value
            else:
                symbol_values[symbol] = None

        if symbol_values["__flash_app_start__"] is not None and symbol_values["__flash_app_end__"] is not None:
            self.size = symbol_values["__flash_app_end__"] - symbol_values["__flash_app_start__"] + 1
        else:
            raise ValueError("Invalid linker script")

    def _append_padding(self):
        """
        Appends padding to the binary data to match the calculated image size.
        """
        target_size = self.size - self.footer_size
        current_size = len(self.binary_data)
        print(f"Current binary data size: {current_size} bytes")
        print(f"Target size after padding: {target_size} bytes")
        
        if current_size < target_size:
            padding_size = target_size - current_size
            print(f"Appending {padding_size} bytes of padding")
            padding = bytearray([0xFF] * padding_size)
            self.binary_data.extend(padding)
        else:
            print("No padding needed")

    def _append_footer(self):
        """
        Appends an empty footer to the binary data.
        """
        print(f"Appending {self.footer_size} bytes for the footer")
        self.binary_data.extend(bytearray(self.footer_size))  # Reserve space for the footer

    def add_crc_to_footer(self, crc32):
        """
        Adds the CRC32 value to the footer of the binary data.
        
        Args:
            crc32 (int): The CRC32 checksum to add to the footer.
        """
        print(f"\nAdding CRC-32 to the footer: {crc32:08X}")
        footer_start = len(self.binary_data) - self.footer_size
        self.binary_data[footer_start:footer_start + CRC_SIZE_BYTES] = crc32.to_bytes(4, "little")

    def add_version_to_footer(self, version_major, version_minor, version_patch):
        """
        Adds the version to the footer of the binary data.
        
        Args:
            version_major (int): The major version to add to the footer.
            version_minor (int): The minor version to add to the footer.
            version_patch (int): The patch version to add to the footer.
        """
        # Print the version information
        print(f"\nAdding version to footer: v{version_major}.{version_minor}.{version_patch}")
        footer_start = len(self.binary_data) - self.footer_size + CRC_SIZE_BYTES  # Offset for version after CRC32
        self.binary_data[footer_start] = version_major
        self.binary_data[footer_start + 1] = (version_minor >> 8) & 0xFF
        self.binary_data[footer_start + 2] = version_minor & 0xFF
        self.binary_data[footer_start + 3] = version_patch

    def add_sha256_to_footer(self):
        """
        Adds the SHA256 hash to the footer of the binary data.
        """
        sha256_hash = hashlib.sha256(self.binary_data[:-self.footer_size]).digest()
        footer_start = len(self.binary_data) - self.footer_size + CRC_SIZE_BYTES + VERSION_SIZE_BYTES
        self.binary_data[footer_start:footer_start + 32] = sha256_hash

    def commit_to_file(self, file_path):
        """
        Commits the modified binary data to the given file path.
        
        Args:
            file_path (str): The path of the file to write the binary data to.
        """
        with open(file_path, "wb") as binary_file:
            binary_file.write(self.binary_data)

def main():
    if len(sys.argv) != 6:
        print(f"Usage: {sys.argv[0]} <binary_file> <linker_script> <version_major> <version_minor> <version_patch>")
        sys.exit(1)

    binary_file_path = sys.argv[1]
    linker_script_path = sys.argv[2]
    version_major = int(sys.argv[3])
    version_minor = int(sys.argv[4])
    version_patch = int(sys.argv[5])

    try:
        with open(binary_file_path, "rb") as binary_file:
            binary_data = bytearray(binary_file.read())

        with open(linker_script_path, "r", encoding='utf-8') as linker_file:
            linker_script_content = linker_file.read()

        # Initialize and analyze the binary provided based on the linker script
        analyzer = BinaryAnalyzer(binary_data, linker_script_content)

        # Calculate CRC32 on the initialized binary data
        crc32 = CRC.compute_crc32(analyzer.binary_data[:-FOOTER_SIZE_BYTES])  # Exclude last 40 bytes for footer

        """
        Add data to the binary. The data that will be added will be:
        1. CRC32 (4 bytes)
        2. Version (Major, Minor, Patch) (4 bytes)
        3. SHA256 (32 bytes)
        They will all exist in the end of the binary.
        """
        # Set version in the footer
        analyzer.add_version_to_footer(version_major, version_minor, version_patch)
        # Add CRC32 to the footer
        analyzer.add_crc_to_footer(crc32)
        # Add SHA256 to the footer
        analyzer.add_sha256_to_footer()

        # Commit all the information to the binary (actually re-write the binary)
        analyzer.commit_to_file(binary_file_path)

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
