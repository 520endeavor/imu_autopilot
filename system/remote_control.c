/*=====================================================================
 
 PIXHAWK Micro Air Vehicle Flying Robotics Toolkit
 Please see our website at <http://pixhawk.ethz.ch>
 
 (c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>
 
 This file is part of the PIXHAWK project
 
 PIXHAWK is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 PIXHAWK is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.
 
 ======================================================================*/

/**
 * @file
 *   @brief Definition of the class remote_control.
 *
 *   @author
 *
 */

#include "remote_control.h"

#include "control_quadrotor_attitude.h"
#include "conf.h"
#include "ppm.h"
#include "inttypes.h"
#include "global_data.h"
// Include comm
#include "comm.h"
#include "pixhawk/mavlink.h"
#include "i2c_motor_mikrokopter.h"
#include "pid.h"
#include "radio_control.h"
#include "control_quadrotor_attitude.h"
#include "control_quadrotor_position.h"
#include "debug.h"
#include "math.h"
#include "sys_state.h"
#include "calibration.h"

inline void remote_control(void)
{
	static uint32_t lossCounter = 0;
	if (global_data.state.mav_mode & (uint8_t) MAV_MODE_FLAG_MANUAL_INPUT_ENABLED)
	{
		if (radio_control_status() == RADIO_CONTROL_ON)
		{
			global_data.state.remote_ok=1;
			if (lossCounter > 3)
			{
				debug_message_buffer("REGAINED REMOTE SIGNAL AFTER LOSS!");
			}
			lossCounter = 0;
			//get remote controll values
			float gas_remote = PPM_SCALE_FACTOR * (ppm_get_channel(
					global_data.param[PARAM_PPM_THROTTLE_CHANNEL]) - PPM_OFFSET);
			//	message_debug_send(MAVLINK_COMM_1, 12, gas_remote);
			float yaw_remote = PPM_SCALE_FACTOR * (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) - PPM_CENTRE);
			//	message_debug_send(MAVLINK_COMM_1, 13, yaw_remote);
			float nick_remote = PPM_SCALE_FACTOR * (ppm_get_channel(
					global_data.param[PARAM_PPM_NICK_CHANNEL]) - PPM_CENTRE);
			//	message_debug_send(MAVLINK_COMM_1, 12, gas_remote);
			float roll_remote = PPM_SCALE_FACTOR * (ppm_get_channel(
					global_data.param[PARAM_PPM_ROLL_CHANNEL]) - PPM_CENTRE);
			//	message_debug_send(MAVLINK_COMM_1, 13, yaw_remote);
			//	if(radio_control_status()==RADIO_CONTROL_OFF){
			//		gas_remote=0.3;
			//		yaw_remote=0;
			//		nick_remote=0;
			//		roll_remote=0;
			//	}
			//MANUAL ATTITUDE CONTROL
			global_data.attitude_setpoint_remote.x = -roll_remote;
			global_data.attitude_setpoint_remote.y = -nick_remote;
			global_data.attitude_setpoint_remote.z = -5 * yaw_remote;// used as yaw speed!!! yaw offset1
			global_data.gas_remote = gas_remote;

			//MANUAL POSITION CONTROL
			//TODO


			//		Switch on MAV_STATE_ACTIVE
			if ((ppm_get_channel(global_data.param[PARAM_PPM_THROTTLE_CHANNEL])
					< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) < PPM_LOW_TRIG))
			{


				//				//Reset YAW integration(only needed if no vision)
				//				global_data.attitude.z = 0;
				//				global_data.yaw_pos_setpoint = 0;

				if (global_data.state.status != MAV_STATE_ACTIVE)
				{
					debug_message_buffer("MAV_STATE_ACTIVE Motors started");

					//debug_message_buffer("CALIBRATING GYROS");
					//start_gyro_calibration();
				}
				//switch on motors
				global_data.state.status = MAV_STATE_ACTIVE;
				global_data.state.fly = FLY_WAIT_MOTORS;
				global_data.state.mav_mode |= MAV_MODE_FLAG_SAFETY_ARMED;
				//this will be done by setpoint
				if (global_data.state.mav_mode & MAV_MODE_FLAG_TEST_ENABLED)
				{
					global_data.state.fly = FLY_FLYING;
				}
			}

			//		Switch off MAV_STATE_STANDBY
			if ((ppm_get_channel(global_data.param[PARAM_PPM_THROTTLE_CHANNEL])
					< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) > PPM_HIGH_TRIG))
			{
				if (global_data.state.status != MAV_STATE_STANDBY)
				{
					debug_message_buffer("MAV_STATE_STANDBY Motors off");
				}
				//switch off motors
				global_data.state.status = MAV_STATE_STANDBY;
				global_data.state.fly = FLY_GROUNDED;
				global_data.state.mav_mode &= ~MAV_MODE_FLAG_SAFETY_ARMED;

			}

			// Start Gyro calibration left stick: left down. right stick right down.
			if ((ppm_get_channel(global_data.param[PARAM_PPM_THROTTLE_CHANNEL])
					< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) > PPM_HIGH_TRIG)
					&& (ppm_get_channel(
							global_data.param[PARAM_PPM_NICK_CHANNEL])
							< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_ROLL_CHANNEL]) < PPM_LOW_TRIG))
			{
				start_gyro_calibration();
			}

			// Start capture: left down. right stick right up.
			if ((ppm_get_channel(global_data.param[PARAM_PPM_THROTTLE_CHANNEL])
					< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) > PPM_HIGH_TRIG)
					&& (ppm_get_channel(
							global_data.param[PARAM_PPM_NICK_CHANNEL])
							> PPM_HIGH_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_ROLL_CHANNEL]) < PPM_LOW_TRIG))
			{
				// FIXME LORENZ CAPTURE START
				//should actually go to capturer not IMU
			}
			// Stop capture: left down. right stick right up.
			if ((ppm_get_channel(global_data.param[PARAM_PPM_THROTTLE_CHANNEL])
					< PPM_LOW_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_YAW_CHANNEL]) > PPM_HIGH_TRIG)
					&& (ppm_get_channel(
							global_data.param[PARAM_PPM_NICK_CHANNEL])
							> PPM_HIGH_TRIG) && (ppm_get_channel(
					global_data.param[PARAM_PPM_ROLL_CHANNEL]) > PPM_HIGH_TRIG))
			{
				// FIXME LORENZ CAPTURE END
				//should actually go to capturer not IMU
			}

			if (global_data.state.mav_mode & MAV_MODE_FLAG_GUIDED_ENABLED)
			{
				// Reset position to 0,0, mostly useful for optical flow with setpoint at 0,0
				if (ppm_get_channel(global_data.param[PARAM_PPM_TUNE4_CHANNEL])
						< PPM_LOW_TRIG)
				{
					if (global_data.param[PARAM_ATT_KAL_YAW_ESTIMATION_MODE] == 3)
					{	//one special case - if yaw is from vicon then reset position to current vicon position
						global_data.position.x = global_data.vicon_data.x;
						global_data.position.y = global_data.vicon_data.y;
					}
					else
					{
						global_data.position.x = 0;
						global_data.position.y = 0;
					}

				}
			}

			if (global_data.state.mav_mode & MAV_MODE_FLAG_TEST_ENABLED)
			{

				if (ppm_get_channel(global_data.param[PARAM_PPM_TUNE1_CHANNEL])
						< PPM_LOW_TRIG)
				{
					//z-controller disabled
					global_data.param[PARAM_MIX_POSITION_Z_WEIGHT] = 0;
				}
				else
				{
					//z-controller enabled
					global_data.param[PARAM_MIX_POSITION_Z_WEIGHT] = 1;
				}

				if (ppm_get_channel(global_data.param[PARAM_PPM_TUNE4_CHANNEL])
						< PPM_LOW_TRIG)
				{
					//low - Position Hold off
					global_data.param[PARAM_MIX_POSITION_WEIGHT] = 0;
					global_data.param[PARAM_MIX_POSITION_YAW_WEIGHT] = 0;

					global_data.position.x = 0;
					global_data.position.y = 0;
				}
				else if (ppm_get_channel(
						global_data.param[PARAM_PPM_TUNE4_CHANNEL])
						> PPM_HIGH_TRIG)
				{
					//high - Position Hold with setpoint movement TODO
					global_data.param[PARAM_MIX_POSITION_WEIGHT] = 1;
					global_data.param[PARAM_MIX_POSITION_YAW_WEIGHT] = 1;
				}
				else
				{
					//center - Position Hold
					global_data.param[PARAM_MIX_POSITION_WEIGHT] = 1;
					global_data.param[PARAM_MIX_POSITION_YAW_WEIGHT] = 1;
				}

			}

			//			//Integrate YAW setpoint
			//			if (fabs(
			//					(ppm_get_channel(global_data.param[PARAM_PPM_GIER_CHANNEL])
			//							- PPM_CENTRE)) > 10)
			//			{
			//				global_data.yaw_pos_setpoint -= 0.02 * 0.03
			//						* (ppm_get_channel(
			//								global_data.param[PARAM_PPM_GIER_CHANNEL])
			//								- PPM_CENTRE);
			//			}
			//		Set PID VALUES

			float tune2 = 1 * (ppm_get_channel(
					global_data.param[PARAM_PPM_TUNE2_CHANNEL]) - 1109);
			float tune3 = 1 * (ppm_get_channel(
					global_data.param[PARAM_PPM_TUNE3_CHANNEL]) - 1109);
			if (tune2 < 0)
			{
				tune2 = 0;
			}
			if (tune2 > 1000)
			{
				tune2 = 1000;
			}

			if (tune3 < 0)
			{
				tune3 = 0;
			}
			if (tune3 > 1000)
			{
				tune3 = 1000;
			}

			//TUNING FOR TOBIS REMOTE CONTROL

			if (global_data.param[PARAM_TRIMCHAN] == 1)
			{
								global_data.param[PARAM_PID_ATT_P] = 0.1 * tune2;
//								global_data.param[PARAM_PID_ATT_I] = 0;
								global_data.param[PARAM_PID_ATT_D] = 0.1 * tune3;
			}
			else if (global_data.param[PARAM_TRIMCHAN] == 2)
			{
								global_data.param[PARAM_PID_POS_P] = 0.01 * tune2;
//								global_data.param[PARAM_PID_POS_I] = 0;
								global_data.param[PARAM_PID_POS_D] = 0.01 * tune3;
			}
			else if (global_data.param[PARAM_TRIMCHAN] == 3)
			{
								global_data.param[PARAM_PID_POS_Z_P] = 0.01 * tune2;
//								global_data.param[PARAM_PID_POS_Z_I] = 0;
								global_data.param[PARAM_PID_POS_Z_D] = 0.01 * tune3;
			}
			else if (global_data.param[PARAM_TRIMCHAN] == 4)
			{
								global_data.param[PARAM_PID_YAWSPEED_P] = 0.1 * tune2;
//								global_data.param[PARAM_PID_YAWSPEED_I] = 0;
								global_data.param[PARAM_PID_YAWSPEED_D] = 0.1 * tune3;
			}
			else if (global_data.param[PARAM_TRIMCHAN] == 5)
			{
								global_data.param[PARAM_PID_YAWPOS_P] = 0.1 * tune2;
//								global_data.param[PARAM_PID_YAWPOS_I] = 0;
								global_data.param[PARAM_PID_YAWPOS_D] = 0.1 * tune3;
			}
			//this is done at 10 Hz
			//			pid_set_parameters(&nick_controller,
			//					global_data.param[PARAM_PID_ATT_P],
			//					global_data.param[PARAM_PID_ATT_I],
			//					global_data.param[PARAM_PID_ATT_D],
			//					global_data.param[PARAM_PID_ATT_AWU]);
			//			pid_set_parameters(&roll_controller,
			//					global_data.param[PARAM_PID_ATT_P],
			//					global_data.param[PARAM_PID_ATT_I],
			//					global_data.param[PARAM_PID_ATT_D],
			//					global_data.param[PARAM_PID_ATT_AWU]);
			//
			//			pid_set_parameters(&x_axis_controller,
			//					global_data.param[PARAM_PID_POS_P],
			//					global_data.param[PARAM_PID_POS_I],
			//					global_data.param[PARAM_PID_POS_D],
			//					global_data.param[PARAM_PID_POS_AWU]);
			//			pid_set_parameters(&y_axis_controller,
			//					global_data.param[PARAM_PID_POS_P],
			//					global_data.param[PARAM_PID_POS_I],
			//					global_data.param[PARAM_PID_POS_D],
			//					global_data.param[PARAM_PID_POS_AWU]);

			//global_data.param_name[PARAM_MIX_REMOTE_WEIGHT] = 1;// add position only keep - tune3;
			//global_data.param[PARAM_MIX_POSITION_WEIGHT] = 1;
			if (global_data.param[PARAM_PPM_TUNE1_CHANNEL] != -1)
			{
				global_data.param[PARAM_CAM_ANGLE_Y_OFFSET]
						= ((float) (ppm_get_channel(
								global_data.param[PARAM_PPM_TUNE1_CHANNEL]))
								- 1000.0f) / 1000.0f;
			}
		}
		else
		{
			//No Remote signal
			lossCounter++;

			if (lossCounter > 3 && global_data.state.remote_ok == 1)
			{
				debug_message_buffer("WARNING! NO REMOTE SIGNAL!");
				//Wait one round and start sinking
				global_data.state.remote_ok = 0;
				static uint16_t countdown;
				//already the second time
				// Emergency Landing
				if (global_data.state.fly != FLY_GROUNDED
						&& global_data.state.fly != FLY_RAMP_DOWN && global_data.state.fly != FLY_LANDING)
				{
					debug_message_buffer_sprintf("global_data.state.fly=%i",global_data.state.fly);
					sys_set_state(MAV_STATE_EMERGENCY);
					global_data.state.fly = FLY_LANDING;//start landing
					debug_message_buffer(
							"EMERGENCY LANDING STARTED. No remote signal");

					countdown = 50 * 5; // 5 seconds
				}
				else
				{
					if (global_data.state.fly == FLY_GROUNDED || global_data.state.fly == FLY_WAIT_MOTORS)
					{
						if (global_data.state.mav_mode & MAV_MODE_FLAG_SAFETY_ARMED)
						{

							if (lossCounter < 5)
							{
								debug_message_buffer(
										"EMERGENCY LANDING FINISHED. No remote signal");
								debug_message_buffer(
										"EMERGENCY LANDING NOW LOCKED");
							}
						}

						// Set to disarmed
						sys_set_mode(global_data.state.mav_mode & ~MAV_MODE_FLAG_SAFETY_ARMED);
						sys_set_state(MAV_STATE_STANDBY);
					}
					else
					{
						//won't come here anymore if once in locked mode
						debug_message_buffer("EMERGENCY LANDING. No remote signal");

					}
				}

				if((global_data.state.mav_mode & MAV_MODE_FLAG_TEST_ENABLED) && global_data.state.fly != FLY_GROUNDED){

					//z-controller enabled
					global_data.param[PARAM_MIX_POSITION_Z_WEIGHT] = 1;
					global_data.position_setpoint.z = global_data.position.z
							+ 0.01;
					global_data.param[PARAM_POSITION_SETPOINT_Z]
							= global_data.position.z + 0.01;
					if (countdown-- <= 1)
					{
						global_data.state.fly = FLY_GROUNDED;
						//global_data.param[PARAM_MIX_POSITION_Z_WEIGHT] = 0;
						debug_message_buffer(
								"EMERGENCY LANDING Z control finished");
					}
				}
			}
		}

	}

}
