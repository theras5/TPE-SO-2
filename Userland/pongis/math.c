#include "math.h"

////double sin_taylor(double x){
////    double term = x; // First term of the series
////    double result = term;// Initialize result with the first term
////    double x_squared = x * x;// Square of x for the series
////    int sign = -1;
////
////    for (int n = 3; n <= 15; n += 2) {
////        term *= x_squared / (n * (n - 1)); // Calculate the next term in the series
////        result += sign * term; // Add or subtract the term from the result
////        sign *= -1; // Alternate the sign for the next term
////    }
////    return result;
////}
//
//double cos_taylor(double x){
//    double term = 1; // First term of the series
//    double result = term; // Initialize result with the first term
//    double x_squared = x * x; // Square of x for the series
//    int sign = -1;
//
//    for (int n = 2; n <= 14; n += 2) {
//        term *= x_squared / (n * (n - 1)); // Calculate the next term in the series
//        result += sign * term; // Add or subtract the term from the result
//        sign *= -1; // Alternate the sign for the next term
//    }
//    return result;
//}
//
//double to_radians(double degrees) {
//    return degrees * (PI / 180.0); // Convert degrees to radians
//}
//
//double to_degrees(double radians) {
//    return radians * (180.0 / PI); // Convert radians to degrees
//}
//
//double normalize_radians(double x){
//    while (x>PI) x -= 2*PI; // Reduce x to the range [-π, π]
//    while (x<-PI) x += 2*PI;
//    return x;
//}
//
//double sin_deg(double degrees) {
//    return sin_taylor(normalize_radians(to_radians(degrees))); // Calculate sine using Taylor series
//}
//
//double cos_deg(double degrees) {
//    return cos_taylor(normalize_radians(to_radians(degrees))); // Calculate cosine using Taylor series
//}
//

// metodo de newton (metodos numericos 4 the win)

float sqrtf(float x) {
    if (x < 0) return -1;
    float r = x < 1 ? 1 : x;      // initial guess
    for (int i = 0; i < 6; i++)
        r = 0.5 * (r + x/r);
    return r;
}
    
uint32_t int_sqrt(uint32_t x) {
    // Simple integer square‐root via binary search / Newton’s method.
    // You can replace this with any fast integer-sqrt you already have.
    uint32_t op = x;
    uint32_t res = 0;
    uint32_t one = 1uL << 30;  // 2^30, highest power of four <= UINT32_MAX

    // "one" starts at the highest power of four <= the argument.
    while (one > op) {
        one >>= 2;
    }

    while (one != 0) {
        if (op >= res + one) {
            op -= res + one;
            res = (res >> 1) + one;
        } else {
            res = res >> 1;
        }
        one >>= 2;
    }
    return res;
}

/**
 * @brief Round a float to the nearest integer, halfway cases away from zero.
 * @param x The float to round.
 * @return The rounded value as a float.
 */
float roundf(float x) {
    if (x >= 0.0f) {
        // Add 0.5 then truncate
        return (float)((int)(x + 0.5f));
    } else {
        // Subtract 0.5 then truncate (truncation goes toward zero)
        return (float)((int)(x - 0.5f));
    }
}