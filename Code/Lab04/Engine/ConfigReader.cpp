#include "ConfigReader.h"
#include "GameLogger.h"
#include "StringFuncs.h"

// Justin Furtado
// 5/5/2016
// ConfigReader.h
// Implementations for class that reads in a configuration file and stores the key-value pairs it finds there

namespace Engine
{
	int ConfigReader::count = 0;
	int ConfigReader::delcount = 0;
	ConfigReader *ConfigReader::pReader = nullptr;

	ConfigReader::ConfigReader()
	{
		m_id = ++count;
		GameLogger::Log(MessageType::Info, "CongfigReader #%d Created with empty KeyValuePairs!\n", m_id);
	}

	ConfigReader::~ConfigReader()
	{
		GameLogger::Log(MessageType::Info, "CongfigReader #%d Destroyed, %d remain!\n", m_id, (count - (++delcount)));
	}

	bool ConfigReader::Initialize(const char *const configFileName)
	{
		if (!ProcessConfigFile(configFileName))
		{
			GameLogger::Log(MessageType::cFatal_Error, "Failed to initialize config reader #%d, could not process config file [%s]!\n", m_id, configFileName);
			return false;
		}

		GameLogger::Log(MessageType::Process, "ConfigReader #%d Initialized Successfully!\n", m_id);
		return true;
	}

	bool ConfigReader::ProcessConfigFile(const char * const configFileName)
	{
		this->fileName = configFileName;

		// reset values in case of re-reading file
		m_keyValuePairs = KeyValuePairs();

		if (pReader == nullptr) { pReader = this; }

		if (!OpenFile()) return false;
		if (!ReadFile()) return false;

		// notify things that should be notifed of config value changes
		CallCallbacks();

		GameLogger::Log(MessageType::Process, "ConfigReader successfully processed file [%s]!\n", configFileName);
		return true;
	}

	bool ConfigReader::ProcessConfigFile()
	{
		return ProcessConfigFile(fileName);
	}

	bool ConfigReader::OpenFile()
	{
		m_configStream.open(fileName);

		if (!m_configStream.is_open())
		{
			GameLogger::Log(MessageType::cFatal_Error, "ConfigReader #%d: Unable to open file [%s] for reading!\n", m_id, fileName);
			return false;
		}

		m_configStream.clear();
		m_configStream.seekg(0, std::ios::beg);

		return true;
	}

	bool ConfigReader::ReadFile()
	{
		int line = 0;
		// TODO: Config reader breaks when lines longer than MAX_LINE are present... investigate this!
		const int MAX_LINE = 250;
		char cLine[MAX_LINE];
		while (!m_configStream.eof())
		{
			line++;
			m_configStream.getline(cLine, MAX_LINE);
			if (!ReadLine(cLine))
			{
				GameLogger::Log(MessageType::cFatal_Error, "ConfigReader #%d: Unable to read line %d of file!\n", m_id, line);
				return false;
			}
		}

		return true;
	}

	bool ConfigReader::ReadLine(const char *const line)
	{
		if (!line) return false;

		static int lineNum = 0;

		const int MAX_CHARS = 256;
		char key[MAX_CHARS];
		char value[MAX_CHARS];
		char debug[MAX_CHARS];

		lineNum++;

		// could not read first word, line was comment or whitespace only
		if (!ReadWord(line, 1, key, MAX_CHARS)) { return true; }
		if (!ReadWord(line, 2, value, MAX_CHARS))
		{
			GameLogger::Log(MessageType::Error, "No value/bad value found for Key [%s] on Line #%d, Skipping!\n", key, lineNum);
			return true;
		}

		if (ReadWord(line, 3, debug, MAX_CHARS))
		{
			GameLogger::Log(MessageType::Error, "Extra text found on line #%d, Skipping!\n", lineNum);
			return true;
		}

		AddUnique(key, value);
		return true;
	}

