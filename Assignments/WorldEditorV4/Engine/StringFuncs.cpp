#include "StringFuncs.h"
#include "StringFuncs.h"
#include "StringFuncs.h"
#include "GameLogger.h"

// Justin Furtado
// 5/4/2016
// StringFuncs.h
// Updated c-style string manipulation utilities

namespace Engine
{
	// initialize static char array
	char StringFuncs::s_buffer[PLENTY]{ '\0' };
	int StringFuncs::s_nextBufferSlot{ 0 };

	// Counts characters in a string by iterating through them until '\0' is encountered
	int StringFuncs::StringLen(const char *const str)
	{
		if (!str) return 0;
		int pos = 0;

		while (*(str + pos))
		{
			pos++;
		}

		return pos;
	}

	// Compares two strings by first comparing their lengths and, if they are of the same length, comparing each character in the string to the character at the same index in the other string.
	// If any characters are not equal, the strings are not equal
	bool StringFuncs::StringsAreEqual(const char *const str1, const char *const str2)
	{
		if (!str1 || !str2) return str1 == str2;

		if (StringLen(str1) != StringLen(str2)) return false;

		for (int pos = 0; *(str1 + pos) && *(str2 + pos); ++pos)
		{
			if (*(str1 + pos) != *(str2 + pos)) return false;
		}

		return true;
	}

	// Tests if a string is a palindrome by comparing the first half of the characters to the second half of the characters, if they are not equal, it is not a palindrome
	// Ex: madam, m == m && a == a, true
	bool StringFuncs::IsPalindrome(const char *const str)
	{
		if (!str) return true;

		for (int pos = 0, last = StringLen(str) - 1; last - pos > pos; ++pos)
		{
			if (*(str + pos) != *(str + last - pos)) return false;
		}

		return true;
	}

	// Compares each of the characters in each string until either a character that is not equal or the end of a string has been reached.
	// Then it returns + or - 1 depending on which comes first alphabetically
	int StringFuncs::StringCompare(const char *const str1, const char *const str2)
	{
		if (!str1 || !str2) return str1 == str2 ? 0 : str1 < str2 ? 1 : -1;

		int pos;

		for (pos = 0; *(str1 + pos) && *(str2 + pos); ++pos)
		{
			int diff = ((*(str1 + pos)) - (*(str2 + pos)));
			if (diff) return (diff < 0 ? -1 : 1); // if not equal, reurn + or - 1 based on diff
		}

		// end of a string reached
		int diff = ((*(str1 + pos)) - (*(str2 + pos)));
		if (!diff) return 0;
		else return diff < 0 ? -1 : 1;
	}

	// Locates the first occurance of substring target within string source, if not found -1 is returned
	// iterates through source, comparing characters to target. if equal, it compares to the next character in target, else it starts looking again
	// if the end of target is reached when before the end of the source has been reached, a substring has been found that ends at sPos
	int StringFuncs::FindSubString(const char *const source, const char *const target)
	{
		if (StringLen(source) < StringLen(target)) return -1; // definitely not a substring
		if (StringLen(target) == 0) return 0; // empty string is always found

		for (int sPos = 0, tPos = 0; *(source + sPos); ++sPos)
		{
			if (*(source + sPos) == *(target + tPos))
			{
				tPos++;
				if (!(*(target + tPos))) return sPos - tPos + 1;
			}
			else
			{
				tPos = 0;

				if (*(source + sPos) == *(target + tPos))
				{
					tPos++;
					if (!(*(target + tPos))) return sPos - tPos + 1;
				}
			}
		}

		// end of string reached
		return -1;
	}

	// reads the end of a string to determine if it ends with the target string
	bool StringFuncs::StringEndsWith(const char *const source, const char *const target)
	{
		if (!source || !target) return false;
		if (target == source) return true;

		for (int pos = 0, len = StringLen(target), srcLen = StringLen(source); *(target + pos); ++pos)
		{
			if (*(source + srcLen + pos - len) != *(target + pos))
			{
				return false;
			}
		}

		return true;
	}

	// Swaps the first half of the characters in a given string with the last half of the characters in the string
	char *StringFuncs::StringReverse(char *const str)
	{
		if (!str) return nullptr;

		for (int pos = 0, last = StringLen(str) - 1; last - pos > pos; ++pos)
		{
			char t = *(str + last - pos);
			*(str + last - pos) = *(str + pos);
			*(str + pos) = t;
		}

		return str;
	}

	// Converts uppercase characters ('A' - 'Z') to lowercase characters ('a' - 'z'), leaves other characters in string untouched
	char *StringFuncs::ToLower(char *const str)
	{
		if (!str) return nullptr;

		for (int pos = 0; *(str + pos); ++pos)
		{
			if (*(str + pos) >= 'A' && *(str + pos) <= 'Z') *(str + pos) += ('a' - 'A');
		}

		return str;
	}

