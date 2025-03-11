# TigerEngineRoboticsDriver
This repository contains all the code for communicating with my personal chess engine, TigerEngine, as well as the electronics and mechanical robotic arm that physically plays chess on the board.

## Overview
This project combines robotics, embedded systems, and AI to create a fully autonomous chess-playing robotic arm. The system integrates my custom-designed 3DOF robotic arm with an electromagnet end effector, controlled by an Arduino Nano, while a Raspberry Pi Zero 2W runs the TigerEngine chess engine and handles interprocess communication.

### Features
- **3DOF Robotic Arm** - Custom Designed PCB for main control functions (stepper motor operation, buttons, limit switches, board state, power regulation), 3D printed mechanical design optimized for quick and precise movement using linear and rotational motion
- **Electronic Chess Board Attachment** - Custom Designed PCB and 3D printed board solution designed to transfer the current board state to the computer, (8 PCB 'strips' daisy-chained together underneath each row of the board)
- **Chess Engine Integration** - The TigerEngine UCI Executable acts as the brain of the robotic arm, pondering the current position and replying with the 'best' move
- **Chess Piece Set** - 3D-printable piece set with a mounting point for a magnet on the bottom of each piece

### Future Plans
- Overhaul main control circuit board to leverage a microprocessor, creating an all-in-one solution elimnating the need for the Arduino and Rpi
- Redesign portions of the robotic arm to decrease sag and optimize reduction ratios for x & y axis
- Different piece recognition solution for the chess board itself
