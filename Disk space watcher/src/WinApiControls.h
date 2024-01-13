#pragma once
#include <Windows.h>

HWND createLabel(wchar_t* text, int x, int y, int w, int h, HWND parent);
HWND createListBox(wchar_t* text, int x, int y, int w, int h, short id, HWND parent);
int listBox_AddString(HWND handle, wchar_t* stringToAdd);
LRESULT listBox_Clear(HWND handle);
int listBox_GetItemCount(HWND handle);
int listBox_GetItemHeight(HWND handle);
int listBox_GetSelectedItemId(HWND handle);
LRESULT listBox_SetSelectedItemId(HWND handle, int id);
int listBox_GetStringLength(HWND handle, int id);
int listBox_GetString(HWND handle, int id, wchar_t* buffer);
