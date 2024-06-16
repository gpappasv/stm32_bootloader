# create_dfu_image.py
This script is being used to post process a firmware binary file, to make it compatible with the bootloader firmware update requirements.
More information for that, can be found inside the python script.
To utilize it for a specific binary, run the following command:

```bash
python create_dfu_image.py </path/to/bin> <path/to/linker_script> <version_major> <version_minor> <patch>
```

After the execution of this script, a new binary will be created that will be ready to be flashed to the board, and be compatible with out bootloader.
Also, a yaml file will be produced that will contain information related to the final binary. (e.g. the crc, sha256 hash, version of the binary).