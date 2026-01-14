"""
@file ecu_sim.py
@brief ECU simulation script for CAN communication testing.

@details
This script emulates the behavior of an Engine Control Unit (ECU) for testing
the Steering Wheel firmware over a virtual CAN interface (`vcan0`).

It performs the following operations in a continuous loop:
1. Listens for incoming messages from the Steering Wheel (ID 0x101).
2. Decodes and prints the received steering data using the DBC file.
3. Periodically transmits an ECU status message (ID 0x201) containing
   simulated temperature values (Temp1, Temp2).

@note
This script requires:
- Python packages: `python-can`, `cantools`
- A virtual CAN interface (`vcan0`) created in the system.
  Example setup:
      sudo modprobe vcan
      sudo ip link add dev vcan0 type vcan
      sudo ip link set up vcan0

@usage
Run this script alongside the Steering Wheel firmware running on PC
(simulated with SocketCAN). The two will exchange CAN frames in real-time.
"""

import cantools, can, time, random

# --------------------------------------------------------------------------
# Initialization
# --------------------------------------------------------------------------

# Load the CAN database (.dbc file) containing message definitions
db = cantools.database.load_file("drivers/steering_wheel.dbc")

# Initialize virtual CAN bus (SocketCAN interface)
#bus = can.interface.Bus(channel='vcan0', bustype='socketcan')
bus = can.interface.Bus(channel='can0', bustype='socketcan')



# --- Initialize "slow" temperature simulation ---
temp1 = random.uniform(-10, 10)     # starting point (°C)
temp2 = random.uniform(110, 130)
target_temp1 = temp1
target_temp2 = temp2
update_counter = 0

gear = 0
pit_active = 0
drs_status = 0
prev_buttons = 0  # save the previus state (bitmask)


# --------------------------------------------------------------------------
# Main simulation loop
# --------------------------------------------------------------------------
while True:

    # --- Receive and decode steering wheel status ---
    msg = bus.recv(0.1) # Listen for 100 ms for incoming messages
    if not msg:
        continue

    if msg and msg.arbitration_id == 0x101:
        # Decode message using DBC definitions
        data = db.decode_message('SteeringWheel_Status', msg.data)
        print("Steering Wheel:", data)

        #Detection of edges
        buttons = 0
        if data["Button_UP"]:
            buttons |= 0x01
        if data["Button_DOWN"]:
            buttons |= 0x02
        if data["Button_DRS"]:
            buttons |= 0x04
        if data["Button_PitLimiter"]:
            buttons |= 0x08

        # Rising edges 0 → 1
        rising_edges = buttons & (~prev_buttons)

        #Dectention both edges (change detectiio)
        changes = buttons ^ prev_buttons

        # --- Gear logic ---
        if changes & 0x01:  # Button_UP pressed
            gear = min(gear + 1, 9)
        if changes & 0x02:  # Button_DOWN pressed
            gear = max(gear - 1, 0)

        # --- PIT logic ---
        if rising_edges & 0x08:
            pit_active = 0 if pit_active else 1

         # --- DRS logic ---
        if rising_edges & 0x04:
            drs_status = 0 if drs_status else 1

        # Update previous state
        prev_buttons = buttons


         # --- Slow temperature dynamics (simulated sensor behavior) ---
        update_counter += 1
        if update_counter % 20 == 0:  # every 1 second (20 loops)
            # pick a new target temperature with small random drift
            target_temp1 = random.uniform(-10, 15)
            target_temp2 = random.uniform(110, 145)

        # Gradually move towards target (like a first-order filter)
        temp1 += (target_temp1 - temp1) * 0.05  # 5% convergence each cycle
        temp2 += (target_temp2 - temp2) * 0.05

        # Add small random sensor noise
        temp1 += random.uniform(-0.05, 0.05)
        temp2 += random.uniform(-0.05, 0.05)



        # --- Simulate ECU temperature data ---
        ecu_status = db.get_message_by_name('ECU_Status_Display')
        payload = ecu_status.encode({

            'Temp1': temp1,   # Temperature sensor 1 (°C)
            'Temp2': temp2,    # Temperature sensor 2 (°C)
            "PitLimiter_Active": pit_active,
            "DRS_Status": drs_status,
            "LED2_PitLimiter": pit_active,
            "LED1_Temperature": 1 if temp2 > 130 else 0,
            "Gear_Actual": gear,
            "Clutch_Feedback": data["ClutchValue"],
            "Rotary_Feedback": data["RotaryPosition"],
        })
    
        # Transmit simulated ECU status frame
        bus.send(can.Message(arbitration_id= 0x201, data=payload, is_extended_id=False))

    # Wait 50 ms before next cycle
    time.sleep(0.050)