#ifndef MATH_H
#define MATH_H

#include <stdint.h>

float sqrtf(float x);

uint32_t int_sqrt(uint32_t x);

/**
 * @brief Round a float to the nearest integer, halfway cases away from zero.
 * @param x The float to round.
 * @return The rounded value as a float.
 */
float roundf(float x);

#endif