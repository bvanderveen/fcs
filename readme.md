# fcs - Flight Control System

This will be a flight control system for flying robots.

## What it does:

- Controls an X-Plane flight simulation over UDP
 - Receives UDP packets from X-Plane containing position, accelleration, and attitude data
 - Sends UDP packets to X-Plane containing aileron, elevator, rudder, throttle data
- Will eventually control a real model:
  - Recieve signals from a GPS and plus 9DoF sensor. Likely the CHR UM6.
  - Send commands to PWM output hardware, hence to servos.
- Output system state over UDP to an external glass cockpit application
- Update system state over UDP base on commands from pilot's high-level flight planner console
- Controller capabilities
  - Fly lists of waypoints (JSON over UDP)
  - Automatic turn coordinator
  - Total energy controller (altitude, speed, angle-of-attack)

## What it will do:

- Autonomous landing

## Design intent:

I wrote this thing in C because I wanted to use the LLVM toolchain to target embedded systems. Higher level applications (glass cockpit, flight planner) will be writ in Clojure.

System maintains a state singleton in memory that can be read or written to over UDP. At the start of the loop the system reads values from the network, updates the state as necessary. Then the control phase updates steps its algorithm forward in time according to the values read from the system state. Finally, any selected output values will be broadcast.

## Setting system state

Configure it to listen on a port (dig in source for now).

Huck UDP packets of JSON at that port. For example, you could just hook a joystick to it:

    [
    {"name": "state.effector.aileron", "value": .12`},
    {"name": "state.effector.rudder", "value": .11},
    {"name": "state.effector.elevator", "value": .21}
    ]
    
Input packets can be single name-value pairs or arrays of name-value pairs
 
## Selecting output values

There are some special values that control the output behavior. You can select a UDP destination and port, and specify the values you want to see output.

`{"name": "state.output.destination.ip", "value": "192.168.0.1" }`
`{"name": "state.output.destination.port", "value": 34231 }`
`{"name": "state.output.values", "value": [ "state.sensor.latitude", "state.sensor.longitude" ]}`

For a list of names see `state.h`. JSON typed values are not broadcast. Each broadcast packet is a 32-bit float representing a value named in `state.output.values`, in corresponding order.



