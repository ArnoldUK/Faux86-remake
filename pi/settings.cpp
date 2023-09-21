
#include <circle/logger.h>

#include "CircleHostInterface.h"
#include "settings.h"

using namespace Faux86;

#ifdef USE_FATFS
Settings::Settings(FATFS *pFileSystem, const char *pFileName)
: m_Properties(pFileName, pFileSystem),
	m_filePath(pFileName)
{
}
#else
Settings::Settings(CFATFileSystem *pFileSystem, const char *pFileName)
: m_Properties(pFileName, pFileSystem),
	m_filePath(pFileName)
{
}
#endif

Settings::~Settings()
{
}


bool Settings::Load()
{
	if (!m_Properties.Load())
	{
		//CLogger::Get()->Write("[SETTINGS]", LogError, "Error loading properties from %s (line %u)", m_filePath, m_Properties.GetErrorLine());
		log(Log, "[SETTINGS] Error loading properties from %s (line %u)", m_filePath, m_Properties.GetErrorLine());
		return false;
	}
	//m_Properties.GetString("logdev", "tty1"));
	//m_Properties.GetNumber("loglevel", 3));
	return true;
}

bool Settings::Save()
{
	if (!m_Properties.Save())
	{
		//CLogger::Get()->Write("[SETTINGS]", LogError, "Error saving properties from %s (line %u)", m_filePath, m_Properties.GetErrorLine());
		log(Log, "[SETTINGS] Error saving properties from %s (line %u)", m_filePath, m_Properties.GetErrorLine());
		return false;
	}
	
	//m_Properties.SetString("logdev", "tty1");
	//m_Properties.SetNumber("loglevel", 3);
	return true;
}

void Settings::Dump()
{
	m_Properties.Dump();
}

const char* Settings::GetString(const char *pName, const char *pDefault) const
{
	return m_Properties.GetString(pName, pDefault);
}

unsigned Settings::GetNumber(const char *pName, unsigned nDefault) const
{
	return m_Properties.GetNumber(pName, nDefault);
}

int Settings::GetInt(const char *pName, int nDefault) const
{
	return m_Properties.GetSignedNumber(pName, nDefault);
}

