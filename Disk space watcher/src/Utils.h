#pragma once
#include <Windows.h>
#include <stdio.h>
#include "WinApiControls.h"

void formatSize(wchar_t* outputBuffer, long long llSize);
void formatRam(wchar_t* outputBuffer, size_t ullSize);
HFONT createFont(wchar_t* fontName, unsigned int width, unsigned int height);
void getSelfDirectory(wchar_t* directory);
int setIsWindowStayingOnTop(HWND windowHandle, int stayOnTop);
int checkMenuItem(HMENU hMenu, UINT itemId, UINT checked);
int getDiskSpace(wchar_t* diskRoot, UINT64* totalSpace, UINT64* usedSpace, UINT64* freeSpace);
int openDiskFromListBox(HWND listBoxHandle, int itemId);
