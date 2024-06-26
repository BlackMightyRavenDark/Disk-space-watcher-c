#include <Windows.h>
#include <stdio.h>
#include "IniFiles.h"
#include "Utils.h"
#include "WinApiControls.h"

#define TIMER_ID_UPDATE_DRIVES 0
#define TIMER_ID_UPDATE_RAM 1

#define ID_LISTBOX (WM_USER + 1)

#define ID_MENU_ITEM_UPDATE_DATA (WM_USER + 10)
#define ID_MENU_ITEM_TIMERS_ACTIVE (WM_USER + 11)
#define ID_MENU_ITEM_SHOW_SUM (WM_USER + 12)
#define ID_MENU_ITEM_STAY_ON_TOP (WM_USER + 13)
#define ID_MENU_ITEM_EXIT (WM_USER + 14)
#define ID_MENU_ITEM_INTERVAL_RAM_500MS (WM_USER + 20)
#define ID_MENU_ITEM_INTERVAL_RAM_1SEC (WM_USER + 21)
#define ID_MENU_ITEM_INTERVAL_RAM_2SEC (WM_USER + 22)
#define ID_MENU_ITEM_INTERVAL_RAM_3SEC (WM_USER + 23)
#define ID_MENU_ITEM_INTERVAL_RAM_4SEC (WM_USER + 24)
#define ID_MENU_ITEM_INTERVAL_RAM_5SEC (WM_USER + 25)
#define ID_MENU_ITEM_INTERVAL_DRIVES_500MS (WM_USER + 26)
#define ID_MENU_ITEM_INTERVAL_DRIVES_1SEC (WM_USER + 27)
#define ID_MENU_ITEM_INTERVAL_DRIVES_2SEC (WM_USER + 28)
#define ID_MENU_ITEM_INTERVAL_DRIVES_3SEC (WM_USER + 29)
#define ID_MENU_ITEM_INTERVAL_DRIVES_4SEC (WM_USER + 30)
#define ID_MENU_ITEM_INTERVAL_DRIVES_5SEC (WM_USER + 31)

HWND hMainWindow;
HWND hLabelRam;
HWND hListBoxDrives;

WNDPROC oldListBoxDrivesWndProc;

COLORREF colorBackground = RGB(240, 240, 240);
COLORREF colorRam = 0;
HBRUSH brushBackground;
HMENU contextMenu;
HMENU intervalsMenu = NULL;

int isTimersEnabled = 1;
int isStayOnTop = 1;
int isShowSum = 1;
int listBoxSelectedIndex = -1;
unsigned int selectedDrivesUpdateInterval;
unsigned int selectedRamUpdateInterval;

HMENU createIntervalsMenu()
{
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_500MS, L"RAM: 500 ms");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_1SEC, L"RAM: 1 second");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_2SEC, L"RAM: 2 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_3SEC, L"RAM: 3 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_4SEC, L"RAM: 4 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_RAM_5SEC, L"RAM: 5 seconds");

        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_500MS, L"DRIVES: 500 ms");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_1SEC, L"DRIVES: 1 second");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_2SEC, L"DRIVES: 2 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_3SEC, L"DRIVES: 3 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_4SEC, L"DRIVES: 4 seconds");
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_INTERVAL_DRIVES_5SEC, L"DRIVES: 5 seconds");
    }

    return hMenu;
}

HMENU createContextMenu()
{
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_UPDATE_DATA, L"�������� ������");

        int flagsItemTimersActive = isTimersEnabled ? MF_STRING | MF_CHECKED : MF_STRING;
        AppendMenu(hMenu, flagsItemTimersActive, ID_MENU_ITEM_TIMERS_ACTIVE, L"��������� �� �������");
        int flagsItemShowSum = isShowSum ? MF_STRING | MF_CHECKED : MF_STRING;
        AppendMenu(hMenu, flagsItemShowSum, ID_MENU_ITEM_SHOW_SUM, L"���������� �����");

        intervalsMenu = createIntervalsMenu();
        if (intervalsMenu)
        {
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)intervalsMenu, L"���������");
        }

        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        int flagsItemStayOnTop = isStayOnTop ? MF_STRING | MF_CHECKED : MF_STRING;
        AppendMenu(hMenu, flagsItemStayOnTop, ID_MENU_ITEM_STAY_ON_TOP, L"������ ���� ����");
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, ID_MENU_ITEM_EXIT, L"�����");
    }

    return hMenu;
}