	bool ConfigReader::ReadWord(const char *const line, int wordNum, char *const buffer, int bufferSize)
	{
		int wordC = 0;
		int charC = 0;
		bool whiteSpace = true;
		int quotes = 0;
		for (int pos = 0; *(line + pos); ++pos)
		{
			if (*(line + pos) == '/' && *(line + pos + 1) == '/') break; // we're done - line over, return whether or not the indicated word was read
			if (*(line + pos) == '"' && *(line + pos + 1) == '"') { wordC++; }
			if (*(line + pos) == '"') { quotes++; continue; }

			if (!StringFuncs::IsWhiteSpace(*(line + pos)) || quotes % 2 == 1)
			{
				// last char was whitespace or beginning of line and not a comment, this is the beginning of a word
				if (whiteSpace) { wordC++; }

				// if the number of the current word we are reading equals the number of the word we are checking for
				if (wordC == wordNum)
				{
					if (charC >= bufferSize - 1)
					{
						GameLogger::Log(MessageType::Warning, "Tried to read more than %d characters into buffer of size %d\n", bufferSize, bufferSize);
						break;
					}

					*(buffer + charC) = *(line + pos);
					charC++;

				}

				whiteSpace = false;
			}
			else
			{
				if (wordC == wordNum) { break; }
				whiteSpace = true;
				continue;
			}

		}

		if (quotes % 2 != 0)
		{
			GameLogger::Log(MessageType::Error, "Unmatched quotes found on line [%s]!\n", line);
			*(buffer + charC) = '\0';
			return false;
		}

		*(buffer + charC) = '\0';

		// end of line, return whether or not a word was read
		return wordC >= wordNum;

	}

	void ConfigReader::CallCallbacks()
	{
		// loop through all of the callbacks
		for (int i = 0; i < MAX_CALLBACKS; ++i)
		{
			// if it is not null
			if (m_callbacks[i])
			{
				// call it
				m_callbacks[i](m_pCallbackInstances[i]);
			}
		}
	}

	bool ConfigReader::RegisterCallbackForConfigChanges(ConfigReaderCallback callbackToRegister, void *callbackClassInstance)
	{
		// validate input
		if (!callbackToRegister) { GameLogger::Log(MessageType::cWarning, "Tried to register nullptr as callback for config reader!\n"); return false; }

		// loop through callbacks
		for (int i = 0; i < MAX_CALLBACKS; ++i)
		{
			// if slot is not in use
			if (!m_callbacks[i])
			{
				// set it up
				m_callbacks[i] = callbackToRegister;
				m_pCallbackInstances[i] = callbackClassInstance;

				// indicate success
				GameLogger::Log(MessageType::Process, "Config Reader successfully registered a callback at index [%d]!\n", i);
				return true;
			}
		}

		// indicate no availiable space
		GameLogger::Log(MessageType::cError, "Tried to register a callback with config reader but no space was availiable!\n");
		return false;
	}

	bool ConfigReader::UnregisterCallbackForConfigChanges(ConfigReaderCallback callbackToUnregister)
	{
		// validate input
		if (!callbackToUnregister) { GameLogger::Log(MessageType::cWarning, "Tried to unregister nullptr as callback for config reader!\n"); return false; }

		// loop through callbacks
		for (int i = 0; i < MAX_CALLBACKS; ++i)
		{
			// if callback matches
			if (m_callbacks[i] == callbackToUnregister)
			{
				// clear space 
				m_callbacks[i] = nullptr;
				m_pCallbackInstances[i] = nullptr;

				// indicate success
				GameLogger::Log(MessageType::Process, "Config Reader successfully unregistered a callback at index [%d]!\n", i);
				return true;
			}
		}

		// indicate not found
		GameLogger::Log(MessageType::cError, "Tried to unregister a callback with config reader it was not found!\n");
		return false;
	}

	bool ConfigReader::ShutDown()
	{
		m_configStream.close();

		if (m_configStream.is_open())
		{
			GameLogger::Log(MessageType::Error, "ConfigReader #%d failed to Shut Down, could not close configStream!\n", m_id);
			return false;
		}

		GameLogger::Log(MessageType::Process, "ConfigReader #%d Shut Down Successfully!\n", m_id);
		return true;
	}

	bool ConfigReader::IsFull() const
	{
		return m_keyValuePairs.IsFull();
	}

	bool ConfigReader::ContainsKey(const char *const key) const
	{
		return m_keyValuePairs.ContainsKey(key);
	}

	bool ConfigReader::ContainsRequiredKey(const char *const key) const
	{
		bool present = m_keyValuePairs.ContainsKey(key);

		if (!present)
		{
			GameLogger::Log(MessageType::cFatal_Error, "Could not find required Key [%s]! Exploding!\n", key);
		}

		return present;
	}
	bool ConfigReader::AddUnique(const char *const key, const char *const value)
	{
		if (IsFull())
		{
			GameLogger::Log(MessageType::cFatal_Error, "Too many keys found, not enough room for more!\n");
			return false;
		}

		return m_keyValuePairs.AddUnique(key, value);
	}

