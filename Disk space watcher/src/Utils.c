#include "Utils.h"

void formatSize(wchar_t* outputBuffer, long long llSize)
{
    if (llSize < 0ll)
    {
        wchar_t* error = L"<error>";
        memcpy(outputBuffer, error, 14);
        outputBuffer[7] = '\0';
        return;
    }

    const int       KB = 1000;
    const int       MB = 1000000;
    const int       GB = 1000000000;
    const long long TB = 1000000000000;
    const long long PB = 1000000000000000;

    long long b = llSize % KB;
    long long kb = (llSize % MB) / KB;
    long long mb = (llSize % GB) / MB;
    long long gb = (llSize % TB) / GB;
    long long tb = (llSize % PB) / TB;

    wchar_t buffer[32];

    wchar_t tbBuf[5];
    swprintf_s(tbBuf, 5, L"%I64d", tb);
    size_t len = wcslen(tbBuf);
    tbBuf[len] = '\x2C';
    tbBuf[len + 1] = '\0';
    memcpy(&buffer, &tbBuf, (wcslen(tbBuf) + 1) * 2);

    wchar_t gbBuf[5];
    swprintf_s(gbBuf, 5, L"%03I64d", gb);
    len = wcslen(gbBuf);
    gbBuf[len] = '\x2C';
    gbBuf[len + 1] = '\0';
    memcpy(&buffer[wcslen(buffer)], &gbBuf, (wcslen(gbBuf) + 1) * 2);

    wchar_t mbBuf[5];
    swprintf_s(mbBuf, 5, L"%03I64d", mb);
    len = wcslen(mbBuf);
    mbBuf[len] = '\x2C';
    mbBuf[len + 1] = '\0';
    memcpy(&buffer[wcslen(buffer)], &mbBuf, (wcslen(mbBuf) + 1) * 2);

    wchar_t kbBuf[5];
    swprintf_s(kbBuf, 5, L"%03I64d", kb);
    len = wcslen(kbBuf);
    kbBuf[len] = '\x2C';
    kbBuf[len + 1] = '\0';
    memcpy(&buffer[wcslen(buffer)], &kbBuf, (wcslen(kbBuf) + 1) * 2);

    wchar_t bBuf[5];
    swprintf_s(bBuf, 5, L"%03I64d", b);
    len = wcslen(bBuf);
    bBuf[len] = '\0';
    memcpy(&buffer[wcslen(buffer)], &bBuf, (wcslen(mbBuf) + 1) * 2);

    memcpy(outputBuffer, &buffer, (wcslen(buffer) + 1) * 2);
}

void formatRam(wchar_t* outputBuffer, size_t ullSize)
{
    const int       KB = 1000;
    const int       MB = 1000000;
    const int       GB = 1000000000;
    const long long TB = 1000000000000;

    long long b = ullSize % KB;
    long long kb = (ullSize % MB) / KB;
    long long mb = (ullSize % GB) / MB;
    long long gb = (ullSize % TB) / GB;

    wchar_t buffer[16];
    buffer[0] = '\0';

    if (ullSize < KB)
    {
        wchar_t bBuf[4];
        swprintf_s(bBuf, 4, L"%I64d", b);
        wcscat_s(buffer, 16, bBuf);
        wcscat_s(buffer, 16, L" bytes");
    }
    else if (ullSize >= KB && ullSize < MB)
    {
        wchar_t kbBuf[4];
        swprintf_s(kbBuf, 4, L"%I64d", kb);
        wcscat_s(buffer, 16, kbBuf);
        wcscat_s(buffer, 16, L",");

        wchar_t bBuf[4];
        swprintf_s(bBuf, 4, L"%03I64d", b);
        wcscat_s(buffer, 16, bBuf);
        wcscat_s(buffer, 16, L" KB");
    }
    else if (ullSize >= MB && ullSize < GB)
    {
        wchar_t mbBuf[4];
        swprintf_s(mbBuf, 4, L"%I64d", mb);
        wcscat_s(buffer, 16, mbBuf);
        wcscat_s(buffer, 16, L",");

        wchar_t kbBuf[4];
        swprintf_s(kbBuf, 4, L"%03I64d", kb);
        wcscat_s(buffer, 16, kbBuf);
        wcscat_s(buffer, 16, L" MB");
    }
    else if (ullSize >= GB)
    {
        wchar_t gbBuf[4];
        swprintf_s(gbBuf, 4, L"%I64d", gb);
        wcscat_s(buffer, 16, gbBuf);
        wcscat_s(buffer, 16, L",");

        wchar_t mbBuf[4];
        swprintf_s(mbBuf, 4, L"%03I64d", mb);
        wcscat_s(buffer, 16, mbBuf);
        wcscat_s(buffer, 16, L" GB");
    }

    memcpy(outputBuffer, &buffer, (wcslen(buffer) + 1) * 2);
}

