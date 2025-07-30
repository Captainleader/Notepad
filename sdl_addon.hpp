#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

   
// open a file

struct FileDialogResult {
    bool success;
    std::wstring filePath;
};

inline FileDialogResult OpenFileDialog(HWND owner = nullptr) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    bool result = GetOpenFileNameW(&ofn) == TRUE;

    if (result) {
        return { true, std::wstring(fileName) };
    } else {
        return { false, L"" };
    }
}



