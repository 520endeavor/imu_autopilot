/*
 * vision_buffer.c
 *
 *  Created on: 13.05.2010
 *      Author: Laurens Mackay
 */

#ifndef VISION_BUFFER_C_
#define VISION_BUFFER_C_
#include "vision_buffer.h"
#include "float_checks.h"
#include "sys_time.h"
#include "global_data.h"
#include "lookup_sin_cos.h"

#include "debug.h"
#include "transformation.h"

#define VISION_BUFFER_COUNT 8

static uint8_t vision_buffer_index_write = 0;
static uint8_t vision_buffer_index_read = 0;
static vision_t vision_buffer[VISION_BUFFER_COUNT];
//static uint16_t vision_buffer_count = 0;
uint32_t vision_buffer_full_count = 0;
uint32_t vision_buffer_reject_count = 0;

void vision_buffer_buffer_init()
{
	for (int i = 0; i < VISION_BUFFER_COUNT; i++)
	{
		vision_buffer[i].time_captured = 0;
	}
}

void vision_buffer_buffer_camera_triggered(uint64_t usec,
		uint64_t loop_start_time, uint32_t seq)
{
	//debug_message_buffer("cam triggered");


	// Emit timestamp of this image
	//	mavlink_msg_image_triggered_send(MAVLINK_COMM_0,
	//			usec);


	if (vision_buffer_index_read - vision_buffer_index_write == 1
			|| (vision_buffer_index_read == 0 && vision_buffer_index_write
					== VISION_BUFFER_COUNT - 1))
	{
		//overwrite oldest
		vision_buffer_index_read = (vision_buffer_index_read + 1)
				% VISION_BUFFER_COUNT;

		//		if (vision_buffer_full_count++ % 16 == 0)
		if (vision_buffer_full_count++ % 256 == 0)
		{
			debug_message_buffer_sprintf("vision_buffer buffer full %u",
					vision_buffer_full_count);
		}

	}
	vision_buffer_index_write = (vision_buffer_index_write + 1)
			% VISION_BUFFER_COUNT;
	//vision_buffer_count++;

	// save position in buffer
	vision_buffer[vision_buffer_index_write].pos = global_data.position;
	vision_buffer[vision_buffer_index_write].ang = global_data.attitude;
	vision_buffer[vision_buffer_index_write].time_captured = usec;
//	vision_buffer[vision_buffer_index_write].time_captured = loop_start_time;

	//debug_message_buffer("vision_buffer stored data");


	// Emit sensor data matching to this image
	//TODO: get this from buffer
	//	mavlink_msg_attitude_send(MAVLINK_COMM_0,
	//			usec,
	//			global_data.attitude.x, global_data.attitude.y,
	//			global_data.attitude.z, global_data.gyros_si.x,
	//			global_data.gyros_si.y, global_data.gyros_si.z);


}

/**
 * @brief Send one of the buffered messages
 * @param pos data from vision
 */
