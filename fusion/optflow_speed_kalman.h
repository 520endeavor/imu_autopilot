/*
 * optflow_speed_kalman.h
 *
 *  Created on: 23.08.2010
 *      Author: Petri Tanskanen
 */

#ifndef OPTFLOW_SPEED_KALMAN_H_
#define OPTFLOW_SPEED_KALMAN_H_

#include "mav_vect.h"

void optflow_speed_kalman_init(void);
void optflow_speed_kalman(float_vect3 *optflow, float_vect3 *optflow_filtered_world);

#endif /* OPTFLOW_SPEED_KALMAN_H_ */
