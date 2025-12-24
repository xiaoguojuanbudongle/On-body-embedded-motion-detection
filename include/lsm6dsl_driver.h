#ifndef LSM6DSL_DRIVER_H
#define LSM6DSL_DRIVER_H

#include "mbed.h"

// Initialize LSM6DSL: set ODR=52 Hz, accel range ±2g, gyro 52 Hz, etc.
bool lsm6dsl_init();

// Read one accelerometer sample (units: g)
// Return: true = success, false = communication failure
bool lsm6dsl_read_accel(float &ax_g, float &ay_g, float &az_g);

#endif // LSM6DSL_DRIVER_H
