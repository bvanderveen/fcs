# fcs - Flight Control System

This will be a flight control system for flying robots.

## What it does

- Controls an X-Plane flight simulation over UDP
 - Receives UDP packets from X-Plane containing position, accelleration, and attitude data
 - Sends UDP packets to X-Plane containing aileron, elevator, rudder, throttle data
- Output system state over UDP to an external glass cockpit application
- Update system state over UDP base on commands from external high-level flight planner
- Controller capabilities
  - Fly lists of waypoints (JSON over UDP)
  - Automatic turn coordinator
  - Total energy controller (altitude, speed, angle-of-attack)

## What it will do

- Autonomous landing
- Will eventually control a real model:
  - Recieve signals from a GPS and plus 9DoF sensor. Likely the CHR UM6.
  - Send commands to PWM output hardware, hence to servos.

## Design

Written C to leverage the LLVM toolchain and target embedded systems. Higher level applications (glass cockpit, flight planner) will be writ in Clojure.

System maintains a state singleton in memory that can be read or written to over UDP. At the start of the loop the system reads values from the network, updates the state as necessary. Then the control phase steps its algorithm forward in time according to the values read from the system state. Finally, any selected output values will be broadcast.

## Setting system state

Configure it to listen on a port (dig in source for now).

Huck UDP packets of JSON at that port. For example, you could just hook a joystick to it:

    {
      "state.effector.aileron": .12,
      "state.effector.rudder": .11,
      "state.effector.elevator": .21
    }
 
## Getting system values

There are some special values that control the output behavior. You can select a UDP destination and port, and specify the values you want to see output.

    {
      "state.output.destination.ip": "192.168.0.1",
      "state.output.destination.port": 34231,
      "state.output.values": [ "state.sensor.latitude", "state.sensor.longitude" ]
    }

For a list of names see `state.h`. JSON typed values are not broadcast. Each broadcast packet is a 32-bit float representing a value named in `state.output.values`, in corresponding order.