	bool ConfigReader::GetStringForKey(const char *const key, char *const& outValue) const
	{
		const char *str = m_keyValuePairs.GetValueForKey(key);
		if (!str) return false;

		StringFuncs::StringCopy(str, outValue, 256); // TODO: check magic number
		return true;
	}

	bool ConfigReader::GetIntForKey(const char *const key, int& outValue) const
	{
		const char *const str = m_keyValuePairs.GetValueForKey(key);

		int out = outValue;
		if (!StringFuncs::GetSingleIntFromString(str, out))
		{
			GameLogger::Log(MessageType::Warning, "Could not get int for key [%s], the value [%s] could not be parsed to an integer!\n", key, str);
			return false;
		}

		// only modify out value in the case of success
		outValue = out;
		return true;
	}

	bool ConfigReader::GetFloatForKey(const char *const key, float& outValue) const
	{
		const char *const str = m_keyValuePairs.GetValueForKey(key);

		float out = outValue;
		if (!StringFuncs::GetSingleFloatFromString(str, out))
		{
			GameLogger::Log(MessageType::Warning, "Could not get float for key [%s], the value, [%s] could not be parsed to a float!\n", key, str);
			return false;
		}

		//only modify out value in the case of success
		outValue = out;
		return true;
	}

	bool ConfigReader::GetBoolForKey(const char *const key, bool& outValue) const
	{
		const char *const str = m_keyValuePairs.GetValueForKey(key);
		if (!str) return false;

		if (StringFuncs::StringsAreEqual(str, "true")) { outValue = true; return true; }
		else if (StringFuncs::StringsAreEqual(str, "false")) { outValue = false; return true; }
		else { GameLogger::Log(MessageType::Error, "Cannot convert key [%s] to bool!\n", str); return false; }
	}

	bool ConfigReader::GetClampedIntForKey(const char * const key, int & outValue, int minValue, int maxValue) const
	{
		int value = outValue;
		if (!GetIntForKey(key, value))
		{
			GameLogger::Log(MessageType::cWarning, "Unable to get clamped int for key [%s], failed to get value before clamping!\n", key);
			return false;
		}

		// clamp value to range specified
		value = (value < minValue) ? minValue : value;
		value = (value > maxValue) ? maxValue : value;

		outValue = value;
		return true; // success
	}

	bool ConfigReader::GetClampedFloatForKey(const char * const key, float & outValue, float minValue, float maxValue) const
	{
		float value = outValue;
		if (!GetFloatForKey(key, value))
		{
			GameLogger::Log(MessageType::cWarning, "Unable to get clamped float for key [%s], failed to get value before clamping!\n", key);
			return false;
		}

		// clamp value to range specified
		value = (value < minValue) ? minValue : value;
		value = (value > maxValue) ? maxValue : value;

		outValue = value;
		return true; // success
	}

	bool ConfigReader::GetClampedFloatsForKey(const char *const key, int numFloats, float *values, float minValue, float maxValue) const
	{
		if (!GetFloatsForKey(key, numFloats, values))
		{
			GameLogger::Log(MessageType::Warning, "GetClampedFloatsForKey failed to GetFloatsForKey!\n");
			return false;
		}

		// clamps values to range specified
		for (int i = 0; i < numFloats; ++i)
		{
			*(values + i) = (*(values + i) < minValue) ? minValue : *(values + i);
			*(values + i) = (*(values + i) > maxValue) ? maxValue : *(values + i);
		}

		return true; // success
	}

	bool ConfigReader::GetClampedIntsForKey(const char *const key, int numInts, int *values, int minValue, int maxValue) const
	{
		if (!GetIntsForKey(key, numInts, values))
		{
			GameLogger::Log(MessageType::Warning, "GetClampedIntsForKey failed to GetIntsForKey!\n");
			return false;
		}

		// clamps values to range specified
		for (int i = 0; i < numInts; ++i)
		{
			*(values + i) = (*(values + i) < minValue) ? minValue : *(values + i);
			*(values + i) = (*(values + i) > maxValue) ? maxValue : *(values + i);
		}

		return true; // success
	}

	bool ConfigReader::GetFloatsForKey(const char *const key, int numFloats, float *values) const
	{
		const char *const str = m_keyValuePairs.GetValueForKey(key);
		return StringFuncs::GetFloatsFromString(str, numFloats, values);
	}

	bool ConfigReader::GetIntsForKey(const char *const key, int numInts, int *values) const
	{
		const char *const str = m_keyValuePairs.GetValueForKey(key);
		return StringFuncs::GetIntsFromString(str, numInts, values);
	}
}