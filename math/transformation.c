#include "transformation.h"
#include "mav_vect.h"
#include <math.h>

void turn_xy_plane(const float_vect3* vector, const float yaw,
		float_vect3* result)
{
	//turn clockwise
	result->x = cosf(yaw) * vector->x + sinf(yaw) * vector->y;
	result->y = -sinf(yaw) * vector->x + cosf(yaw) * vector->y;
	result->z = vector->z; //leave direction normal to xy-plane untouched

}

void navi2body_xy_plane(const float_vect3* vector, const float yaw,
		float_vect3* result)
{
	turn_xy_plane(vector, yaw, result);
	//	result->x = cosf(yaw) * vector->x + sinf(yaw) * vector->y;
	//	result->y = -sinf(yaw) * vector->x + cosf(yaw) * vector->y;
	//	result->z = vector->z; //leave direction normal to xy-plane untouched
}
void body2navi_xy_plane(const float_vect3* vector, const float yaw,
		float_vect3* result)
{
	turn_xy_plane(vector, -yaw, result);
	//	result->x = cosf(yaw) * vector->x + -sinf(yaw) * vector->y;
	//	result->y = sinf(yaw) * vector->x + cosf(yaw) * vector->y;
	//	result->z = vector->z; //leave direction normal to xy-plane untouched
}

void navi2body(const float_vect3* vector, const float_vect3* angles,
		float_vect3* result)
{
	//TODO implement this
}
//Laurens
void body2navi(const float_vect3* vector, const float_vect3* angles,
		float_vect3* result)
{
	result->x = cosf(angles->y) * cosf(angles->z) * vector->x

	+ (-cosf(angles->x) * sinf(angles->z) + sinf(angles->x) * sinf(angles->y) * cosf(angles->z)) * vector->y

	+ (cosf(angles->x) * sinf(angles->y) * cosf(angles->z) + sinf(angles->x) * sinf(angles->z)) * vector->z;


	result->y = (cosf(angles->y) * sinf(angles->z)) * vector->x

	+ (cosf(angles->x) * cosf(angles->z) + sinf(angles->x) * sinf(angles->y) * sinf(angles->z)) * vector->y

	+ (cosf(angles->x) * sinf(angles->y) * sinf(angles->z) - sinf(angles->x) * cosf(angles->z)) * vector->z;


	result->z = (-sinf(angles->y)) * vector->x

	+ (sinf(angles->x) * cosf(angles->y)) * vector->y

	+ cosf(angles->x) * cosf(angles->y) * vector->z;
}
//
////TOBI WORKING FOR YAW=0 roll is somtimes wrong for big angles
//void body2navi(const float_vect3* vector, const float_vect3* angles,
//		float_vect3* result)
//{
//	result->x = cosf(angles->y) * cosf(angles->z) * vector->x
//
//	+ (-cosf(angles->x) * sinf(angles->z) - sinf(angles->x) * sinf(angles->y)
//			* cosf(angles->z)) * vector->y
//
//	+ (cosf(angles->x) * sinf(angles->y) * cosf(angles->z) + sinf(angles->x) * sinf(
//			angles->z)) * vector->z;
//
//
//	result->y = (cosf(angles->y) * sinf(angles->z)) * vector->x
//
//	+ (cosf(angles->x) * cosf(angles->z) + sinf(angles->x) * sinf(angles->y) * sinf(
//			angles->z)) * vector->y
//
//	+ (cosf(angles->x) * sinf(angles->y) * sinf(angles->z) - sinf(angles->x) * cosf(
//			angles->z)) * vector->z;
//
//
//	result->z = (-sinf(angles->y)) * vector->x
//
//	+ (sinf(angles->x) * cosf(angles->y)) * vector->y
//
//	+ cosf(angles->x) * cosf(angles->y) * vector->z;
//}

//calculates the yaw from the z-gyro


