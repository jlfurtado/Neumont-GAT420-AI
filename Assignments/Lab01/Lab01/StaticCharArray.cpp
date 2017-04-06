#include "StaticCharArray.h"
#include <iostream>

// Justin Furtado
// 4/4/2017
// StaticCharacterArray.h
// Encapsulates a large array for storing c-style strings and accessing them


const int MAX_CHARS = 10000000;
char s_uniqueWords[MAX_CHARS]{ 0 };
int s_nextSlot = 0;

const char * StaticCharArray::GetNextStringPtr()
{
	return &s_uniqueWords[s_nextSlot];
}

void StaticCharArray::AddChar(char c)
{
	if (s_nextSlot < MAX_CHARS) { s_uniqueWords[s_nextSlot++] = c; }
	else { printf("Static char array ran out of space!\n"); }
}
