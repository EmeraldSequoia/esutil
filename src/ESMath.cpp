#include "ESMath.hpp"

#include <math.h>

double
EC_fmod(double arg1,
	double arg2)
{
    return (arg1 - floor(arg1/arg2)*arg2);
}
