#ifndef HASHFUNCTIONS_H
#define HASHFUNCTIONS_H

// Justin Furtado
// 4/5/2017
// HashFunctions.h
// Encapsulates static methods

typedef unsigned int(*HashCallback)(const char *const foundString);

class HashFunctions
{
public:
	static unsigned int Hash1(const char *str);
	static unsigned int Hash2(const char *str);
	static unsigned int Hash3(const char *str);
	static unsigned int Hash4(const char *str);
	static unsigned int Hash5(const char *str);
	static unsigned int Hash6(const char *str);
};

#endif // ifndef HASHFUNCTIONS_H