HFONT createFont(wchar_t* fontName, unsigned int width, unsigned int height)
{
    LOGFONT logFont;
    memset(&logFont, 0, sizeof(LOGFONT));

    logFont.lfWidth = width;
    logFont.lfHeight = height;
    memcpy(logFont.lfFaceName, fontName, wcslen(fontName) * 2);

    return CreateFontIndirect(&logFont);
}

void getSelfDirectory(wchar_t* outputBuffer)
{
    LPWSTR commandLine = GetCommandLine();

    wchar_t buffer[256];
    memset(&buffer, 0, 512);

    size_t length = wcslen(commandLine);
    int i = 1;
    for (; i < length; ++i)
    {
        if (commandLine[i] == '\"') { break; }
        buffer[i - 1] = commandLine[i];
    }

    for (; i > 0; --i)
    {
        if (buffer[i] == '\\') { break; }
    }
    buffer[i] = '\0';

    memcpy(outputBuffer, &buffer, (wcslen(buffer) + 1) * 2);
}

BOOL setIsWindowStayingOnTop(HWND windowHandle, int stayOnTop)
{
    return SetWindowPos(windowHandle, stayOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

int checkMenuItem(HMENU menuHandle, UINT itemId, UINT checked)
{
    MENUITEMINFO menuItemInfo;
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_STATE;
    if (GetMenuItemInfo(menuHandle, itemId, 0, &menuItemInfo))
    {
        menuItemInfo.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
        return SetMenuItemInfo(menuHandle, itemId, 0, &menuItemInfo);
    }

    return 0;
}

int getDiskSpace(wchar_t* diskRoot, UINT64* totalSpace, UINT64* usedSpace, UINT64* freeSpace)
{
    if (GetDiskFreeSpaceEx(diskRoot, NULL, (PULARGE_INTEGER)totalSpace, (PULARGE_INTEGER)freeSpace))
    {
        *usedSpace = *totalSpace - *freeSpace;
        return 1;
    }
    else
    {
        *totalSpace = *usedSpace = *freeSpace = 0ull;
        return 0;
    }
}

int openDiskFromListBox(HWND listBoxHandle, int itemId)
{
    int stringLength = listBox_GetStringLength(listBoxHandle, itemId);
    if (stringLength == LB_ERR) { return LB_ERR; }

    if (stringLength > 1)
    {
        size_t bufferSize = (size_t)((stringLength + 1) * 2);
        wchar_t* buffer = (wchar_t*)malloc(bufferSize);
        if (buffer)
        {
            memset(buffer, 0, bufferSize);
            stringLength = listBox_GetString(listBoxHandle, itemId, buffer);
            if (stringLength == LB_ERR)
            {
                free(buffer);
                return LB_ERR;
            }

            if (stringLength > 1)
            {
                wchar_t driveRoot[4];
                memcpy(&driveRoot, buffer, 4);
                free(buffer);
                driveRoot[2] = '\\';
                driveRoot[3] = '\0';

                INT_PTR errorCode = (INT_PTR)ShellExecute(NULL, NULL, driveRoot, NULL, NULL, SW_SHOW);
                return errorCode > 32ll;
            }

            free(buffer);
        }
    }

    return 0;
}
