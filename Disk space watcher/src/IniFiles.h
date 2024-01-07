#pragma once
#include <Windows.h>
#include <stdio.h>

int IniSetInt(wchar_t* filePath, wchar_t* sectionName, wchar_t* keyName, int newValue);
int IniGetInt(wchar_t* filePath, wchar_t* sectionName, wchar_t* keyName, int defaultValue);