void vision_buffer_handle_data(mavlink_vision_position_estimate_t* pos)
{
	if (vision_buffer_index_write == vision_buffer_index_read)
	{
		//buffer empty
		return;

	}
	vision_buffer_index_read = (vision_buffer_index_read + 1)
			% VISION_BUFFER_COUNT;

	//TODO: find data and process it
	uint8_t for_count = 0;
	uint8_t i = vision_buffer_index_read;
	for (; (vision_buffer[i].time_captured < pos->usec)
			&& (vision_buffer_index_write - i != 1); i = (i + 1)
			% VISION_BUFFER_COUNT)
	{

		if (++for_count > VISION_BUFFER_COUNT)
		{
			debug_message_buffer("vision_buffer PREVENTED HANG");
			break;
		}
	}
	if (vision_buffer[i].time_captured == pos->usec)
	{

		//we found the right data
		if (!isnumber(pos->x) || !isnumber(pos->y) || !isnumber(pos->z)
				|| !isnumber(pos->roll) || !isnumber(pos->pitch) || !isnumber(pos->yaw)
				|| pos->x == 0.0 || pos->y == 0.0 || pos->z == 0.0)
		{
			//reject invalid data
			debug_message_buffer("vision_buffer invalid data (inf,nan,0) rejected");
		}
		else if (fabs(vision_buffer[i].ang.x - pos->roll)
				< global_data.param[PARAM_VISION_ANG_OUTLAYER_TRESHOLD]
				&& fabs(vision_buffer[i].ang.y - pos->pitch)
						< global_data.param[PARAM_VISION_ANG_OUTLAYER_TRESHOLD])
		{
			// Update validity time
			global_data.pos_last_valid = sys_time_clock_get_time_usec();

			//Pack new vision_data package
			global_data.vision_data.time_captured
					= vision_buffer[i].time_captured;
			global_data.vision_data.comp_end = sys_time_clock_get_unix_time();

			//Set data from Vision directly
			global_data.vision_data.ang.x = pos->roll;
			global_data.vision_data.ang.y = pos->pitch;
			global_data.vision_data.ang.z = pos->yaw;

			global_data.vision_data.pos.x = pos->x;
			global_data.vision_data.pos.y = pos->y;
			global_data.vision_data.pos.z = pos->z;

			// If yaw input from vision is enabled, feed vision
			// directly into state estimator
			global_data.vision_magnetometer_replacement.x = 200.0f*lookup_cos(pos->yaw);
			global_data.vision_magnetometer_replacement.y = -200.0f*lookup_sin(pos->yaw);
			global_data.vision_magnetometer_replacement.z = 0.f;

			//If yaw goes to infinity (no idea why) set it to setpoint, next time will be better
			if (global_data.attitude.z > 18.8495559 || global_data.attitude.z < -18.8495559)
			{
				global_data.attitude.z = global_data.yaw_pos_setpoint;
				debug_message_buffer("vision_buffer CRITICAL FAULT yaw was bigger than 6 PI! prevented crash");
			}

			global_data.vision_data.new_data = 1;

			//TODO correct also all buffer data needed if we are going to have overlapping vision data
		}
		else
		{
			//rejected outlayer
			if (vision_buffer_reject_count++ % 16 == 0)
			{
				debug_message_buffer_sprintf("vision_buffer rejected outlier #%u",
						vision_buffer_reject_count);
			}
		}
		if (global_data.param[PARAM_SEND_SLOT_DEBUG_1] == 1)
		{

			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 202, global_data.attitude.z);

			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 210, pos.x);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 211, pos.y);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 212, pos.z);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 215, pos.yaw);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 212, pos.z);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 203, pos.r1);
			//mavlink_msg_debug_send(global_data.param[PARAM_SEND_DEBUGCHAN], 204, pos.confidence);
		}
		if (for_count)
		{
			debug_message_buffer_sprintf(
					"vision_buffer data found skipped %i data sets", for_count);
		}
	}
	else
	{
		//we didn't find it
		//		debug_message_buffer("vision_buffer data NOT found");
		if (for_count)
		{
			debug_message_buffer_sprintf(
					"vision_buffer data NOT found skipped %i data sets",
					for_count);
		}
	}
	vision_buffer_index_read = i;//skip all images that are older;
	//	if (for_count)
	//	{
	//		debug_message_buffer_sprintf("vision_buffer skipped %i data sets",
	//				for_count);
	//	}
}

/**
 * @brief Send one of the buffered messages
 * @param pos data from vision
 */
