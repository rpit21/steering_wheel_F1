@mainpage F1 Steering Wheel — Firmware & Simulator
@tableofcontents
@brief Comprehensive documentation for the firmware and host simulator of the F1 Steering Wheel project.

---

# F1 Steering Wheel — Firmware & Simulator
The **F1 Steering Wheel** project focuses on designing and developing the firmware of a Formula 1–style steering wheel Board.

It also includes a **PC-based simulator** for early-stage testing using **SDL2** (for graphics) and **SocketCAN** (for CAN bus communication).

## 🎯 Project Overview
The goal of this project is to design and build the electronic board and firmware for a Formula 1 style steering wheel. This simulator allows for the development and testing of the firmware logic on a host PC before deploying it to the target hardware (NXP S32K).

The steering wheel needs to:
* Read 4 digital buttons (UP, DOWN, DRS, Pit Limiter).
* Read 2 analog inputs (Clutch position, 10-position Rotary Switch).
* Communicate with the main ECU via CAN bus, sending input states and receiving vehicle data.
* Display information (Temperatures, Statuses) on a 128x64 monochrome display.
* Control 2 status LEDs.


## 🧩 Dual BSP Design 
This repository contains the **firmware** (Target s32k) and the **PC simulator** (host) for an F1 “Steering Wheel” board featuring different drivers for:

- Buttons,
- Rotary Switch,
- Clutch Signal.
- Display (TFT LCD),
- CAN Interface.

The key idea is to keep **the same application code** and **the same drivers**, changing only the **HAL (Hardware Abstraction Layer)** depending on the target:
- `host_pc` → runs on LINUX UBUNTU to simulate hardware (keyboard/console/SDL2);
- `target_s32k` → runs on NXP S32K MCU (integration and linker WIP).

---

## 🧱 Current Status (October 28, 2025)
The host PC simulator currently implements the following:

* **Graphical Display:** An SDL2 window simulates the 128x64 monochrome display.

* **Basic Drawing:** Functions for clearing the screen, drawing rectangles (filled and outline), and drawing basic text/numbers using an embedded 5x7 font are implemented (`drivers/display.c`).

* **Buttons Input:** Keyboard keys '1' through '4' simulate the four digital buttons. Input reading uses Linux terminal functions in non-blocking mode (`hal/host_pc/hal_buttons_host.c`) and includes debouncing logic (`drivers/buttons.c`). Visual feedback for button presses is shown on the SDL display.

* **ADC Input Simulation:** Rotary switch and clutch inputs are currently simulated with automatically cycling values (defined on `adc_data.csv`) in `hal/host_pc/hal_adc_host.c`.

* **CAN Communication:**
    * **Sending:** The simulator sends the `SteeringWheel_Status` CAN message (ID `0x101`) whenever the state of the buttons, simulated rotary, or simulated clutch changes. The payload is packed according to the `steering_wheel.dbc` specification in `drivers/can.c`. Communication occurs over a virtual CAN interface (`vcan0`) using SocketCAN (`hal/host_pc/hal_can_host.c`).

    * **Receiving:** Basic non-blocking CAN frame reception is implemented (`hal_can_receive`, `canbus_process_incoming`),and the logic to *decode* the `ECU_Status_Display` message (`0x201`) it is implemented in `drivers/can.c`.
    
* **ECU Simulation (Python):** A separate Python script (`ecu_sim.py`, located in the project root) acts as the simulated ECU. It:
    * Listens on the `vcan0` interface.
    * Uses the `cantools` library to decode incoming `0x101` messages based on `steering_wheel.dbc`.
    * Simulates basic vehicle logic (gear changes based on UP/DOWN buttons, Pit Limiter toggle, temperature simulation).
    * Encodes and sends the `ECU_Status_Display` message (`0x201`) periodically, containing the simulated vehicle state.

---

## 🗂️ Folder Structure (simplified)
```bash
FIRMWARE/
    app/
        app_main.c|.h
    drivers/
        buttons.c|.h
        can.c|.h
        clutch.c|.h
        display.c|.h
        rotary_switch.c|.h
        TFT_LCD.c|.h
    hal/
        hal_adc.h
        hal_gpio.h
        hal_can.h
        hal_delay.h
        hal_display.h
        hal_lcd.h
        hal_spi.h
        hal.h                     
        host_pc/
            hal_gpio_host.c
            hal_delay_host.c
            hal_display_host.c
            hal_lcd_host.c
            hal_adc_host.c
            hal_can_host              
    test/
        button_test.c, button_test.h
        adc_test.c, adc_test.h
        can_test.c, can_test.h
        display_demo.c, display_demo.h
        adc_data.csv
    ADC_DEVELOPMENT.md
    BUTTON_DEVELOPMENT.md
    CAN_DEVELOPMENT.md
    Doxyfile                                # Configuration for Doxygen documentatio
    ecu_sim.py                              # Python ECU simulator (SocketCAN + cantools)
    main.c
    Makefile                                # build host (sim) and HW placeholder

```

## 🧩 Code Structure

The firmware follows a layered architecture for better organization, testability, and portability:

* **`app/`**: Contains the main application logic (`app_main.c`). This layer orchestrates the drivers and implements the high-level behavior of the steering wheel.


* **`drivers/`**: Contains device drivers that provide a higher-level interface to peripherals, handling tasks like debouncing (`buttons.c`), drawing primitives (`display.c`), processing analog inputs (`clutch.c`), and managing CAN message packing/unpacking (`canbus.c`). Drivers use the HAL.


