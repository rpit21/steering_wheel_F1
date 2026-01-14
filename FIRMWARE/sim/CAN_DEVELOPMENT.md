# Subsystem ADC - Step-by-Step Guide

## Layered Architecture

The ADC system is divided in the next layer:

1. **HAL (hal\hal_can_*):**
   Manage the hardware layer (s32k) or create the simulated environment (host PC)

2. **Drivers (drivers\can.c):**
   Have all the functions for processing the ADC signal for specific peripheral.
   You will find 2 drivers: *clutch* and *rotary_switch*

3. **Test (test\can_test.c)**
    Contains all high level code for testing the drivers and hal

4. **ECU simulator (ecu_sim.py)**
    Is a python scripts that behaives like an Real ECU to recept the incoming can message from our Firmware and send it back the corresponding messages. 

5. **Data Base CAN (steering_wheel)**
    The CAN database file defines the messages structure. Is a dictionary that translates raw bytes into a significant value 


## Steps to use and simulate CAN 

1. Create the Virtual CAN Network

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 restart-ms 100
sudo ip link set can0 up

candump can0
# en otra terminal
cansend can0 123#1122334455667788

```


These comands simulates the CAN network (vcan0) where my BSP firmware and my ECU (ecu_sim.py) comunicates each other. 

> You have to execute once 

You can verify the network with the next command:

```bash
ip link show vcan0
```

2. Compilate the FIRMWARE and execute it

```bash
make sim
make run
```

3. Executes the simulated ECU --> Python script (ecu_sim.py)

In a new terminal activate the virtual environment and executes the .py:

```bash
source .venv/bin/activate
python ecu_sim.py
```

4. (OPTIONAL) Monitor

Open a new terminal

```bash
candump vcan0
```