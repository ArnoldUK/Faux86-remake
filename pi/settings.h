#ifndef _INCLUDE_SETTINGS_H_
#define _INCLUDE_SETTINGS_H_

#include <circle/string.h>
#include <circle/util.h>

#ifdef USE_FATFS
#include <fatfs/ff.h> //Generic FAT Filesystem Module (by ChaN)
#include <Properties/propertiesfatfsfile.h>
#else
#include <circle/fs/fat/fatfs.h> //CFatFileSystem Native filesystem with FAT16 and FAT32 support using C++ classes
#include <Properties/propertiesfile.h>
#endif

/*
defaultconfig = {
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
	{ "speed", "10" },
	{ "delay", "120" },
	{ "slowsys", "1" },
	{ "multithreaded", "0" },
	{ "resw", "640" },
	{ "resh", "400" },
	{ "render", "1" },
	{ "monitor", "0" },
	{ "mouseport", "2" },
	{ "snddisney", "0" },
	{ "sndblaster", "0" },
	{ "sndadlib", "1" },
	{ "sndspeaker", "1" },
	{ "latency", "200" },
	{ "samprate", "32000" },
	{ "console", "1" },
	{ "menu", "1" }
};
*/

namespace Faux86
{
	class Settings
	{
	public:
		#ifdef USE_FATFS
		Settings(FATFS *pFileSystem, const char *pFileName);
		#else
		Settings(CFATFileSystem *pFileSystem, const char *pFileName);
		#endif
		~Settings();

		bool Load();
		bool Save();
		void Dump();
		
		const char *GetString(const char *pName, const char *pDefault = 0) const;
		unsigned GetNumber(const char *pName, unsigned nDefault = 0) const;
		int GetInt(const char *pName, int nDefault = 0) const;
		
		/*
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
		*/

	private:
		#ifdef USE_FATFS
		CPropertiesFatFsFile	m_Properties;
		#else
		PropertiesFile 				m_Properties;
		#endif
		
		CString		m_filePath;
	};
}

#endif //_INCLUDE_SETTINGS_H_
