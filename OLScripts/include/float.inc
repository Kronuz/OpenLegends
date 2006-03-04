#if defined _float_included
  #endinput
#endif

#pragma rational float

enum floatround_method {
  floatround_round, 	/* Rounds mathematically to the nearest integer. */
  floatround_floor, 	/* Rounds down to the nearest integer. */
  floatround_ceil	/* Rounds up to the nearest integer. */
};

//Convert to float
native float:float(value);
native float:floatstr(string[]);

//Operations
native float:floatmul(float:val1, float:val2);
native float:floatdiv(float:val1, float:val2);
native float:floatadd(float:val1, float:val2);
native float:floatsub(float:val1, float:val2);
native float:floatfract(float:val);
native floatround(float: value, floatround_method = floatround_round);
native floatcmp(float:val1, float:val2);

//Integer Functions
native abs(val1);

//Functions
native float:floatabs(float:val);
native float:floatmod(float:val1, float:val2);
native float:exp(float:val);
native float:log(float:val);
native float:log10(float:val);
native float:pow(float:val1, float:val2);
native float:sqrt(float:val1);
native float:sin(float:degrees);
native float:cos(float:degrees);
native float:tan(float:degrees);
native float:acos(float:degrees);
native float:asin(float:degrees);
native float:atan(float:degrees);
native float:cosh(float:degrees);
native float:sinh(float:degrees);
native float:tanh(float:degrees);


