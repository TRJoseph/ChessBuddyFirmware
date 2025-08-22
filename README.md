# TigerEngineRoboticsDriver
This repository contains all the code for communicating with my personal chess engine, TigerEngine, as well as the electronics and mechanical robotic arm that physically plays chess on the board.

## Overview
This project combines robotics, embedded systems, and AI to create a fully autonomous chess-playing robotic arm. The system integrates my custom-designed 4DOF robotic arm with an electromagnet end effector or claw grip end effector. The project is controlled by a custom designed ESP32 MCU PCB with stepper motor driver ICs, a power management subsystem, an LCD Screen header + more.

### Features
- **4DOF Robotic Arm** - Custom Designed PCB for main control functions (stepper motor operation, buttons, limit switches, board state, power regulation), 3D printed mechanical design optimized for quick and precise movement using linear and rotational motion
- **Electronic Chess Board Attachment** - Custom Designed PCB and 3D printed board solution designed to transfer the current board state to the computer. 8x8 grid of hall effect sensors with LEDs for each square corresponding to a square's occupancy state (empty or occupied).
- **Chess Engine Integration** - The TigerEngine UCI Executable acts as the brain of the robotic arm, pondering the current position and replying with the 'best' move
- **Chess Piece Set** - 3D-printable piece set with a mounting point for a magnet on the bottom of each piece

### Future Plans
- Overhaul current LED solution for more even light distribution on squares
- Change stepper motor ICs to TMC2208s to essentially eliminate noise
