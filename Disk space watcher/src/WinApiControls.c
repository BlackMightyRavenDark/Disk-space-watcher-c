#include "WinApiControls.h"

HWND createLabel(wchar_t* text, int x, int y, int w, int h, HWND parent)
{
	return CreateWindow(L"Static", text, WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
		x, y, w, h, parent, NULL, NULL, NULL);
}

HWND createListBox(wchar_t* text, int x, int y, int w, int h, short id, HWND parent)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, L"ListBox", text, WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
		x, y, w, h, parent, (HMENU)id, NULL, NULL);
}

int listBox_AddString(HWND handle, wchar_t* stringToAdd)
{
	return (int)SendMessage(handle, LB_ADDSTRING, 0, (LPARAM)stringToAdd);
}

LRESULT listBox_Clear(HWND handle)
{
	return SendMessage(handle, LB_RESETCONTENT, 0, 0);
}

int listBox_GetItemCount(HWND handle)
{
	return (int)SendMessage(handle, LB_GETCOUNT, 0, 0);
}

int listBox_GetItemHeight(HWND handle)
{
	return (int)SendMessage(handle, LB_GETITEMHEIGHT, 0, 0);
}

int listBox_GetSelectedItemId(HWND handle)
{
	return (int)SendMessage(handle, LB_GETCURSEL, 0, 0);
}

LRESULT listBox_SetSelectedItemId(HWND handle, int id)
{
	return SendMessage(handle, LB_SETCURSEL, id, 0);
}
