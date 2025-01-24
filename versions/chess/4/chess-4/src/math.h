#ifndef MY_MATH
#define MY_MATH
float floorf(float x);
#endif // MY_MATH

#ifdef MY_MATH_IMPLEMENTATION
#undef MY_MATH_IMPLEMENTATION
float floorf(float x)
{
	int i = (int)x;
    return (x < 0 && x != i) ? i - 1 : i;
}
#endif // MY_MATH_IMPLEMENTATION
