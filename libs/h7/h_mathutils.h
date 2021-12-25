#ifndef H_MATHUTILS_H
#define H_MATHUTILS_H

//#define h7_nanoToSec 1/1000000000.0f
//#define h7_PI 3.14159265
//#define h7_PI2 (h7_PI * 2)
//#define h7_radiansToDegrees (180/h7_PI)
//#define h7_radDeg h7_radiansToDegrees
//#define h7_degreesToRadians (h7_PI/180)
//#define h7_degRad h7_degreesToRadians

#define H7_SHIFT_R_LOGIC(n, c) ((unsigned int)(n) >> c)
#define H7_MAX(a, b) (a > b ? a : b)
#define H7_MIN(a, b) (a > b ? b : a)

int h7_nextPowerOfTwo(int value);
int h7_isPowerOfTwo (int value);

/**
     * Returns the number of zero bits following the lowest-order ("rightmost")
     * one-bit in the two's complement binary representation of the specified
     * {@code int} value.  Returns 32 if the specified value has no
     * one-bits in its two's complement representation, in other words if it is
     * equal to zero.
     *
     * @param i the value whose number of trailing zeros is to be computed
     * @return the number of zero bits following the lowest-order ("rightmost")
     *     one-bit in the two's complement binary representation of the
     *     specified {@code int} value, or 32 if the value is equal
     *     to zero.
     */
int h7_numberOfTrailingZeros(int i);
int h7_bitCount(int i);
int h7_random(int i);

#endif // H_MATHUTILS_H
