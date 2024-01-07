#pragma once
#include <Windows.h>
#include <stdio.h>

void formatSize(wchar_t* outputBuffer, long long llSize);
void formatRam(wchar_t* outputBuffer, size_t ullSize);
HFONT createFont(wchar_t* fontName, unsigned int width, unsigned int height);
void getSelfDirectory(wchar_t* directory);
