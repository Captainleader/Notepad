#include <windows.h>
#include <commdlg.h>
#include <string>
#include <iostream>

struct FileDialogResult {
    bool success;
    std::wstring filePath;
};

FileDialogResult OpenFileDialog(HWND owner = nullptr) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    bool result = GetOpenFileNameW(&ofn) == TRUE;
    return { result, result ? std::wstring(fileName) : L"" };
}

int main() {
    FileDialogResult result = OpenFileDialog();
    if (result.success) {
        std::wcout << L"Selected file: " << result.filePath << std::endl;
    } else {
        std::wcout << L"No file selected." << std::endl;
    }
    return 0;
}
