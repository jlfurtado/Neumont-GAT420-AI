#include "HashFunctions.h"
#include <math.h>
#include <iostream>

// Justin Furtado
// 4/5/2017
// HashFunctions.cpp
// Encapsulates static methods

// First bullet point hash from lab doc
unsigned int HashFunctions::Hash1(const char * str)
{
	return 0;	// puts everything in the first bucket
}

// "Return the ascii value of the first letter of the word"
unsigned int HashFunctions::Hash2(const char * str)
{
	return *str;
}

// "Return the sum of the ascii values of the first three letters of the word (or, for example, two for “or” or one for “a”)."
unsigned int HashFunctions::Hash3(const char * str)
{
	char c[3];
	c[0] = *str;
	c[1] = c[0] ? *(str + 1) : '\0';
	c[2] = c[1] ? *(str + 2) : '\0';
	return c[0] + c[1] + c[2];
}

// "Return the sum of the ascii values of all the letters in the word."
unsigned int HashFunctions::Hash4(const char * str)
{
	unsigned int sum = 0;
	while (*str) { sum += *str++; }
	return sum;
}

// "Return the xor of the ascii values of all the letters in the word, left shifting by n for the nth letter."
unsigned int HashFunctions::Hash5(const char * str)
{
	unsigned int result = *str;
	for (int i = 1; *(str + i); ++i)
	{
		result = result ^ (*(str + i) << i);
	}
	return result;
}

const int SIZE = 8 * sizeof(unsigned int);
// "One other of your own experimentation that works better than any of these"
unsigned int HashFunctions::Hash6(const char * str)
{
	unsigned int result = *str;
	for (int i = 1; *(str + i); ++i)
	{
		int x = (int)powf(*(str + i), 4.125f);
		result ^= (x >> i | x << (SIZE - i));
	}

	return result;
}
