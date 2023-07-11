
#include "settings.h"

Settings::Settings(string filePath)
: m_filePath(filePath)
{
}

#ifdef USE_UNORDERED_MAP
Settings::Settings(string filePath, string header, unordered_map<string, string> defaultconfig)
: m_filePath(filePath),
	m_header(header),
	m_defaultconfig(defaultconfig)
{
}
#else
Settings::Settings(string filePath, string header, map<string, string> defaultconfig)
: m_filePath(filePath),
	m_header(header),
	m_defaultconfig(defaultconfig)
{
}
#endif

Settings::~Settings()
{
}

void Settings::loadDefault()
{
	m_data = m_defaultconfig;
}

void Settings::load()
{
	std::ifstream   fileStream(m_filePath);
	string          line;

	//this->loadDefault();

	// Create the file with default configuration if it's not there
	if (!fileStream.is_open()) {
		this->loadDefault();
		this->save();
	}	else {
		// Read file line by line
		while(std::getline(fileStream, line))
		{
			string	key;
			string	value;
			bool	keyFound     = false;
			bool	readingValue = false;

			// Read character by character
			for(char x : line)
			{
				// Comment
				if(	x == '#' || x == '/' || x == ';')
					break;

				// Ignore whitespace if we are reading the key
				else if((x == ' ' || x == '\t') && !readingValue)
					continue;

				else if(x == '=')
					keyFound = true;

				else if (!keyFound)
					key.push_back(x);

				else if (keyFound)
				{
					value.push_back(x);
					readingValue = true;
				}
			}

			// Insert {key,value} in the map
			if(keyFound)
				this->set(key, value);
		}
	}
	fileStream.close();
}

void Settings::save() const
{
	std::fstream fileStream(m_filePath, std::fstream::out);

	fileStream << m_header  << std::endl;
	fileStream << std::endl;

	for(auto x : m_data)
		fileStream << x.first << "=" << x.second << std::endl;

	fileStream.close();
}

void Settings::print() const
{
	for(auto x : m_data)
		std::cout << x.first << "=" << x.second << std::endl;
}

bool Settings::findKey(const string& key) const
{
	if (m_data.find(key) != m_data.end()) return true;
	return false;
}

bool Settings::findKeyUpper(const string& key) const
{
	string _key = key;
	transform(_key.begin(), _key.end(), _key.begin(), ::toupper);
	if (m_data.find(_key) != m_data.end()) return true;
	return false;
}

bool Settings::findKeyLower(const string& key) const
{
	string _key = key;
	transform(_key.begin(), _key.end(), _key.begin(), ::tolower);
	if (m_data.find(_key) != m_data.end()) return true;
	return false;
}

void Settings::set(const string& key, const string& value)
{
	// Check if already in the map
	if (this->findKey(key))
		m_data.at(key) = value;
	else
		m_data.insert({key, value});
}

void Settings::set(const string& key, char value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, unsigned char value)
{
	this->set(key, std::to_string(value));
}


void Settings::set(const string& key, bool value)
{
	this->set(key, string((value) ? "true" : "false"));
}

void Settings::set(const string& key, float value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, int value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, unsigned value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, long value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, double value)
{
	this->set(key, std::to_string(value));
}

void Settings::set(const string& key, const char* value)
{
	this->set(key, string(value));
}

string Settings::get(const string& key) const
{
	if (this->findKey(key))
		return m_data.at(key);
	else
		return "";
}

char Settings::getChar(const string& key) const
{
	if (this->findKey(key)) {
		string str = m_data.at(key);
		return char(str.at(0));
	}	else
		return char(0);
}

unsigned char Settings::getByte(const string& key) const
{
	if (this->findKey(key)) {
		string str = m_data.at(key);
		return (unsigned char)(atoi(str.c_str()));
	}	else
		return (unsigned char)(0);
}


bool Settings::getBool(const string& key) const
{
	if (this->findKey(key))
	{
		string value = m_data.at(key);
		std::transform(value.begin(), value.end(), value.begin(), tolower);

		if( value == "true" || value == "1" || value == "on" 	|| value == "yes") return true;
	}
	return false;
}

float Settings::getFloat(const string& key) const
{
	if (this->findKey(key))
		return std::atof(m_data.at(key).c_str());
	else
		return 0.f;
}

int Settings::getInt(const string& key) const
{
	if (this->findKey(key))
		return std::atoi(m_data.at(key).c_str());
	else
		return 0;
}

unsigned Settings::getUInt(const string& key) const
{
	if (this->findKey(key))
		return std::atoi(m_data.at(key).c_str());
	else
		return 0;
}

long Settings::getLong(const string& key) const
{
	if (this->findKey(key))
		return std::atol(m_data.at(key).c_str());
	else
		return 0;
}

double Settings::getDouble(const string& key) const
{
	if (this->findKey(key))
		return std::stod(m_data.at(key).c_str());
	else
		return 0;
}
