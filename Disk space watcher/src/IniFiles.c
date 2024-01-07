#include "IniFiles.h"

int IniSetInt(wchar_t* filePath, wchar_t* sectionName, wchar_t* keyName, int newValue)
{
	wchar_t buf[64];
	swprintf_s(buf, 64, L"%d", newValue);
	return WritePrivateProfileString(sectionName, keyName, buf, filePath);
}

int IniGetInt(wchar_t* filePath, wchar_t* sectionName, wchar_t* keyName, int defaultValue)
{
	return GetPrivateProfileInt(sectionName, keyName, defaultValue, filePath);
}