void listDrivesFreeSpace()
{
    listBox_Clear(hListBoxDrives);

    wchar_t buffer[256];
    if (GetLogicalDriveStrings(256, buffer) > 0)
    {
        UINT64 summarySpace = 0ull;
        UINT64 summaryFreeSpace = 0ull;
        short driveCount = 0;

        LPWSTR drive = buffer;
        while (drive[0])
        {
            wchar_t driveName[3];
            memcpy(&driveName, &drive[0], 4);
            driveName[2] = '\0';

            wchar_t resultString[64];
            memset(&resultString, 0, 128);
            memcpy(&resultString, &driveName[0], 4);

            resultString[2] = '\x20';

            UINT64 freeSpace;
            UINT64 usedSpace;
            UINT64 totalSpace;
            if (getDiskSpace(drive, &totalSpace, &usedSpace, &freeSpace))
            {
                wchar_t freeSpaceFormatted[32];
                formatSize(freeSpaceFormatted, freeSpace);
                wcscat_s(resultString, 64, freeSpaceFormatted);

                if (totalSpace > 0ull)
                {
                    double percent = 100.0 / (double)totalSpace * freeSpace;
                    wchar_t percentFormatted[16];
                    const wchar_t* fmt = percent < 10.0f ? L" ( %.2f" : L" (%.2f";
                    swprintf_s(percentFormatted, 16, fmt, percent);
                    wcscat_s(percentFormatted, 16, L"%)");

                    wcscat_s(resultString, 64, percentFormatted);

                    summarySpace += totalSpace;
                }

                summaryFreeSpace += freeSpace;
            }
            else
            {
                memcpy(&resultString[wcslen(resultString)], L"<error>", 14);
            }

            listBox_AddString(hListBoxDrives, resultString);

            drive += wcslen(drive) + 1;
            driveCount++;
        }

        if (isShowSum && driveCount > 1)
        {
            if (summaryFreeSpace > 0ull)
            {
                wchar_t totalResultString[32];
                formatSize(totalResultString, summaryFreeSpace);
                wchar_t* plus = L"+: ";
                wchar_t totalFreeSpaceFormatted[32];
                totalFreeSpaceFormatted[0] = '\0';
                wcscat_s(totalFreeSpaceFormatted, 32, plus);
                wcscat_s(totalFreeSpaceFormatted, 32, totalResultString);

                double totalPercent = 100.0 / (double)summarySpace * summaryFreeSpace;
                wchar_t percentFormatted[16];
                const wchar_t* fmt = totalPercent < 10.0f ? L" ( %.2f" : L" (%.2f";
                swprintf_s(percentFormatted, 16, fmt, totalPercent);
                wcscat_s(percentFormatted, 16, L"%)");
                wcscat_s(totalFreeSpaceFormatted, 32, percentFormatted);

                listBox_AddString(hListBoxDrives, totalFreeSpaceFormatted);
            }
            else
            {
                listBox_AddString(hListBoxDrives, L"+: <error>");
            }
        }
    }
}

void updateDrives()
{
    listDrivesFreeSpace();

    int itemCount = listBox_GetItemCount(hListBoxDrives);
    if (itemCount > 0 && listBoxSelectedIndex < itemCount)
    {
        listBox_SetSelectedItemId(hListBoxDrives, listBoxSelectedIndex);
    }

    RECT r;
    if (GetWindowRect(hMainWindow, &r))
    {
        int itemHeight = listBox_GetItemHeight(hListBoxDrives);
        int lbHeight = itemHeight * (itemCount + 1);

        int windowWidth = r.right - r.left;
        SetWindowPos(hMainWindow, 0, 0, 0, windowWidth, lbHeight + 14, SWP_NOMOVE | SWP_NOZORDER);
        SetWindowPos(hListBoxDrives, 0, 4, 4, windowWidth - 8, lbHeight, SWP_NOZORDER);
        SetWindowPos(hLabelRam, 0, 4, lbHeight - 4, windowWidth - 8, 20, SWP_NOZORDER);
    }
}

