# Cellotrope
software for a 3d zoetrope for animation film workshops - Cellofan' - Lille(fr)

The system consists of a 2 meters diameter cylinder, mounted on a vertical rotation axis, driven by a 
12VDC motor, and a number of DMX512 LED stroboscopes.

A 500steps+zero encoder allows a Fraise Versa2 board to measure the absolute rotational position of the cylinder. When the position crosses a 1/24 turn boundary (the number of steps is configurable), a DMX512 sequence is sent to flash the stroboscope.

The Versa2 is also connected to a VNH5019 H-bridge to control the speed of the DC motor.

A RaspberryPi3 allows to control the overall installation by an OpenStageControl server that can be accessed by any Web client connected to the rPi's wifi access point. The user can then set the rotation speed, the number of steps and the intensity of the stroboscopes light (dimming) through a web interface.

## DMX stroboscope
The system is currently configured for the "Thunder Wash 100W" model from Cameo.

The stroboscopes have to be configured in 3CH-1 mode at DMX 001 address, so the following channels are used:  
- 1: dimmer  
- 2: speed (with special values 0=always-on and 6=always-off)  
- 3: flash duration (actually not used by this project, the flash duration being controlled by the firmware of the Fraise Versa)  