////TOBI2
//void body2world(const float_vect3* vector, const float_vect3* angles, float_vect3*  result)
//{
//	result->x = cosf(angles->y) * cosf(angles->z) * vector->x
//
//			+ cosf(angles->y) * sinf(
//			angles->z) * vector->y
//
//			- sinf(angles->y) * vector->z;
//
//	result->y = (sinf(angles->x) * sinf(angles->y) * cosf(angles->z) - cosf(angles->x)
//			* sinf(angles->z)) * vector->x
//
//			+ (cosf(angles->x) * cosf(angles->z)
//			+ sinf(angles->x) * sinf(angles->y) * sinf(angles->z)) * vector->y
//
//			+ sinf(angles->x) * cosf(angles->y) * vector->z;
//
//	result->z = (cosf(angles->x) * sinf(angles->y) * cosf(angles->z) + sinf(angles->x)
//			* sinf(angles->z)) * vector->x
//
//			+ (cosf(angles->x) * sinf(angles->y) * sinf(
//			angles->z) - sinf(angles->x) * cosf(angles->z)) * vector->y
//
//			+ cosf(angles->x) * cosf(angles->y) * vector->z;
//}


// AMIR new
// Transforms from Body Frame to Navigation Frame(NED->North, East,Down, Origin:Take Off Point)
//void body2Nav(const float_vect3* vector, const float_vect3* angles, float_vect3*  result)
//{
//	result->x = cosf(angles->y) * cosf(angles->z) * vector->x
//				+(cosf(angles->z)*sinf(angles->x) * sinf(angles->y)-cosf(angles->x)*sinf(angles->z)) * vector->y
//				+(sinf(angles->x)*sinf(angles->z)+cosf(angles->x)*cosf(angles->z)*sinf(angles->y)) * vector->z;
//
//	result->y = cosf(angles->y))* sinf(angles->z) * vector->x
//			+ (cosf(angles->x) * cosf(angles->z)+ sinf(angles->x) * sinf(angles->y) * sinf(angles->z)) * vector->y
//			+ (cosf(angles->x) * sinf(angles->y) * sinf(angles->z)-cosf(angles->z)*sinf(angles->x))* vector->z;
//
//	result->z = - sinf(angles->y) * vector->x
//				+ cosf(angles->y) * sinf(angles->x) * vector->y
//				+ cosf(angles->x) * cosf(angles->y) * vector->z;
//}


//void body2world(const float_vect3* vector, const float_vect3* angles, float_vect3*  result)
//{
//	result->x = cosf(angles->y) * cosf(angles->z) * vector->x
//
//			+ cosf(angles->y) * sinf(
//			angles->z) * vector->y
//
//			- sinf(angles->y) * vector->z;
//
//	result->y = (-sinf(angles->x) * sinf(angles->y) * cosf(angles->z) - cosf(angles->x)
//			* sinf(angles->z)) * vector->x
//
//			+ (-sinf(angles->x) * sinf(angles->y)
//			* sinf(angles->z) + cosf(angles->x) * cosf(angles->z)) * vector->y
//
//			- sinf(angles->x) * cosf(angles->y) * vector->z;
//
//	result->z = (cosf(angles->x) * sinf(angles->y) * cosf(angles->z) - sinf(angles->x)
//			* sinf(angles->z)) * vector->x
//
//			+ (cosf(angles->x) * sinf(angles->y) * sinf(
//			angles->z) + sinf(angles->x) * cosf(angles->z)) * vector->y
//
//			+ cosf(angles->x) * cosf(angles->y) * vector->z;
//}

// AMIR
//void body2world(const float_vect3* vector, const float_vect3* angles, float_vect3*  result)
//{
//	result->x = cosf(angles->y) * cosf(angles->z) * vector->x + cosf(angles->y) * sinf(
//			angles->z) * vector->y - sinf(angles->y) * vector->z;
//
//	result->y = (-sinf(angles->x) * sinf(angles->y) * cosf(angles->z) - cosf(angles->x)
//			* sinf(angles->z)) * vector->x
//			+ (-sinf(angles->x) * sinf(angles->y)
//			* sinf(angles->z) + cosf(angles->x) * cosf(angles->z)) * vector->y
//			- sinf(angles->x) * cosf(angles->y) * vector->z;
//
//	result->z = (cosf(angles->x) * sinf(angles->y) * cosf(angles->z) - sinf(angles->x)
//			* sinf(angles->z)) * vector->x + (cosf(angles->x) * sinf(angles->y) * sinf(
//			angles->z) + sinf(angles->x) * cosf(angles->z)) * vector->y + cosf(
//			angles->x) * cosf(angles->y) * vector->z;
//}
