#include "iostream"
#include "sstream"
#include "fstream"
#include "algorithm"

#include "SettingFileReader.h"

SettingsReader::SettingsReader()
{
	data.clear();
}

SettingsReader::~SettingsReader()
{
	data.clear();
}

void SettingsReader::removeSpace(std::string& szStr)
{
	int ii=0, kk=0;
	std::string input = szStr;
	char szTempStr[500];
	int length = input.length();
	
	for(ii = 0; ii < length; ii++)
	{
		if(input[ii] == ' ') continue;
		szTempStr[kk++] = input[ii];
	}
	szTempStr[kk] = '\0';
	szStr.clear();
	szStr = std::string(szTempStr);
	return;
}

int SettingsReader::ReadFile(std::string szFileName)
{
	std::fstream file;
	file.open(szFileName.c_str(), std::ios::in);
	std::string szLine, szField, szValue;
	std::string szPresentszSection("default");
	int marker, length;

	if(file.good())
	{
		while(std::getline(file, szLine))
		{
			if(szLine.length() == 0)
			{
				continue;
			}
			if(szLine[0] == ';') continue;
			if(szLine[0] == '#') continue;

			if(szLine[0] == '[')
			{
				// szSection begins
				length = szLine.length();
				// to remove [ and ]
				szPresentszSection = szLine.substr(1, length-2);
				std::transform(szPresentszSection.begin(), szPresentszSection.end(), szPresentszSection.begin(), ::tolower);
				continue;
			}
			marker = szLine.find("=");
			szField = szLine.substr(0, marker);
			removeSpace(szField);
			std::transform(szField.begin(), szField.end(), szField.begin(), ::tolower);		
			szValue = szLine.substr(marker+1, szLine.length()-2);
			removeSpace(szValue);
			// dont change case of value
			//std::transform(szValue.begin(), szValue.end(), szValue.begin(), ::tolower);		

			data[szPresentszSection][szField] = szValue;

			std::cout << "szSection|" << szPresentszSection << "|Field|" << szField << "|Value|" << szValue << "|"<< std::endl;
		}
	}
	else
	{
		std::cout << "File " << szFileName << " read error" << std::endl;
		exit(1);
	}

	return 0;
}

bool SettingsReader::GetBool(std::string szSection, std::string szFieldName, bool defaultValue)
{
	std::transform(szSection.begin(), szSection.end(), szSection.begin(), ::tolower);
	std::transform(szFieldName.begin(), szFieldName.end(), szFieldName.begin(), ::tolower);
	section = data.find(szSection);
	if(section != data.end())
	{
		content = section->second.find(szFieldName);
		if(content != section->second.end())
		{
			std::string szValue = content->second;
			std::transform(szValue.begin(), szValue.end(), szValue.begin(), ::tolower);
			if(!szValue.compare(std::string("true")))
			{
				return true;
			}
			if(!szValue.compare(std::string("false")))
			{
				return false;
			}
			return defaultValue;
		}
		return defaultValue;
	}
	else
	{
		return defaultValue;
	}
	return defaultValue;
}

int SettingsReader::GetInt(std::string szSection, std::string szFieldName, int defaultValue)
{
	std::transform(szSection.begin(), szSection.end(), szSection.begin(), ::tolower);
	std::transform(szFieldName.begin(), szFieldName.end(), szFieldName.begin(), ::tolower);

	section = data.find(szSection);
	if(section != data.end())
	{
		content = section->second.find(szFieldName);
		if(content != section->second.end())
		{
			return atoi(content->second.c_str());
		}
		return defaultValue;
	}
	else
	{
		return defaultValue;
	}
	return defaultValue;
}

long SettingsReader::GetLong(std::string szSection, std::string szFieldName, long defaultValue)
{
	std::transform(szSection.begin(), szSection.end(), szSection.begin(), ::tolower);
	std::transform(szFieldName.begin(), szFieldName.end(), szFieldName.begin(), ::tolower);

	section = data.find(szSection);
	if(section != data.end())
	{
		content = section->second.find(szFieldName);
		if(content != section->second.end())
		{
			return atol(content->second.c_str());
		}
		return defaultValue;
	}
	else
	{
		return defaultValue;
	}
	return defaultValue;
}

double SettingsReader::GetDouble(std::string szSection, std::string szFieldName, double defaultValue)
{
	std::transform(szSection.begin(), szSection.end(), szSection.begin(), ::tolower);
	std::transform(szFieldName.begin(), szFieldName.end(), szFieldName.begin(), ::tolower);

	section = data.find(szSection);
	if(section != data.end())
	{
		content = section->second.find(szFieldName);
		if(content != section->second.end())
		{
			return atof(content->second.c_str());
		}
		return defaultValue;
	}
	else
	{
		return defaultValue;
	}
	return defaultValue;
}

std::string SettingsReader::GetString(std::string szSection, std::string szFieldName, std::string defaultValue)
{
	std::transform(szSection.begin(), szSection.end(), szSection.begin(), ::tolower);
	std::transform(szFieldName.begin(), szFieldName.end(), szFieldName.begin(), ::tolower);

	section = data.find(szSection);
	if(section != data.end())
	{
		content = section->second.find(szFieldName);
		if(content != section->second.end())
		{
			return (content->second);
		}
		return defaultValue;
	}
	else
	{
		return defaultValue;
	}
	return defaultValue;
}