	// Converts lowercase characters ('a' - 'z') into uppercase ones ('A' - 'Z'), leaves other characters in string untouched
	char *StringFuncs::ToUpper(char *const str)
	{
		if (!str) return nullptr;

		for (int pos = 0; *(str + pos); ++pos)
		{
			if (*(str + pos) >= 'a' && *(str + pos) <= 'z') *(str + pos) -= ('a' - 'A');
		}

		return str;
	}

	// Copies up to a maximum of targetSize - 1 characters into target from source then adds '\0' to the end
	// Returns number of characters copied
	int StringFuncs::StringCopy(const char *const source, char *const target, int targetSize)
	{
		if (!source || !target || targetSize <= 0) return 0;

		int pos, length;
		for (pos = 0, length = StringLen(source); pos < targetSize - 1 && pos < length; ++pos)
		{
			*(target + pos) = *(source + pos);
		}

		*(target + pos) = '\0';

		return pos;
	}

	// coppies up to targetSize - 1 characters from str1, sep and str2 into target in a way that resembles string concatenation
	int StringFuncs::StringConcatIntoBuffer(const char *const str1, const char *const str2, const char *const sep, char *const target, int targetSize)
	{
		if (!str1 || !str2 || !sep || !target || targetSize <= 0) return 0;

		int pos, len1, len2, len3;

		for (pos = 0, len1 = StringLen(str1), len2 = StringLen(str2), len3 = StringLen(sep); pos < targetSize - 1 && pos < (len1 + len2 + len3); ++pos)
		{
			*(target + pos) = ((pos < len1) ? *(str1 + pos) : ((pos < len1 + len3) ? *(sep - len1 + pos) : *(str2 - len1 - len3 + pos)));
		}

		*(target + pos) = '\0';

		return pos;
	}

	bool StringFuncs::GetSingleFloatFromString(const char * const string, float & outValue)
	{
		if (!string) return false;
		if (*(string) == '\0') { GameLogger::Log(MessageType::Warning, "Cannot convert the empty string to a floating-point value!\n"); return false; }

		float prev = outValue;
		outValue = 0.0f;
		float dec = 10.0f;
		bool decimal = false;
		bool e = false;
		bool neg = false;
		int eVal = 0;

		for (int pos = 0; *(string + pos) && !IsWhiteSpace(*(string + pos)); ++pos)
		{
			if (*(string + pos) == '-') { neg = !neg; continue; }
			if (!e && (*(string + pos) == 'e' || *(string + pos) == 'E')) { e = true; if (neg) { outValue *= -1; } neg = false; continue; }
			if (*(string + pos) >= '0' && *(string + pos) <= '9')
			{
				if (e)
				{
					eVal *= 10;
					eVal += (int)((*(string + pos) - '0'));
					continue;
				}

				if (!decimal)
				{
					outValue *= 10;
					outValue += (float)((*(string + pos) - '0'));
				}
				else
				{
					outValue += ((float)((*(string + pos) - '0') / dec));
					dec *= 10.0f;
				}
			}
			else if (*(string + pos) == '.' && !e)
			{
				if (!decimal) { decimal = true; }
				else { GameLogger::Log(MessageType::Error, "Cannot parse [%s] to float! It contains more than one decimal point!\n", string); outValue = prev; return false; }
			}
			else { GameLogger::Log(MessageType::Error, "Cannot parse [%s] to float! character [%c] is not a valid digit!\n", string, *(string + pos)); outValue = prev; return false; }
		}

		if (e)
		{
			outValue *= (float)pow(10.0, neg ? -eVal : eVal);
		}
		else
		{
			outValue *= neg ? -1.0f : 1.0f;
		}

		return true;
	}

	bool StringFuncs::GetFloatsFromString(const char * const string, int numFloats, float * outValues)
	{
		if(!string) return false;
		if (*string == '\0') { GameLogger::Log(MessageType::Warning, "Cannot extract floating point values from the empty string!\n"); return false; }
		if (numFloats <= 0) { GameLogger::Log(MessageType::Warning, "Invalid value [%d] passed for numFloats in GetFloatsForString()!\n", numFloats); return false; }

		int outOffset = 0;
		for (int offset = 0; *(string + offset); ++offset)
		{
			if ((!offset || IsWhiteSpace(*(string + offset - 1))) && !IsWhiteSpace(*(string + offset)))
			{
				float value = 0.0f;
				if (!GetSingleFloatFromString(string + offset, value))
				{
					GameLogger::Log(MessageType::Warning, "GetFloatsForString [%s] failed, float #%d could not be parsed! Aborting!\n", string, outOffset);
					return false;
				}

				if (outOffset >= numFloats) { GameLogger::Log(MessageType::Warning, "GetFloatsForString expected to find [%d] floats but found extra! Aborting!\n", numFloats); return false; }

				*(outValues + outOffset) = value;
				outOffset++;
			}
		}

		if (outOffset < numFloats) { GameLogger::Log(MessageType::Warning, "GetFloatsForString expected to find [%d] floats but found [%d]! Aborting!\n", numFloats, outOffset); return false; }
		else { GameLogger::Log(MessageType::Info, "Found the correct number of floats!\n"); }

		return true;
	}