* **`hal/`**: The Hardware Abstraction Layer.
    * Contains generic header files (`hal_*.h`) defining the interface for each peripheral (buttons, display, ADC, CAN).
    * Contains subdirectories for each target platform:
        * **`hal/host_pc/`**: Implementation of the HAL functions for the host PC simulation (using SDL2, Linux terminal I/O, SocketCAN).
        * **`hal/target_s32k/`**: (Currently contains placeholders) Implementation of the HAL functions for the NXP S32K microcontroller, using its specific SDK/registers.


* **`test/`**: Contains isolated test code for specific modules (currently compiled but not linked into the main simulator executable).


* **`main.c`**: The C standard entry point. Initializes the program and calls `app_main()`.


* **`Makefile`**: The build script used to compile and link the code using `make`.


* **`steering_wheel.dbc`**: The CAN database file defining the messages and signals used for communication.


* **`ecu_sim.py`**: Python Script thar simulates an ECU for receiving the status of the Steering Wheel and sending a feedback by CAN

* **`Doxyfile`**: File for the compilation of Doxygen for generating code's documentation.

---

## 🔧 Prerequisites 

### LINUX (Ubuntu):
To compile and run this simulator is highly recommended setup a Linux environment with Ubuntu.

1) Install natively Ubuntu or use a Virtual Machine (Virtual Box)

2) Install toolchain and libraries by opening a terminal
```Bash
sudo apt update
sudo apt upgrade
sudo apt install build-essential pkg-config can-utils libsdl2-dev python3-pip python3-venv
```
* `build-essential`: Installs `gcc`, `make`, and other core development tools.
* `pkg-config`: Used by the Makefile to find SDL2 flags.
* `can-utils`: Provides command-line tools like `candump` for interacting with CAN interfaces.
* `libsdl2-dev`: Development headers and libraries for SDL2 graphics.
* `python3-pip`: Package installer for Python.
* `python3-venv`: Tool for creating Python virtual environments.

3) Navigate to the `F1_Project` folder and set up Python Virtual Environment and Packages:
```bash
python3 -m venv .venv  #create an virtual environment
source .venv/bin/activate #Activate the environment (Do it every time)
pip pip install python-can cantools #Install Python Libraries
```
4) Git configuration 
>Configure your Git identity (use the same name/email as your GitHub account):

```bash
    git config --global user.name "Your GitHub Username"
    git config --global user.email "your_github_email@example.com"
```


>Configure Git to handle line endings correctly when interacting with files potentially edited on Windows

```bash
    git config --global core.autocrlf input
```


>Configure Git to ignore file permission changes, which can cause noise when working on mounted Windows filesystems:

```bash
        git config --global core.fileMode false
```

### WINDOWS (VSCode + MSYS2/UCRT64)
1) **MSYS2** installed in `C:\msys64`  
2) Open the **MSYS2 UCRT64** terminal in VSCode (custom profile):
     - User settings → `settings.json`  
         ```json
         "terminal.integrated.profiles.windows": {
             "MSYS2 UCRT64": {
                 "path": "C:\\msys64\\usr\\bin\\bash.exe",
                 "args": ["-l","-i"],
                 "env": { "MSYSTEM": "UCRT64", "CHERE_INVOKING": "1" },
                 "icon": "terminal-bash"
             }
         },
         "terminal.integrated.defaultProfile.windows": "MSYS2 UCRT64"
         ```
3) Install toolchain and libraries (from UCRT64):
     ```bash
     pacman -Syu
     # restart UCRT64 if prompted
     pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf make
     ```
     (SDL2 will be useful when the display via SDL is completed; for now it’s optional)
4) Verify:
     ```bash
     echo $MSYSTEM           # UCRT64
     which gcc               # /ucrt64/bin/gcc
     which make              # /ucrt64/bin/make
     ```
5) IntelliSense (optional but recommended): create `.vscode/c_cpp_properties.json` and point to UCRT64 includes (gcc and standard headers).

---

## ▶️ Build and Run on PC (host simulator)

Enter the `FIRMWARE/` folder and run:
```bash
cd FIRMWARE
make sim   # build host (default)
make hw    # placeholder for S32K (linker and startup WIP)
make run   # to run the code
make clean # clean up
```

**Only** In case that you are going to use any application with **CAN** follow the next steps :
> ONLY FOR UBUNTU OS!!!!!

1. Create the Virtual CAN Network:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```


2. Compilate the FIRMWARE and execute it

```bash
make sim
make run
```


3. Executes the simulated ECU --> Python script (ecu_sim.py)

> In a new terminal activate the virtual environment and executes the .py:

```bash
source .venv/bin/activate
python ecu_sim.py
```

4. (OPTIONAL) Monitor the CAN BUS

> Open a new terminal

```bash
candump vcan0
```


The current test program runs `app_main()` (see `main.c`)

---

### DOCUMENTATION
```
button_test() → buttons_init() → hal_buttons_init()

loop:
    raw = hal_buttons_read()   // raw read (1–4 from keyboard)
    stable = buttons_getStable() // debounce filter
    printf(raw, stable)
```

For detailed notes on each driver implementation, see:

@ref ADC_DEVELOPMENT "ADC Development Notes"

@ref BUTTON_DEVELOPMENT "Button Driver Notes"

@ref CAN_DEVELOPMENT "CAN Driver Notes"

@ref DISPLAY_DEVELOPMENT "Display Driver Notes"

These Markdown documents describe each module’s design, behavior, and testing strategy.