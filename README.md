# Steering Wheel Board Project

## Project Overview
The goal of this project is to design and build the electronic board and firmware for a Formula 1 style steering wheel. 

This projects aims to solved the next requirements:

* Read 4 digital buttons (UP, DOWN, DRS, Pit Limiter).
* Read 2 analog inputs (Clutch position, 10-position Rotary Switch).
* Communicate with the main ECU via CAN bus, sending input states and receiving vehicle data.
* Display information (Temperatures, Statuses) on a 128x64 monochrome display.
* Control 2 status LEDs.

## About this repository

This repository was made to contain all the firware desing of the Steering Wheel Board

The repository is divided in:

1. **FIRMWARE/:** In this folder you will find all the codes used for this project

2. **Renode_sim/:** This folder contains a basic simulator for the MCU S32K118 on *Renode* which can be usefull to check if the firmware is capable to flash and debug

3. **HL_desing.cpc:** Which have a High level desing of the whole architecture on pseudo code

