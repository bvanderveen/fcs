#include "heading.h"

// float heading_error(float heading, float desiredHeading) {
//     float a = desiredHeading - heading;
//     return a + ((a > 180.0) ? -360.0 : ((a < -180.0) ? 360.0 : 0.0));
// }

// void core_heading_setting_update(core_context *context, void *heading_setting, float dt) {
// 	core_heading_setting *setting = (core_heading_setting *)heading_setting;

//     // heading control is a PID cascade controller. 
//     // outer loop takes heading error and commands bank angle 
//     float bearingError = heading_error(context->sensor_state.heading, setting->desiredHeading);
//     pid_update(&setting->aileronController, bearingError, dt);

//     // inner loop takes bank angle and commands aileron deflection
//     float roll_error = -setting->aileronController.output - context->sensor_state.roll;
//     pid_update(&setting->aileronController2, roll_error, dt);
//     context->effector_state.ail = -setting->aileronController2.output;
// }
