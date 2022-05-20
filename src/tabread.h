#ifndef tabread_h
#define tabread_h

#include <stdlib.h>

static float read1(const float *tab, size_t size, float index) __attribute__((used));
static float read2(const float *tab, size_t size, float index) __attribute__((used));


static float read1(const float *tab, size_t size, float index)
{
	index = index - (int)index;
	float p = index * size + 0.5;
	size_t i = p;
	if(i >= size) i-= size;
	return tab[i];
}


static float read2(const float *tab, size_t size, float index)
{
	index = index - (int)index;
	float p = index * size;
	size_t i = p;
	size_t j = i+1;
	if(j >= size) j -= size;
	float a0 = p - i;
	float a1 = 1.0 - a0;

	float v0 = tab[i];
	float v1 = tab[j];

	return v0 * a1 + v1 * a0;
}



#endif