void updateRam()
{
    const DWORD sizeOfMemoryStatusEx = sizeof(MEMORYSTATUSEX);
    MEMORYSTATUSEX memoryStatusEx;
    memset(&memoryStatusEx, 0, sizeOfMemoryStatusEx);
    memoryStatusEx.dwLength = sizeOfMemoryStatusEx;

    if (GlobalMemoryStatusEx(&memoryStatusEx))
    {
        wchar_t freeRamFormatted[32];
        formatRam(freeRamFormatted, memoryStatusEx.ullAvailPhys);

        wchar_t t[40];
        t[0] = '\0';
        wcscat_s(t, 40, L"Free RAM: ");
        wcscat_s(t, 40, freeRamFormatted);

        double percentFree = 100.0 / memoryStatusEx.ullTotalPhys * memoryStatusEx.ullAvailPhys;
        wchar_t percentFormatted[16];
        swprintf_s(percentFormatted, 16, L" (%.3f", percentFree);
        wcscat_s(percentFormatted, 16, L"%)");
        wcscat_s(t, 40, percentFormatted);

        if (percentFree > 30.0)
        {
            colorRam = RGB(0, 127, 0);
        }
        else if (percentFree > 10.0)
        {
            colorRam = RGB(127, 127, 0);
        }
        else
        {
            colorRam = RGB(255, 0, 0);
        }

        SetWindowText(hLabelRam, t);
    }
}

