/*  Float arithmetic for the Small AMX engine
 *
 *  Copyright (c) Artran, Inc. 1999
 *  Written by Greg Garner (gmg@artran.com)
 *  This file may be freely used. No warranties of any kind.
 *
 * Modified by Kronuz for the Open Zelda Project. August 10, 2003
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>      /* for NULL */
#include <stdlib.h>     /* for atof() */
#include <math.h>
#include "amx.h"

#define D2RF 0.017453292519943295769236907684886f

// Private function to
// allocate and fill a C style string from a small type string.
static char *pcCreateAndFillStringFromCell(AMX *amx,cell params)
{
    char *szDest;
    int nLen;
    cell *pString;

    // Get the real address of the string.
    amx_GetAddr(amx,params,&pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, &nLen);
    szDest = new char[nLen+1];

    // Now convert the Small String into a C type null terminated string
    amx_GetString(szDest, pString);

    return szDest;
}

/*static*/ cell ConvertFloatToCell(float fValue)
{
    float *pFloat;
    cell fCell;

    // Get a pointer to the cell that is a float pointer.
    pFloat = (float *)((void *)&fCell);

    // Now put the float value into the cell.
    *pFloat = fValue;

    // Return the cell that contains the float
    return fCell;
}

/*static*/ float fConvertCellToFloat(cell cellValue)
{
    float *pFloat;

    // Get a pointer to the cell that is a float pointer.
    pFloat = (float *)((void *)&cellValue);

    // Return the float
    return *pFloat;
}

#if defined OZDoc
float float(float value); /*!< 
	\ingroup float
	\brief Convert an integer into a floating point value.

	\param value Integer to convert to float

	\return Floating point version of value.

	\remarks Most of the time you won't need to use float(). There are overloaded operators
		every possible operation, so explicit conversion it's not used too often.

	<b>Example:</b>
	\code
	...
	new factor = 2;
	new float: gravity = 9.8;
	gravity *= float(factor);
	// now gravity value is 19.6   
	...
	\endcode
*/
#endif
static cell _float(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = long value to convert to a float
    */
    float fValue;

    // Convert to a float. Calls the compilers long to float conversion.
    fValue = (float) params[1];

    // Return the cell.
    return ConvertFloatToCell(fValue);
}

static cell _floatstr(AMX *amx, cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = virtual string address to convert to a float
    */
    char *szSource;
    float fNum;
    long lCells;

    lCells = params[0]/sizeof(cell);

    // They should have sent us 1 cell.
    assert(lCells==1);

    // Convert the Small string to a C style string.
    szSource = pcCreateAndFillStringFromCell(amx, params[1]);

    // Now convert this to a float.
    fNum = (float)atof(szSource);

    // Delete the string storage that was newed...
    delete[] szSource;

    return ConvertFloatToCell(fNum);
}

static cell _floatmul(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA * fB;

    return ConvertFloatToCell(fRes);
}

static cell _floatdiv(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float dividend (top)
    *   params[2] = float divisor (bottom)
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA / fB;

    return ConvertFloatToCell(fRes);
}

static cell _floatadd(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA + fB;

    return ConvertFloatToCell(fRes);
}

static cell _floatsub(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA - fB;

    return ConvertFloatToCell(fRes);
}

// Return fractional part of float
static cell _floatfract(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    */
    float fA;

    fA = fConvertCellToFloat(params[1]);
    fA = fA - (float)(floor((double)fA));

    return ConvertFloatToCell(fA);
}

#if defined OZDoc
/*! 
	\ingroup float
	\brief Tagged enum floatround_method type.
	Used by floatround to select the method to be used during the rounding process.
*/
 enum floatround_method {
  floatround_round, /*!< Rounds mathematically to the nearest integer. */
  floatround_floor, /*!< Rounds down to the nearest integer. */
  floatround_ceil	/*!< Rounds up to the nearest integer. */
}; 

floatround(float value, floatround_method method = floatround_round); /*!< 
	\ingroup float
	\brief Return integer part of float, rounded.

	\param value Float to round.

	\param method Method to round the float number.
		This can be floatround_round, floatround_floor, or floatround_ceil, as
		defined in the floatround_method tagged enum.

	\return Integer part of the float, rounded by the specified method.

	\remarks
	You can also use the stocked functions ceil(), floor(), and round() instead.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: number = 123.54;
	new round, down, up;

	round = floatround(number, fround_round);
	down = floatround(number, fround_floor);
	up = floatround(number, fround_ceil);

	printf("original number     %f^n", number);   
	printf("rounded number      %d^n", round);
	printf("number rounded down %d^n", down);
	printf("number rounded up   %d^n", up);
}
	\endcode

	<b>Output:</b>
	\verbatim 
original number     123.540000
rounded number      124
number rounded down 123
number rounded up   124 \endverbatim 

	\sa floatmod()
*/
#endif
static cell _floatround(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    *   params[2] = Type of rounding (long)
    */
    float fA;

    fA = fConvertCellToFloat(params[1]);

    switch (params[2])
    {
        case 1:       /* round downwards (truncate) */
            fA = (float)(floor((double)fA));
            break;
        case 2:       /* round upwards */
            fA = (float)(ceil((double)fA));
            break;
        default:      /* standard, round to nearest */
            fA = (float)(floor((double)fA + 0.5f));
            break;
    }

    return (long)fA;
}

