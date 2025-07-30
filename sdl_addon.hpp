#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <windows.h>
#include <commdlg.h> 
#include <string>
#include <iostream>

    
// sdl button
class Button {
public:
    int x, y, w, h;
    std::string title;
    bool clicked = false;

    SDL_Color gray = {34, 53, 66, 255};
    SDL_Color blue = {37, 160, 245, 255};
    SDL_Color white = {221, 227, 237, 255};

private:
    Uint32 clickTime = 0; // When was clicked
    const Uint32 clickDuration = 300; // ms to keep blue color

public:
    Button(int x_, int y_, int w_, int h_, const std::string& title_)
        : x(x_), y(y_), w(w_), h(h_), title(title_) {}

    void handleEvent(const SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mx = e.button.x;
            int my = e.button.y;
            if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
                clicked = true;
                clickTime = SDL_GetTicks();
            }
        }
    }

    void update() {
        if (clicked) {
            Uint32 now = SDL_GetTicks();
            if (now - clickTime > clickDuration) {
                clicked = false;
            }
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font) {
        SDL_Color bgColor = clicked ? blue : gray;

        // Draw button rectangle
        SDL_SetRenderDrawColor(ren, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(ren, &rect);

        // Render title text
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, title.c_str(), white);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(ren, textSurface);
            int textW = textSurface->w;
            int textH = textSurface->h;
            SDL_FreeSurface(textSurface);

            SDL_Rect dst = {x + (w - textW) / 2, y + (h - textH) / 2, textW, textH};
            SDL_RenderCopy(ren, textTexture, nullptr, &dst);
            SDL_DestroyTexture(textTexture);
        }
    }
};


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
