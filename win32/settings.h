#ifndef _INCLUDE_SETTINGS_H_
#define _INCLUDE_SETTINGS_H_

//UNCOMMENT TO USE UNSORTED MAP FILE
//#define USE_UNORDERED_MAP 1

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
//#include <vector>

#ifdef USE_UNORDERED_MAP
#include <unordered_map>
using std::unordered_map;
#else
#include <map>
using std::map;
#endif

using std::string;
//using std::vector;

#ifdef USE_UNORDERED_MAP
static std::unordered_map<std::string, std::string> defaultconfig = {
#else
static std::map<std::string, std::string> defaultconfig = {
#endif
	{ "fd0", "fd0.img" },
	{ "fd1", "" },
	{ "hd0", "hd0.img" },
	{ "hd1", "" },
	{ "boot", "hd0" },
	{ "biosrom", "pcxtbios.bin" },
	{ "videorom", "videorom.bin" },
	{ "bootrom", "rombasic.bin" },
	{ "charrom", "asciivga.dat" },
	{ "netid", "0" },
	{ "nosound", "0" },
	{ "fullscreen", "0" },
	{ "verbose", "1" },
	{ "speed", "20" },
	{ "delay", "20" },
	{ "slowsys", "0" },
	{ "multithreaded", "0" },
	{ "resw", "640" },
	{ "resh", "350" },
	{ "render", "1" },
	{ "monitor", "0" },
	{ "mouseport", "2" },
	{ "snddisney", "0" },
	{ "sndblaster", "1" },
	{ "sndadlib", "1" },
	{ "sndspeaker", "1" },
	{ "latency", "100" },
	{ "samprate", "48000" },
	{ "console", "1" },
	{ "menu", "1" }
};

class Settings
{
public:
	Settings(string filePath);
	#ifdef USE_UNORDERED_MAP
	Settings(string filePath, string header, unordered_map<string, string> defaultconfig);
	#else
	Settings(string filePath, string header, map<string, string> defaultconfig);
	#endif
	~Settings();

	void	load();
	void	loadDefault();
	void	save() const;
	void	print() const;

	void	set(const string& key, const string& value);
	void	set(const string& key, char value);
	void	set(const string& key, unsigned char value);
	void	set(const string& key, bool value);
	void	set(const string& key, float value);
	void	set(const string& key, int value);
	void	set(const string& key, unsigned value);
	void	set(const string& key, long value);
	void	set(const string& key, double value);
	void	set(const string& key, const char* value);

	string				get(const string& key) const;
	char					getChar(const string& key) const;
	unsigned char	getByte(const string& key) const;
	bool					getBool(const string& key) const;
	float					getFloat(const string& key) const;
	int						getInt(const string& key) const;
	unsigned			getUInt(const string& key) const;
	long					getLong(const string& key) const;
	double				getDouble(const string& key) const;
	

private:
	bool			findKey(const string& key) const;
	bool			findKeyUpper(const string& key) const;
	bool			findKeyLower(const string& key) const;
	
	#ifdef USE_UNORDERED_MAP
	unordered_map<string,string>	m_data;
	unordered_map<string,string>	m_defaultconfig;
	#else
	map<string,string>	m_data;
	map<string,string>	m_defaultconfig;
	#endif
	
	string							m_filePath;
	string							m_header;
};

#endif //_INCLUDE_SETTINGS_H_
