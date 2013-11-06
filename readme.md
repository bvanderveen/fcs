# fcs - Flight Control System

This will be a flight control system for flying robots.

What it does:

- Controls an X-Plane flight simulation over UDP
 - Receives UDP packets from X-Plane containing position, accelleration, and attitude data
 - Sends UDP packets to X-Plane containing aileron, elevator, rudder, throttle data
- Will eventually control a real model:
  - Recieve signals from a GPS and plus 9DoF sensor. Likely the CHR UM6.
  - Send commands to PWM output hardware, hence to servos.
- Output system state over UDP to a monitor (floating point arrays only)
- Update system state over UDP based on output from an external, high-level flight planner
- Controller capabilities
  - Fly lists of waypoints (JSON over UDP)
  - Automatic turn coordinator
  - Total energy controller (altitude, speed, angle-of-attack)

What it will do:

- Autonomous landing
