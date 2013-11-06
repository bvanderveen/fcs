# fcs - Flight Control System

This will be a flight control system for flying robots.

What it does:

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

What it will do:

- Autonomous landing

Design intent:

I wrote this thing in C because I wanted to use the LLVM toolchain to target embedded systems. Higher level applications (glass cockpit, flight planner) will be writ in Clojure.

System maintains a state singleton in memory that can be read or written to over UDP. At the start of the loop the system reads values from the network, updates the state as necessary. Then the control phase updates steps its algorithm forward in time according to the values read from the system state. Finally, any selected output values will be broadcast.

## Selecting values

Send the JSON message to the listener:

`{"name": "state.output": [ ... names of states to be output ... ]}`

For a list of names see `state.h`.

The values are output in the perfectly adequate X-Plane format.

`"DATA\0"` followed N of:

    struct xplane_message_data
    {
        uint32_t index; // 
        float data[8]; // these are 32 bits each on my system
    };

That's the string `"DATA\0"` followed by N * 9 32-bit numbers. The `index` integer value will indicate what the following floats values mean. Exactly what means what is TBD.

