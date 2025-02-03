//#define MY_MATH_IMPLEMENTATION

#pragma once

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)
float floorf(float x);


float floorf(float x)
{
	int i = (int)x;
    return (x < 0 && x != i) ? i - 1 : i;
}