	bool StringFuncs::GetIntsFromString(const char * const string, int numInts, int * outValues)
	{
		if (!string) return false;
		if (*string == '\0') { GameLogger::Log(MessageType::Warning, "Cannot extract int values from the empty string!\n"); return false; }
		if (numInts <= 0) { GameLogger::Log(MessageType::Warning, "Invalid value [%d] passed for numInts in GetIntsForString()!\n", numInts); return false; }

		int outOffset = 0;
		for (int offset = 0; *(string + offset); ++offset)
		{
			if ((!offset || IsWhiteSpace(*(string + offset - 1))) && !IsWhiteSpace(*(string + offset)))
			{
				int value = 0;
				if (!GetSingleIntFromString(string + offset, value))
				{
					GameLogger::Log(MessageType::Warning, "GetIntsForString [%s] failed, int #%d could not be parsed! Aborting!\n", string, outOffset);
					return false;
				}

				if (outOffset >= numInts) { GameLogger::Log(MessageType::Warning, "GetIntsForKey expected to find [%d] ints but found extra! Aborting!\n", numInts); return false; }

				*(outValues + outOffset) = value;
				outOffset++;
			}
		}

		if (outOffset < numInts) { GameLogger::Log(MessageType::Warning, "GetIntsForKey expected to find [%d] ints but found [%d]! Aborting!\n", numInts, outOffset); return false; }
		else { GameLogger::Log(MessageType::Info, "Found the correct number of ints!\n"); }
		return true;
	}

	bool StringFuncs::GetSingleIntFromString(const char * const string, int & outValue)
	{
		if (!string) return false;
		if (*(string) == '\0') { GameLogger::Log(MessageType::Warning, "Cannot convert the empty string to an integer!\n"); return false; }

		int prev = outValue;
		outValue = 0;
		bool neg = false;

		for (int pos = 0; *(string + pos) && !IsWhiteSpace(*(string + pos)); ++pos)
		{
			if (*(string + pos) == '-') { neg = !neg; continue; }
			if (*(string + pos) >= '0' && *(string + pos) <= '9')
			{
				outValue *= 10;
				outValue += (int)((*(string + pos)) - '0');
			}
			else { GameLogger::Log(MessageType::Error, "Cannot parse [%s] to integer! character [%c] is not a valid digit!\n", string, *(string + pos)); outValue = prev; return false; }
		}

		if (neg)
		{
			outValue *= -1;
		}

		return true;
	}

	bool StringFuncs::IsWhiteSpace(char c)
	{
		return c == ' ' || c == '\t' || c == ' ';
	}

	bool StringFuncs::IsDigit(char c)
	{
		return (c >= '0') && (c <= '9');
	}

	bool StringFuncs::CountUp(char * numBuffer, unsigned bufferSize)
	{
		numBuffer[bufferSize - 1]++;
		for (int i = bufferSize - 1; i >= 0; --i)
		{
			if (numBuffer[i] > '9')
			{
				numBuffer[i] = '0';
				if (i == 0) { GameLogger::Log(MessageType::cWarning, "Tried to CountUp but ran out of space!\n"); return false; }
				numBuffer[i - 1]++;
			}
		}

		return true;
	}

	bool StringFuncs::CountDown(char * numBuffer, unsigned bufferSize)
	{
		numBuffer[bufferSize - 1]--;
		for (int i = bufferSize - 1; i >= 0; --i)
		{
			if (numBuffer[i] < '0')
			{
				numBuffer[i] = '9';
				if (i == 0) { GameLogger::Log(MessageType::cWarning, "Tried to CountDown but ran out of space!\n"); return false; }
				numBuffer[i - 1]--;
			}
		}
		return true;
	}

	const char * StringFuncs::GetPtrToInCharArray(const char * const strToLookFor)
	{
		// find the string
		int index = FindSubString(&s_buffer[0], strToLookFor);
		
		// if its in there return the address
		if (index >= 0 && index < PLENTY) { return &s_buffer[index]; }

		// if not in there return nullptr
		return nullptr;
	}

	const char *StringFuncs::AddToCharArray(const char * const str)
	{
		const char *check = GetPtrToInCharArray(str);
		if (check) { return check; }

		int remaining = PLENTY - s_nextBufferSlot;
		if (remaining < StringLen(str)) { GameLogger::Log(MessageType::cError, "Failed to AddToCharArray! Not enough space for [%d] chars in buffer of size [%d]!\n", StringLen(str), PLENTY); return nullptr; }

		int pos = StringConcatIntoBuffer("", str, "", &s_buffer[s_nextBufferSlot], remaining);

		char * spot = &s_buffer[s_nextBufferSlot];

		s_nextBufferSlot = s_nextBufferSlot + pos + 1; // TODO CHECK +1!?!?1

		return spot; 
	}
}