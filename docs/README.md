This folder contains all the relevant documents for each project.

# bootloader_architecture.drawio
![Bootloader Architecture](bootloader_architecture.drawio.svg)

Provides an architectural description of the bootloader, indicating the different modules and their interconnection. (Software design of the bootloader)

# bootloader_boot_sequence.drawio
![Bootloader sequence](bootloader_boot_sequence.drawio.svg)

This drawio drawing provides an in-depth description on the bootloader sequence, from the device boot state, to the actual boot of the application OR until it reaches the bootloop/recover state.

# bootloader_state_machine_diagram.drawio
![Bootloader state machine](bootloader_state_machine_diagram.drawio.svg)

This drawio diagram provides a high level overview of the bootloader state machine, that can be found in main.c.
This diagram will present the different state machine states and the transitions based on the events/results of each state's handler. Each state machine handler is a different state machine state and based on that and its output, the next state machine handler to be executed is decided.