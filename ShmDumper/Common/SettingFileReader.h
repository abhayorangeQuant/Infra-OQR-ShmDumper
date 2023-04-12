#ifndef _SETTING_FILE_READER_HEADER
#define _SETTING_FILE_READER_HEADER

#include "iostream"
#include "string.h"
#include "map"

typedef std::map<std::string, std::string> ContentMap;
typedef std::map<std::string, std::string>::iterator ContentMapIter;
typedef std::map<std::string, ContentMap> SectionMap;
typedef std::map<std::string, ContentMap>::iterator SectionMapIter;

class SettingsReader
{
private:
	SectionMap data;
	ContentMapIter content;
	SectionMapIter section;

	void removeSpace(std::string& );
public:
	SettingsReader();
	~SettingsReader();

	int ReadFile(std::string szFileName);
	bool GetBool(std::string szSection, std::string szFieldName, bool defaultValue);
	int GetInt(std::string szSection, std::string szFieldName, int defaultValue);
	long GetLong(std::string szSection, std::string szFieldName, long defaultValue);
	double GetDouble(std::string szSection, std::string szFieldName, double defaultValue);
	std::string GetString(std::string szSection, std::string szFieldName, std::string defaultValue);
};

#endif