LRESULT CALLBACK listBoxDrivesWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_MENU_ITEM_UPDATE_DATA:
                    updateDrives();
                    updateRam();
                    break;

                case ID_MENU_ITEM_TIMERS_ACTIVE:
                    if (isTimersEnabled)
                    {
                        KillTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES);
                        KillTimer(hMainWindow, TIMER_ID_UPDATE_RAM);

                        checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 0u);

                        isTimersEnabled = 0;
                    }
                    else
                    {
                        SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                        SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);

                        checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);

                        isTimersEnabled = 1;
                    }

                    break;

                case ID_MENU_ITEM_SHOW_SUM:
                    if (isShowSum)
                    {
                        checkMenuItem(contextMenu, ID_MENU_ITEM_SHOW_SUM, 0u);

                        isShowSum = 0;
                    }
                    else
                    {
                        checkMenuItem(contextMenu, ID_MENU_ITEM_SHOW_SUM, 1u);

                        isShowSum = 1;
                    }

                    if (isTimersEnabled) { updateDrives(); }

                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_500MS:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 500u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_1SEC; id <= ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_500MS, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 500u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_1SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 1000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_500MS; id <= ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_1SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 1000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_2SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 2000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_500MS; id <= ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_2SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 2000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_3SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 3000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_500MS; id <= ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_3SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 3000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_4SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 4000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_500MS; id <= ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_4SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 4000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_DRIVES_5SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, 5000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_DRIVES_500MS; id < ID_MENU_ITEM_INTERVAL_DRIVES_5SEC; ++id)
                    {
                        checkMenuItem(contextMenu, id, 0u);
                    }
                    checkMenuItem(contextMenu, ID_MENU_ITEM_INTERVAL_DRIVES_5SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedDrivesUpdateInterval = 5000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_500MS:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 500u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_1SEC; id <= ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_500MS, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 500u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_1SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 1000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_500MS; id <= ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_1SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 1000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_2SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 2000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_500MS; id <= ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_2SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 2000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_3SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 3000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_500MS; id <= ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_3SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 3000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_4SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 4000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_500MS; id <= ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_4SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 4000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_INTERVAL_RAM_5SEC:
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_RAM, 5000u, NULL);
                    for (unsigned int id = ID_MENU_ITEM_INTERVAL_RAM_500MS; id < ID_MENU_ITEM_INTERVAL_RAM_5SEC; ++id)
                    {
                        checkMenuItem(intervalsMenu, id, 0u);
                    }
                    checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_5SEC, 1u);
                    checkMenuItem(contextMenu, ID_MENU_ITEM_TIMERS_ACTIVE, 1u);
                    isTimersEnabled = 1;
                    selectedRamUpdateInterval = 5000u;
                    SetTimer(hMainWindow, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
                    break;

                case ID_MENU_ITEM_STAY_ON_TOP:
                    if (isStayOnTop)
                    {
                        setIsWindowStayingOnTop(hMainWindow, 0);
                        checkMenuItem(contextMenu, ID_MENU_ITEM_STAY_ON_TOP, 0u);
                        isStayOnTop = 0;
                    }
                    else
                    {
                        setIsWindowStayingOnTop(hMainWindow, 1);
                        checkMenuItem(contextMenu, ID_MENU_ITEM_STAY_ON_TOP, 1u);
                        isStayOnTop = 1;
                    }

                    break;

                case ID_MENU_ITEM_EXIT:
                    DestroyWindow(hMainWindow);
                    break;
            }
            break;

        case WM_LBUTTONDBLCLK:
        {
            if (listBoxSelectedIndex >= 0)
            {
                int itemCount = listBox_GetItemCount(hListBoxDrives);
                if (isShowSum && itemCount > 1 && listBoxSelectedIndex >= itemCount - 1)
                {
                    break;
                }
                openDiskFromListBox(hListBoxDrives, listBoxSelectedIndex);
            }
            break;
        }

        case WM_RBUTTONUP:
        {
            POINT p;
            if (GetCursorPos(&p))
            {
                TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hListBoxDrives, NULL);
            }

            break;
        }
    }

    return CallWindowProc(oldListBoxDrivesWndProc, hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK mainWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static HFONT font;
    static POINT mousePosition;
    static int canDragWindow = 0;
    static wchar_t iniFilePath[256];

    switch (uMessage)
	{
        case WM_CTLCOLORLISTBOX:
            SetBkColor((HDC)wParam, colorBackground);
            return (LRESULT)brushBackground;

        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, colorRam);
            SetBkColor((HDC)wParam, colorBackground);
            return (LRESULT)brushBackground;

        case WM_TIMER:
            switch (wParam)
            {
                case TIMER_ID_UPDATE_DRIVES:
                    updateDrives();
                    break;

                case TIMER_ID_UPDATE_RAM:
                    updateRam();
                    break;
            }
            break;

        case WM_LBUTTONDOWN:
        {
            mousePosition.x = LOWORD(lParam);
            mousePosition.y = HIWORD(lParam);

            HCURSOR cursorSizeAll = LoadCursor(NULL, IDC_SIZEALL);
            SetCursor(cursorSizeAll);
            SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)cursorSizeAll);

            SetCapture(hWnd);
            canDragWindow = 1;

            break;
        }

        case WM_MOUSEMOVE:
        {
            if (canDragWindow)
            {
                RECT r;
                if (GetWindowRect(hWnd, &r))
                {
                    short x = LOWORD(lParam);
                    short y = HIWORD(lParam);

                    int newX = r.left + x - mousePosition.x;
                    int newY = r.top + y - mousePosition.y;

                    SetWindowPos(hWnd, 0, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                }
            }

            break;
        }

        case WM_KILLFOCUS:
        case WM_LBUTTONUP:
        {
            if (canDragWindow)
            {
                HCURSOR cursorArrow = LoadCursor(NULL, IDC_ARROW);
                SetCursor(cursorArrow);
                SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)cursorArrow);

                canDragWindow = 0;
                ReleaseCapture();
            }

            break;
        }

        case WM_RBUTTONUP:
        {
            POINT p;
            if (GetCursorPos(&p))
            {
                TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hListBoxDrives, NULL);
            }

            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_LISTBOX:
                    if (HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        listBoxSelectedIndex = listBox_GetSelectedItemId(hListBoxDrives);
                    }
                    break;
            }
            break;

        case WM_CREATE:
        {
            hMainWindow = hWnd;

            contextMenu = createContextMenu();

            //Get self directory and config file name.
            wchar_t selfDirectory[256];
            getSelfDirectory(selfDirectory);
            memcpy(&iniFilePath, &selfDirectory, (wcslen(selfDirectory) + 1) * 2);
            wcscat_s(iniFilePath, 256, L"\\DSW.ini");

            isShowSum = IniGetInt(iniFilePath, L"Main", L"Show sum", 1);

            font = createFont(L"Lucida Console", 8, 14);

            hLabelRam = createLabel(L"RAM", 0, 0, 0, 0, hWnd);
            if (hLabelRam > 0)
            {
                SendMessage(hLabelRam, WM_SETFONT, (WPARAM)font, 0);

                updateRam();

                selectedRamUpdateInterval = (unsigned int)IniGetInt(iniFilePath, L"Main", L"RAM update interval", 0);
                switch (selectedRamUpdateInterval)
                {
                    case 500u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_500MS, 1u);
                        break;

                    case 1000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_1SEC, 1u);
                        break;

                    case 2000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_2SEC, 1u);
                        break;

                    case 3000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_3SEC, 1u);
                        break;

                    case 4000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_4SEC, 1u);
                        break;

                    case 5000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_5SEC, 1u);
                        break;

                    default:
                        selectedRamUpdateInterval = 1000u;
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_RAM_1SEC, 1u);
                        break;
                }

                SetTimer(hWnd, TIMER_ID_UPDATE_RAM, selectedRamUpdateInterval, NULL);
            }

            hListBoxDrives = createListBox(L"Drive list", 0, 0, 0, 0, ID_LISTBOX, hWnd);
            if (hListBoxDrives > 0)
            {
                SendMessage(hListBoxDrives, WM_SETFONT, (WPARAM)font, 0);

                updateDrives();

                oldListBoxDrivesWndProc = (WNDPROC)SetWindowLongPtr(hListBoxDrives, GWLP_WNDPROC, (LONG_PTR)&listBoxDrivesWndProc);

                selectedDrivesUpdateInterval = (unsigned int)IniGetInt(iniFilePath, L"Main", L"Drives update interval", 0);
                switch (selectedDrivesUpdateInterval)
                {
                    case 500u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_500MS, 1u);
                        break;

                    case 1000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_1SEC, 1u);
                        break;

                    case 2000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_2SEC, 1u);
                        break;

                    case 3000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_3SEC, 1u);
                        break;

                    case 4000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_4SEC, 1u);
                        break;

                    case 5000u:
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_5SEC, 1u);
                        break;

                    default:
                        selectedDrivesUpdateInterval = 5000u;
                        checkMenuItem(intervalsMenu, ID_MENU_ITEM_INTERVAL_DRIVES_5SEC, 1u);
                        break;
                }

                SetTimer(hWnd, TIMER_ID_UPDATE_DRIVES, selectedDrivesUpdateInterval, NULL);
            }

            //Load window position.
            const int magicNumber = -10001;
            int windowPositionX = IniGetInt(iniFilePath, L"Main", L"Window left", magicNumber);
            int windowPositionY = IniGetInt(iniFilePath, L"Main", L"Window top", magicNumber);

            if (windowPositionX == magicNumber && windowPositionY == magicNumber)
            {
                POINT cursorPosition;
                if (GetCursorPos(&cursorPosition))
                {
                    windowPositionX = cursorPosition.x;
                    windowPositionY = cursorPosition.y;
                }
                else
                {
                    windowPositionX = windowPositionY = 0;
                }
            }

            //Apply loaded position.
            SetWindowPos(hWnd, 0, windowPositionX, windowPositionY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            break;
        }

        case WM_DESTROY:
        {
            KillTimer(hWnd, TIMER_ID_UPDATE_DRIVES);
            KillTimer(hWnd, TIMER_ID_UPDATE_RAM);
            DeleteObject(font);
            DestroyMenu(intervalsMenu);
            DestroyMenu(contextMenu);

            IniSetInt(iniFilePath, L"Main", L"Show sum", isShowSum);
            IniSetInt(iniFilePath, L"Main", L"Drives update interval", (int)selectedDrivesUpdateInterval);
            IniSetInt(iniFilePath, L"Main", L"RAM update interval", (int)selectedRamUpdateInterval);

            //Save window position.
            RECT r;
            if (GetWindowRect(hMainWindow, &r))
            {
                IniSetInt(iniFilePath, L"Main", L"Window left", r.left);
                IniSetInt(iniFilePath, L"Main", L"Window top", r.top);
            }

            PostQuitMessage(0);
            break;
        }

        case WM_QUERYENDSESSION:
            return 1;

        case WM_ENDSESSION:
            if (wParam == 1) { DestroyWindow(hWnd); }
            return 0;

        default:
            return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }

	return 0;
}

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    const LPCWSTR MY_CLASS = L"DSW_C_MainWnd_class";
    const LPCWSTR MY_TITLE = L"Disk space watcher";
    const HINSTANCE selfInstance = hInstance;

    brushBackground = CreateSolidBrush(colorBackground);

    WNDCLASSEX wndClassEx;
    memset(&wndClassEx, 0, sizeof(WNDCLASSEX));

    wndClassEx.cbSize        = sizeof(WNDCLASSEX);
    wndClassEx.style         = CS_HREDRAW | CS_VREDRAW;
    wndClassEx.hbrBackground = brushBackground;
    wndClassEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndClassEx.lpfnWndProc   = mainWndProc;
    wndClassEx.hInstance     = selfInstance;
    wndClassEx.lpszClassName = MY_CLASS;

    if (!RegisterClassEx(&wndClassEx))
    {
        MessageBox(0, L"Window class registering failed!", MY_TITLE, MB_ICONERROR);
        return 0;
    }

    const int windowWidth = 246;
    const int windowHeight = 100;

    DWORD styleEx = isStayOnTop ? WS_EX_TOOLWINDOW | WS_EX_TOPMOST : WS_EX_TOOLWINDOW;
    hMainWindow = CreateWindowEx(styleEx,
        wndClassEx.lpszClassName, MY_TITLE,
        WS_VISIBLE | WS_CLIPCHILDREN | WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
        NULL, NULL, selfInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0u, 0u))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!UnregisterClass(wndClassEx.lpszClassName, wndClassEx.hInstance))
    {
        DeleteObject(brushBackground);
        MessageBox(0, L"Window class unregistering failed!", MY_TITLE, MB_ICONERROR);
    }

    return 0;
}