static cell _floatcmp(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    if (fA == fB)
        return 0;
    else if (fA>fB)
        return 1;
    else
        return -1;

}

/* FUNCTIONS ADDED BY OPEN ZELDA PROJECT: */
/* (All these functions work as the C ANSI standard Math library 
	except on that these use degrees instead of radians) */
#if defined OZDoc
abs(cell value); /*!< 
	\ingroup float
	\brief Gets the absolute value of an integer.

	\param value Integer value.

	\return The <b>abs</b> function returns the absolute value of its parameter. There is no error return.

	\remarks
	This routine always returns the absolute value of its argument.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: fnumber = -3.141593
	new number = -5;

	printf("The absolute value of %d is %d^n", number, abs(number));
	printf("The absolute value of %f is %f^n", fnumber, floatabs(fnumber));   

}
	\endcode

	<b>Output:</b>
	\verbatim 
The absolute value of -5 is 5
The absolute value of -3.141593 is 3.141593 \endverbatim 

	\sa floatabs()
*/
#endif
static cell AMX_NATIVE_CALL Abs(AMX *amx, cell *params)
{
	return abs((int)params[1]);
}
static cell AMX_NATIVE_CALL Exp(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)exp(a) );
}
#if defined OZDoc
float floatabs(float value); /*!< 
	\ingroup float
	\brief Calculates the absolute value of a floting-point number.

	\param value Floating point number.

	\return <b>floatabs</b> returns the absolute floating point value of its argument. 
		There is no error return.

	\remarks
	This routine returns the absolute value of its argument.

	<b>Example:</b>
	See the example for abs().

	\sa abs()

*/
#endif
static cell AMX_NATIVE_CALL Fabs(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)fabs(a) );
}
static cell AMX_NATIVE_CALL Fmod(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	float b = fConvertCellToFloat( params[2] );
	return ConvertFloatToCell( (float)fmod(a,b) );
}

#if defined OZDoc
float log(float x); /*!<
	\ingroup float
	\brief Natural logarithm function.

	\param x Floating point value whose logarithm is to be found.

	\return
	- On success, returns the natural log of x.
	- On error, returns a negative value.

	\remarks
	Calculates the natural logarithm of x.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
   new float: result;
   new float: x = 9000.0;

   result = log(x);
   printf("The natural log of %f is %f^n", x, result);   

   result = log10(x);
   printf("The common log of %f is %f^n", x, result);

}
	\endcode

	<b>Output:</b>
	\verbatim 
The natural log of 9000.000000 is 9.104980
The common log of 9000.000000 is 3.954243 \endverbatim 

	\sa log10() | exp() | pow()
*/
#endif
static cell AMX_NATIVE_CALL Log(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)log(a) );
}
#if defined OZDoc
float log10(float x); /*!<
	\ingroup float
	\brief Common logarithm function.

	\param x Floating point value whose logarithm is to be found.

	\return
	- On success, returns the log (base 10) of x.
	- On error, returns a negative value.

	\remarks
	Calculates the base 10 logarithm of x.

	<b>Example:</b>
	See the example for log().

	\sa log() | exp() | pow()
*/
#endif
static cell AMX_NATIVE_CALL Log10(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)log10(a) );
}
#if defined OZDoc
float pow(float x, float y); /*!<
	\ingroup float
	\brief Power function, x to the y (x^y, or x**y).

	\param x, y Floating point numbers.

	\return
	- On success, returns the calculated value x^y.
	- On error, returns a negative value.

	\remarks
	Calculates x^y. Sometimes the arguments passed to this function 
	produce results that overflow or are incalculable. When the correct 
	value would overflow, pow returns -1.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: x = 2.0;
	new float: y = 3.0;

	printf("%f raised to %f is %f^n", x, y, pow(x, y));   
}
	\endcode

	<b>Output:</b>
	\verbatim 
2.000000 raised to 3.000000 is 8.000000 \endverbatim 

*/
#endif
static cell AMX_NATIVE_CALL Pow(AMX *amx, cell *params)
{
	float a = fConvertCellToFloat( params[1] );
	float b = fConvertCellToFloat( params[2] );
	return ConvertFloatToCell( (float)pow(a,b) );
}
static cell AMX_NATIVE_CALL Sqrt(AMX *amx, cell *params)
{
	// Returns the square root of a floating-point number
	float a = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)sqrt(a) );
}