void vision_buffer_handle_global_data(mavlink_global_vision_position_estimate_t* pos)
{
	if (vision_buffer_index_write == vision_buffer_index_read)
	{
		//buffer empty
		debug_message_buffer("ERROR VIS BUF: buffer empty");
		return;
	}
	vision_buffer_index_read = (vision_buffer_index_read + 1)
			% VISION_BUFFER_COUNT;

	//TODO: find data and process it
	uint8_t for_count = 0;
	uint8_t i = vision_buffer_index_read;
	for (; (vision_buffer[i].time_captured < pos->usec)
			&& (vision_buffer_index_write - i != 1); i = (i + 1)
			% VISION_BUFFER_COUNT)
	{
		if (++for_count > VISION_BUFFER_COUNT) break;
	}

	if (vision_buffer[i].time_captured == pos->usec)
	{

		//we found the right data
		if (!isnumber(pos->x) || !isnumber(pos->y) || !isnumber(pos->z)
				|| !isnumber(pos->roll) || !isnumber(pos->pitch) || !isnumber(pos->yaw)
				|| pos->x == 0.0 || pos->y == 0.0 || pos->z == 0.0)
		{
			//reject invalid data
			debug_message_buffer("ERROR VIS BUF: invalid data (inf,nan,0) rejected");
		}
		else if (fabs(vision_buffer[i].ang.x - pos->roll) < global_data.param[PARAM_VISION_ANG_OUTLAYER_TRESHOLD]
				&& fabs(vision_buffer[i].ang.y - pos->pitch) < global_data.param[PARAM_VISION_ANG_OUTLAYER_TRESHOLD])
		{
			// Update validity time
			global_data.pos_last_valid = sys_time_clock_get_time_usec();

			//Pack new vision_data package
			global_data.vision_data_global.time_captured
					= vision_buffer[i].time_captured;
			global_data.vision_data_global.comp_end = sys_time_clock_get_unix_time();

#define PROJECT_GLOBAL_DATA_FORWARD

#ifdef PROJECT_GLOBAL_DATA_FORWARD
			// FIXME currently visodo is not allowed to run in parallel, else race condititions!

			// Project position measurement
			global_data.vision_data_global.pos.x = pos->x + (global_data.position.x - vision_buffer[i].pos.x);
			global_data.vision_data_global.pos.y = pos->y + (global_data.position.y - vision_buffer[i].pos.y);
			global_data.vision_data_global.pos.z = pos->z + (global_data.position.z - vision_buffer[i].pos.z);

			// Set roll and pitch absolutely
			global_data.vision_data_global.ang.x = pos->roll;
			global_data.vision_data_global.ang.y = pos->pitch;

			// Project yaw
			//global_data.vision_data_global.ang.z = pos->yaw-vision_buffer[i].ang.z;

			for (uint8_t j = (i+1) % VISION_BUFFER_COUNT; j != i; j = (j + 1) % VISION_BUFFER_COUNT)
			{
				vision_buffer[j].pos.x = vision_buffer[j].pos.x + (pos->x - vision_buffer[i].pos.x);
				vision_buffer[j].pos.y = vision_buffer[j].pos.y + (pos->y - vision_buffer[i].pos.y);
				vision_buffer[j].pos.z = vision_buffer[j].pos.z + (pos->z - vision_buffer[i].pos.z);
			}

#else
			global_data.vision_data_global.pos.x = pos->x;
			global_data.vision_data_global.pos.y = pos->y;
			global_data.vision_data_global.pos.z = pos->z;

			//Set data from Vision directly
			global_data.vision_data_global.ang.x = pos->roll;
			global_data.vision_data_global.ang.y = pos->pitch;
			global_data.vision_data_global.ang.z = pos->yaw;
#endif

			// If yaw input from vision is enabled, feed vision
			// directly into state estimator
			global_data.vision_global_magnetometer_replacement.x = 200.0f*lookup_cos(pos->yaw);
			global_data.vision_global_magnetometer_replacement.y = -200.0f*lookup_sin(pos->yaw);
			global_data.vision_global_magnetometer_replacement.z = 0.f;

			//If yaw goes to infinity (no idea why) set it to setpoint, next time will be better
			if (global_data.attitude.z > 18.8495559 || global_data.attitude.z < -18.8495559)
			{
				global_data.attitude.z = global_data.yaw_pos_setpoint;
				debug_message_buffer("vision_buffer CRITICAL FAULT yaw was bigger than 6 PI! prevented crash");
			}

			global_data.vision_data_global.new_data = 1;
			debug_message_buffer_sprintf("vision_buffer data found skipped %i data sets", for_count);
			//TODO correct also all buffer data needed if we are going to have overlapping vision data
		}
		else
		{
			//rejected outlier
			//if (vision_buffer_reject_count++ % 16 == 0)
			{
				debug_message_buffer_sprintf("vision_buffer rejected outlier #%u",
						vision_buffer_reject_count);
			}
		}
	}
	else
	{
		//we didn't find it
		debug_message_buffer_sprintf("vision_buffer data NOT found skipped %i data sets", for_count);
	}
	vision_buffer_index_read = i;//skip all images that are older;
}

#endif /* VISION_BUFFER_C_ */
