#ifndef STATICCHARACTERARRAY_H
#define STATICCHARACTERARRAY_H

// Justin Furtado
// 4/4/2017
// StaticCharacterArray.h
// Encapsulates a large array for storing c-style strings and accessing them

class StaticCharArray
{
public:
	static const char *GetNextStringPtr();
	static void AddChar(char c);
};

#endif // ifndef STATICCHARACTERARRAY_H