#if defined OZDoc
float sin(float degrees); /*!< 
	\ingroup float
	\brief Returns the sine trigonometric function of an angle.

	\param degrees Angle in degrees.

	\return The sine of the given degrees.

	\remarks The angle is expected to be in degrees.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: deg = 360.0;
	new float: x, y;

	x = deg / 8;
	y = sin(x);
	printf( "The sine of %f degrees is %f^n", x, y );
	x = sinh(y);
	printf( "The hyperbolic sine of %f is %f^n", y, x );

	x = deg / 8;
	y = cos(x);
	printf( "The cosine of %f degrees is %f^n", x, y );
	x = cosh(y);
	printf( "The hyperbolic cosine of %f is %f^n", y, x );   
}

	\endcode

	<b>Output:</b>
	\verbatim 
The sine of 45.000000 degrees is 0.707107
The hyperbolic sine of 0.707107 is 0.767523
The cosine of 45.000000 degrees is 0.707107
The hyperbolic cosine of 0.707107 is 1.260592 \endverbatim 

	\sa acos() | asin() | atan() | sinh() | cos() | cosh() | tan() | tanh()
*/
float Sin(cell degrees); /*!< 
	\ingroup float
	\brief Returns the sine trigonometric function of an angle.

	\param degrees Angle in degrees.

	\return The sine of the given degrees.

	\remarks The angle is expected to be in degrees.

	\deprecated This function name has been deprecated and is only provided
		for compatibility it should no longer be used since future versions
		might not include it at all. Developers creating scripts using this 
		function are urged to use sin() instead.

	<b>Example:</b>
	See the example for sin().

	\sa acos() | asin() | atan() | sin() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Sin(AMX *amx, cell *params)
{
	// Convert to radians
	float radians = D2RF * fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)sin(radians) );
}

#if defined OZDoc
float cos(float degrees); /*!< 
	\ingroup float
	\brief Returns the cosine trigonometric function of an angle.

	\param degrees Angle in degrees.

	\return The cosine of the given degrees.

	\remarks The angle is expected to be in degrees.

	<b>Example:</b>
	See the example for sin().

	\sa acos() | asin() | atan() | sin() | sinh() | cosh() | tan() | tanh()
*/
float Cos(cell degrees); /*!< 
	\ingroup float
	\brief Returns the cosine trigonometric function of an angle.

	\param degrees Angle in degrees.

	\return The cosine of the given degrees.

	\remarks The angle is expected to be in degrees.

	\deprecated This function name has been deprecated and is only provided
		for compatibility it should no longer be used since future versions
		might not include it at all. Developers creating scripts using this 
		function are urged to use cos() instead.

	<b>Example:</b>
	See the example for sin().

	\sa acos() | asin() | atan() | sin() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Cos(AMX *amx, cell *params)
{
	// Convert to radians
	float radians = D2RF * fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)cos(radians) );
}

#if defined OZDoc
float acos(float x); /*!< 
	\ingroup float
	\brief Calculates the arccosine.

	\param x Value between –1 and 1 whose arccosine is to be calculated.

	\return 
		- On success, the <b>acos</b> function returns the arccosine of x in the range 0 to 180 degrees. 
		- On error, if x is less than –1 or greater than 1, acos returns an indefinite value.

	\remarks 
	The resulting angle is returned in degrees.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: deg = 15.0;
	new float: x, y;

	x = sin(deg);
	printf( "The sine of %f degrees is %f^n", deg, x );
	y = asin(x);
	printf( "The arcsine of of %f is %f^n", x, y );

	x = cos(deg);
	printf( "The cosine of %f degrees is %f^n", deg, x );
	y = acos(x);
	printf( "The arccosine of %f is %f^n", x, y );   
}
	\endcode

	<b>Output:</b>
	\verbatim 
The sine of 15.000000 degrees is 0.258819
The arcsine of 0.258819 is 15.000000
The cosine of 15.000000 degrees is 0.965926
The arccosine of 0.965926 is 15.000000 \endverbatim 

	\sa asin() | atan() | sin() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Acos(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)acos(x) / D2RF );
}
#if defined OZDoc
float asin(float x); /*!< 
	\ingroup float
	\brief Calculates the arcsine.

	\param x Value between –1 and 1 whose arcsine is to be calculated.

	\return 
		- On success, the <b>asin</b> function returns the arcsine of x in the range 0 to 180 degrees. 
		- On error, if x is less than –1 or greater than 1, acos returns an indefinite value.

	\remarks 
	The resulting angle is returned in degrees.

	<b>Example:</b>
	See the example for acos().

	\sa acos() | atan() | sin() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Asin(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)asin(x) / D2RF );
}
#if defined OZDoc
float atan(float x); /*!< 
	\ingroup float
	\brief Calculates the arccosine.

	\param x Any number.

	\return <b>atan</b> returns the arctangent of x in the range of –90 to 90 degrees. 
		If x is 0, atan returns 0.

	\remarks 
	The atan function calculates the arctangent of x.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: deg = 45.0;
	new float: x, y;

	x = sin(deg);
	printf( "The tangent of %f degrees is %f^n", deg, x );
	y = asin(x);
	printf( "The arctangent of of %f is %f^n", x, y );
}
	\endcode

	<b>Output:</b>
	\verbatim 
The tangent of 45.000000 degrees is 1.000000
The arctangent of 1.000000 is 45.000000 \endverbatim 

	\sa asin() | acos() | sin() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Atan(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)atan(x) / D2RF );
}
#if defined OZDoc
float cosh(float x); /*!< 
	\ingroup float
	\brief Returns the hyperbolic cosine trigonometric function of x.

	\param x the value to calculate the hyperbolic cosine from.

	\return The floating point hyperbolic cosine of x.

	\remarks Computes the hyperbolic cosine: (e^x + e^-x) / 2.

	<b>Example:</b>
	See the example for sin().

	\sa acos() | asin() | atan() | sin() | sinh() | cos() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Cosh(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)cosh(x) );
}
#if defined OZDoc
float sinh(float x); /*!< 
	\ingroup float
	\brief Returns the hyperbolic sine trigonometric function of x.

	\param x the value to calculate the hyperbolic sine from.

	\return The floating point hyperbolic sine of x.

	\remarks Computes the hyperbolic sine: (e^x - e^-x) / 2.

	<b>Example:</b>
	See the example for sin().

	\sa acos() | asin() | atan() | sin() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Sinh(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)sinh(x) );
}
#if defined OZDoc
float tan(float degrees); /*!< 
	\ingroup float
	\brief Calculate the tangent trigonometric function of an angle.

	\param degrees Angle in degrees.

	\return 
		- On success, the <b>tan</b> function returns the tangent of x in the valid range -1 to 1. 
		- On error, returns a value outside the valid range.

	\remarks The angle is expected to be in degrees.

	<b>Example:</b>
	\code
#include <console>
#include <float>

main()
{
	new float: deg = 360.0;
	new float: x, y;

	x = tan(deg / 8);
	y = tanh(x);
	printf( "The tangent of %f degrees is %f^n", deg/8, x );
	printf( "The hyperbolic tangent of %f is %f^n", x, y );
}

	\endcode

	<b>Output:</b>
	\verbatim 
The tangent of 45.000000 degrees is 1.000000
The hyperbolic tangent of 1.000000 is 0.761594 \endverbatim 

	\sa acos() | asin() | atan() | sinh() | cos() | cosh() | tan() | tanh()
*/
#endif
static cell AMX_NATIVE_CALL Tan(AMX *amx, cell *params)
{
	// Convert to radians
	float radians = D2RF * fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)tan(radians) );
}

