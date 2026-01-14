This folder contains all the necesary files to simulate the "Stearing Wheel Board" on Renode emulator.

## 1. What contains?
This folder is subdived in 3 subfolders:
- Binary
    Here you can find/put the build files from the compiler .elf that you want to try. 
- Board
    Inside youl will find all .repl files that describes the MCU and the Board.
- Scripts
    Contains a file .resc which have all the comands to prepare the renode simulator.

## 2. Installation
Until now, the way to use the simulator is necesary to have already downloaded Renode

Download this folder and put it inside the **Renode's folder** where it was installed.

## 3. How to Use it?
Open Renode

On the console, include the .resc file that contains all the necesary comands
`(monitor) i @renode_sim/Scripts/StearingBoard_test.resc`

> At this point the .resc file load the binary file that is the **Binary** folder. If you want to use other .elf, put the file in the Binary folder and chage with the respective file's name on the .resc

This create us a machine that contains all. To start the simulation:
`(s32k118_Board) start`

To interact with one of the buttons:
`(s32k118_Board) portA.Button1 press`

> You can use *release* or *toggle*. If you want to know more actions press tab.

To see a digital Output (In this case a Led)
`(s32k118_Board) logLevel -1 portB.LED1`

To pause simulation
`(s32k118_Board) pause`

To exit
`(s32k118_Board) quit`

## 4. Debuging Configuration for S32 Desing Studio 
To have a complete track of the code at simulation. It is necesary to connect Renode and S32 Desing Studio's Debuger.

1. Include the .resc file on **Renode**
`(monitor) i @renode_sim/Scripts/StearingBoard_test.resc`
> Here is a executed a command `machine StartGdbServer 3333 true` which start a gdb services

2. On **S32 Desing Studio** go to run > Debug Cofigurations > GDB Hardware Debugging. Here you can modify the actual configuration or create a new one.

3. Press on *Debugger* tab and chages this parameters with these values
    - GDB Command: ${S32DS_GDB_ARM32_EXE}
    - Check on the box *use remote target*
    - Debug Server: Generic TCP/IP
    - Connection: localhost:3333

4. Finally press *apply* and start the debug

> Every time for making a debug is necesary to load the .elf file to the machine on renode. Do not start the simulation, the simulation starts at the moment you press on debug in the S32 Desing Studio.  