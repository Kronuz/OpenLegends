/* Float arithmetic
 *
 * (c) Copyright 1999, Artran, Inc.
 * Written by Greg Garner (gmg@artran.com)
 * Modified in March 2001 to include user defined
 * operators for the floating point functions.
 *
 * This file is provided as is (no warranties).
 *
 * Modified by Kronuz for the Open Zelda Project. August 20, 2003.
 */
#if defined _float_included
  #endinput
#endif
#define _float_included

/* Different methods of rounding */
enum floatround_method {
  floatround_round,
  floatround_floor,
  floatround_ceil
}

/**************************************************/
/* Convert an integer into a floating point value */
native float:float(value);

/**************************************************/
/* Return the Current Time Delta as a float */
native float:GetTimeDelta();

/**************************************************/
/* Gets the absolute value of an integer.  */
native abs( value );

native float:floatabs( float:value );

native float:cos( float:degrees );
native float:acos( float:x );
native float:cosh( float:x );

native float:sin( float:degrees );
native float:asin( float:x );
native float:sinh( float:x );

native float:tan( float:degrees );
native float:tanh( float:x );
native float:atan( float:x );

native float:exp( float:number );

/**************************************************/
/* Calculate the natural logarithm of x */
native float:log( float:x );

/**************************************************/
/* Calculate the base 10 logarithm of x */
native float:log10( float:x );

/**************************************************/
/* Power function, x to the y (x^y) */
native float:pow( float:x, float:y );

/**************************************************/
/* Calculates the positive square root of the input value */
native float:sqrt( float:value );

native float:floatmod( float:oper1, float:oper2 );

/**************************************************/
/* Convert a string into a floating point value */
native float:floatstr(const string[]);

/**************************************************/
/* Multiple two floats together */
native float:floatmul(float:oper1, float:oper2);

/**************************************************/
/* Divide the dividend float by the divisor float */
native float:floatdiv(float:dividend, float:divisor);

/**************************************************/
/* Add two floats together */
native float:floatadd(float:dividend, float:divisor);

/**************************************************/
/* Subtract oper2 float from oper1 float */
native float:floatsub(float:oper1, float:oper2);

/**************************************************/
/* Return the fractional part of a float */
native float:floatfract(float:value);

/**************************************************/
/* Round a float into a integer value */
native floatround(float:value, floatround_method:method = floatround_round);

/**************************************************/
/* Compare two integers. If the two elements are equal, return 0.
   If the first argument is greater than the second argument, return 1,
   If the first argument is less than the second argument, return -1. */
native floatcmp(float:fOne, float:fTwo);


/**************************************************/
#pragma rational float

stock round(float:number)
    return floatround(number, fround_round);

stock ceil(float:number)
    return floatround(number, fround_ceil);

stock floor(float:number)
    return floatround(number, fround_floor);

/* user defined operators */
native float:operator%(float:oper1, float:oper2) = floatmod;
native float:operator*(float:oper1, float:oper2) = floatmul;
native float:operator/(float:oper1, float:oper2) = floatdiv;
native float:operator+(float:oper1, float:oper2) = floatadd;
native float:operator-(float:oper1, float:oper2) = floatsub;

stock float:operator++(float:oper)
    return oper+1.0;

stock float:operator--(float:oper)
    return oper-1.0;

stock float:operator-(float:oper)
    return oper^float:0x80000000;               /* IEEE values are sign/magnitude */

stock float:operator%(float:oper1, oper2)
	return floatmod(oper1, float(oper2));

stock float:operator%(oper1, float:oper2)
	return floatmod(float(oper1), oper2);

stock float:operator*(float:oper1, oper2)
    return floatmul(oper1, float(oper2));       /* "*" is commutative */

stock float:operator/(float:oper1, oper2)
    return floatdiv(oper1, float(oper2));

stock float:operator/(oper1, float:oper2)
    return floatdiv(float(oper1), oper2);

stock float:operator+(float:oper1, oper2)
    return floatadd(oper1, float(oper2));       /* "+" is commutative */

stock float:operator-(float:oper1, oper2)
    return floatsub(oper1, float(oper2));

stock float:operator-(oper1, float:oper2)
    return floatsub(float(oper1), oper2);

stock bool:operator==(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) == 0;

stock bool:operator==(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) == 0;  /* "==" is commutative */

stock bool:operator!=(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) != 0;

stock bool:operator!=(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) != 0;  /* "==" is commutative */

stock bool:operator>(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) > 0;

stock bool:operator>(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) > 0;

stock bool:operator>(oper1, float:oper2)
    return floatcmp(float(oper1), oper2) > 0;

stock bool:operator>=(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) >= 0;

stock bool:operator>=(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) >= 0;

stock bool:operator>=(oper1, float:oper2)
    return floatcmp(float(oper1), oper2) >= 0;

stock bool:operator<(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) < 0;

stock bool:operator<(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) < 0;

stock bool:operator<(oper1, float:oper2)
    return floatcmp(float(oper1), oper2) < 0;

stock bool:operator<=(float:oper1, float:oper2)
    return floatcmp(oper1, oper2) <= 0;

stock bool:operator<=(float:oper1, oper2)
    return floatcmp(oper1, float(oper2)) <= 0;

stock bool:operator<=(oper1, float:oper2)
    return floatcmp(float(oper1), oper2) <= 0;

stock bool:operator!(float:oper)
    return floatcmp(oper1, 0.0) == 0;

/* Deprecated functions: **************************/

/* Sin( degrees )                                 */
stock float:Sin( degrees )
	return sin(float(degrees));
	
/* Cos( degrees )                                 */
stock float:Cos( degrees )
	return cos(float(degrees));

/* Calculate the Square Root of a float		      */
stock float:Sqrt( float:number )
	return sqrt(number);