#if defined OZDoc
float tanh(float x); /*!< 
	\ingroup float
	\brief Returns the hyperbolic tangent trigonometric function of x.

	\param x the value to calculate the hyperbolic tangent from.

	\return The floating point hyperbolic tangent of x.

	\remarks Computes the hyperbolic tangent: sinh(x) / cosh(x).

	<b>Example:</b>
	See the example for tan().

	\sa acos() | asin() | atan() | sin() | sinh() | cos() | cosh() | tan()
*/
#endif
static cell AMX_NATIVE_CALL Tanh(AMX *amx, cell *params)
{
	float x = fConvertCellToFloat( params[1] );
	return ConvertFloatToCell( (float)tanh(x) );
}

extern "C" AMX_NATIVE_INFO float_Natives[] = {
  { "float",         _float },
  { "floatstr",      _floatstr },
  { "floatmul",      _floatmul },
  { "floatdiv",      _floatdiv },
  { "floatadd",      _floatadd },
  { "floatsub",      _floatsub },
  { "floatfract",    _floatfract},
  { "floatround",    _floatround},
  { "floatcmp",      _floatcmp},

  { "abs",			Abs},	/* Added by Open Zelda project */
  { "acos",   		Acos},
  { "asin",  		Asin},
  { "atan",   		Atan},
  { "cos",  		Cos},
  { "cosh",  		Cosh},
  { "exp",   		Exp},
  { "floatabs", 	Fabs},
  { "floatmod", 	Fmod},
  { "log",   		Log},
  { "log10", 		Log10},
  { "pow",   		Pow},
  { "sin",  		Sin},
  { "sinh",  		Sinh},
  { "tan",   		Tan},
  { "tanh",  		Tanh},
  { "sqrt",  		Sqrt},
  { NULL, NULL }        /* terminator */
};
