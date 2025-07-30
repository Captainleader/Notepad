#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <commdlg.h> 
#include "logic.hpp"
#include "sdl_addon.hpp"
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <codecvt>
SDL_Color lineNumberColor = {150, 150, 150, 255};
void loadfile(std::vector<lineObj>& lines) {
    lines.clear();
    lines.shrink_to_fit();
    auto res = OpenFileDialog();
    if (res.success) {
        std::wcout << L"Selected file: " << res.filePath << L"\n";
    } else {
        std::wcout << L"No file selected or dialog canceled.\n";
    }
    std::wifstream inputFile(res.filePath.c_str());
    std::wstring fileLine;
    while (std::getline(inputFile, fileLine)) {
        lineObj newLine;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string temptext = converter.to_bytes(fileLine);
        newLine.text = temptext;
        newLine.texture = nullptr; // Initialize lazily
        newLine.textureValid = false;
        newLine.height = 18; // Default height
        newLine.width = 0;
        lines.push_back(newLine);
    }
    inputFile.close();
}

SDL_Color Black = {0, 0, 0, 255};
int totallineheight = 0;

int main(int argc, char** argv) {
    // Initialize SDL and SDLTTF
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    TTF_Font* font = TTF_OpenFont("c:/windows/fonts/segoeui.ttf", 18);
    if (!font) {
        std::cerr << "Failed to open font: " << TTF_GetError() << "\n";
        return 1;
    }

    TTF_Font* settingsfont = TTF_OpenFont("c:/windows/fonts/consola.ttf", 18);
    if (!settingsfont) {
        std::cerr << "Failed to open settingsfont: " << TTF_GetError() << "\n";
        return 1;
    }

    // Initialize window and renderer
    SDL_Window* win = SDL_CreateWindow("Notepad", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Rect cursordst = {0, 0, 2, 18};

    // Load file
    std::vector<lineObj> lines;
    loadfile(lines);

    // Initialize cursor logic
    bool showCursor = true;
    Uint32 lastToggle = SDL_GetTicks();
    const Uint32 blinkInterval = 500;
    
    // Cache font metrics
    int fontHeight = TTF_FontHeight(font);
    
    // Smooth scrolling variables
    float scrollY = 0.0f;
    float targetScrollY = 0.0f;
    const float scrollSpeed = 0.15f; // Interpolation factor for smooth scrolling

    int winH, winW;
    SDL_GetWindowSize(win, &winW, &winH);

    // Initialize sliders
    HorizontalSlider marginX(0, 0, 150, 40, 0, 200, 40);
    HorizontalSlider marginY(200, 0, 150, 40, 0, 200, 70);
    HorizontalSlider fontsize(400, 0, 150, 40, 8, 72, 18);
    HorizontalSlider lineSpacing(600, 0, 150, 40, 100, 200, 120);
    VerticalSlider scroll(winW-40, 0, 40, winH, 0, 100, 0);
    Button myButton(800, 0, 150, 40, "Open File");

    int oldsize = 18;
    int line = 0;
    int column = 0;
    
    // Pre-calculate total content height
    auto calculateTotalHeight = [&]() {
        float lineHeight = fontHeight * (lineSpacing.Value / 100.0f);
        return lines.size() * lineHeight;
    };

    // Cursor position cache
    int cursorPixelX = marginX.Value;
    int cursorPixelY = marginY.Value;
    
    // Optimized cursor update function
    auto updateCursorPosition = [&]() {
        if (line >= 0 && line < lines.size()) {
            // Calculate Y position with proper line spacing
            float lineHeight = fontHeight * (lineSpacing.Value / 100.0f);
            cursorPixelY = line * lineHeight + marginY.Value;
            
            // Calculate X position
            if (column > 0 && !lines[line].text.empty()) {
                std::string substr = lines[line].text.substr(0, std::min(column, (int)lines[line].text.size()));
                int width;
                TTF_SizeUTF8(font, substr.c_str(), &width, nullptr);
                cursorPixelX = width + marginX.Value;
            } else {
                cursorPixelX = marginX.Value;
            }
            
            // Update cursor rectangle with proper screen position
            cursordst.x = cursorPixelX;
            cursordst.y = cursorPixelY - scrollY;
            cursordst.w = 2;
            cursordst.h = fontHeight;
        }
    };

    // Ensure texture validity for visible lines
    auto ensureTextureValid = [&](int lineIndex) {
    if (lineIndex < 0 || lineIndex >= (int)lines.size()) return;

    if (!lines[lineIndex].textureValid) {
        // Destroy old textures if exist
        if (lines[lineIndex].texture) {
            SDL_DestroyTexture(lines[lineIndex].texture);
            lines[lineIndex].texture = nullptr;
        }
        if (lines[lineIndex].lineNumberTexture) {
            SDL_DestroyTexture(lines[lineIndex].lineNumberTexture);
            lines[lineIndex].lineNumberTexture = nullptr;
        }

        // Create texture for line text
        if (!lines[lineIndex].text.empty()) {
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, lines[lineIndex].text.c_str(), Black);
            if (surface) {
                lines[lineIndex].texture = SDL_CreateTextureFromSurface(ren, surface);
                lines[lineIndex].width = surface->w;
                lines[lineIndex].height = surface->h;
                SDL_FreeSurface(surface);
            }
        } else {
            lines[lineIndex].texture = nullptr;
            lines[lineIndex].width = 0;
            lines[lineIndex].height = fontHeight;
        }

        // Create texture for line number
        std::string lineNumberStr = std::to_string(lineIndex + 1);
        SDL_Surface* lineNumberSurface = TTF_RenderUTF8_Blended(font, lineNumberStr.c_str(), lineNumberColor);
        if (lineNumberSurface) {
            lines[lineIndex].lineNumberTexture = SDL_CreateTextureFromSurface(ren, lineNumberSurface);
            lines[lineIndex].lineNumberWidth = lineNumberSurface->w;
            lines[lineIndex].lineNumberHeight = lineNumberSurface->h;
            SDL_FreeSurface(lineNumberSurface);
        } else {
            lines[lineIndex].lineNumberTexture = nullptr;
            lines[lineIndex].lineNumberWidth = 0;
            lines[lineIndex].lineNumberHeight = fontHeight;
        }

        lines[lineIndex].textureValid = true;
    }
};




    SDL_StartTextInput();
    bool quit = false;
    SDL_Event e;
    bool typing = true;

    // Main loop
    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        int mouseX, mouseY;
        SDL_GetWindowSize(win, &winW, &winH);
        SDL_GetMouseState(&mouseX, &mouseY);
        scroll.x = winW - 40;
        scroll.h = winH;
        if (myButton.clicked)
        {
            loadfile(lines);
        }
        
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                    
                case SDL_TEXTINPUT: {
                    if (typing && line >= 0 && line < lines.size()) {
                        lines[line].text.insert(column, e.text.text);
                        lines[line].textureValid = false;
                        column += strlen(e.text.text);
                        updateCursorPosition();
                        showCursor = true;
                    }
                    break;
                }
                
                case SDL_KEYDOWN: {
                    if (!typing) break;
                    
                    bool ctrlPressed = (SDL_GetModState() & KMOD_CTRL) != 0;
                    
                    switch (e.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (column > 0) {
                                column--;
                            } else if (line > 0) {
                                line--;
                                column = lines[line].text.size();
                            }
                            updateCursorPosition();
                            showCursor = true;
                            break;
                            
                        case SDLK_RIGHT:
                            if (column < lines[line].text.size()) {
                                column++;
                            } else if (line < lines.size() - 1) {
                                line++;
                                column = 0;
                            }
                            updateCursorPosition();
                            showCursor = true;
                            break;
                            
                        case SDLK_UP:
                            if (line > 0) {
                                line--;
                                column = std::min(column, (int)lines[line].text.size());
                                updateCursorPosition();
                                showCursor = true;
                            }
                            break;
                            
                        case SDLK_DOWN:
                            if (line < lines.size() - 1) {
                                line++;
                                column = std::min(column, (int)lines[line].text.size());
                                updateCursorPosition();
                                showCursor = true;
                            }
                            break;
                            
                        case SDLK_HOME:
                            column = 0;
                            updateCursorPosition();
                            showCursor = true;
                            break;
                            
                        case SDLK_END:
                            column = lines[line].text.size();
                            updateCursorPosition();
                            showCursor = true;
                            break;
                            
                        case SDLK_BACKSPACE:
                            if (column > 0) {
                                lines[line].text.erase(column - 1, 1);
                                lines[line].textureValid = false;
                                column--;
                                updateCursorPosition();
                            } else if (line > 0) {
                                // Merge with previous line
                                column = lines[line - 1].text.size();
                                lines[line - 1].text += lines[line].text;
                                lines[line - 1].textureValid = false;
                                lines.erase(lines.begin() + line);
                                line--;
                                updateCursorPosition();
                            }
                            showCursor = true;
                            break;
                            
                        case SDLK_DELETE:
                            if (column < lines[line].text.size()) {
                                lines[line].text.erase(column, 1);
                                lines[line].textureValid = false;
                            } else if (line < lines.size() - 1) {
                                // Merge with next line
                                lines[line].text += lines[line + 1].text;
                                lines[line].textureValid = false;
                                lines.erase(lines.begin() + line + 1);
                            }
                            updateCursorPosition();
                            showCursor = true;
                            break;
                            
                        case SDLK_RETURN:
                            {
                                lineObj newLine;
                                newLine.text = lines[line].text.substr(column);
                                newLine.texture = nullptr;
                                newLine.textureValid = false;
                                newLine.height = fontHeight;
                                newLine.width = 0;
                                
                                lines[line].text = lines[line].text.substr(0, column);
                                lines[line].textureValid = false;
                                
                                lines.insert(lines.begin() + line + 1, newLine);
                                line++;
                                column = 0;
                                updateCursorPosition();
                                showCursor = true;
                            }
                            break;
                    }
                    break;
                }
                
                case SDL_MOUSEBUTTONDOWN:
                    if (marginX.Value < mouseX && mouseX < winW - 40 && mouseY > marginY.Value && mouseY < winH) {
                        typing = true;
                        
                        // Find clicked line
                        float lineHeight = fontHeight * (lineSpacing.Value / 100.0f);
                        int clickedLine = (mouseY + scrollY - marginY.Value) / lineHeight;
                        
                        if (clickedLine >= 0 && clickedLine < lines.size()) {
                            line = clickedLine;
                            
                            // Find column position
                            int mouseRelativeX = mouseX - marginX.Value;
                            column = 0;
                            
                            if (!lines[line].text.empty() && mouseRelativeX > 0) {
                                int bestDiff = INT_MAX;
                                for (int j = 0; j <= lines[line].text.size(); j++) {
                                    int width = 0;
                                    if (j > 0) {
                                        std::string substr = lines[line].text.substr(0, j);
                                        TTF_SizeUTF8(font, substr.c_str(), &width, nullptr);
                                    }
                                    
                                    int diff = abs(width - mouseRelativeX);
                                    if (diff < bestDiff) {
                                        bestDiff = diff;
                                        column = j;
                                    }
                                }
                            }
                            
                            updateCursorPosition();
                            showCursor = true;
                        }
                    } else {
                        typing = false;
                    }
                    break;
                    
                case SDL_MOUSEWHEEL:
                    if (typing) {
                        float totalHeight = calculateTotalHeight();
                        float maxScroll = std::max(0.0f, totalHeight - (winH - marginY.Value));
                        targetScrollY = std::max(0.0f, std::min(maxScroll, targetScrollY - e.wheel.y * 60.0f));
                    }
                    break;
            }
            
            // Handle slider events
            marginX.handleEvent(e);
            marginY.handleEvent(e);
            fontsize.handleEvent(e);
            scroll.handleEvent(e);
            lineSpacing.handleEvent(e);
            myButton.handleEvent(e);
        }

        // Update sliders
        myButton.update();
        marginX.update();
        marginY.update();
        fontsize.update();
        scroll.update();
        lineSpacing.update();

        // Handle manual scroll slider changes
        if (scroll.dragging) {
            float totalHeight = calculateTotalHeight();
            float maxScroll = std::max(1.0f, totalHeight - (winH - marginY.Value));
            targetScrollY = (scroll.Value / 100.0f) * maxScroll;
        }

        // Smooth scrolling interpolation
        scrollY = scrollY + (targetScrollY - scrollY) * scrollSpeed;
        
        // Update scroll slider based on actual scroll position (only if not being dragged)
        if (!scroll.dragging) {
            float totalHeight = calculateTotalHeight();
            float maxScroll = std::max(1.0f, totalHeight - (winH - marginY.Value));
            if (maxScroll > 0) {
                scroll.Value = std::min(100, std::max(0, (int)((scrollY / maxScroll) * 100)));
            }
        } //砕けたチョークっぽいフリーフォント「チョークS

        // Handle font size changes
        if (fontsize.Value != oldsize) {
            oldsize = fontsize.Value;
            TTF_CloseFont(font);
            font = TTF_OpenFont("c:/windows/fonts/segoeui.ttf", oldsize);
            fontHeight = TTF_FontHeight(font);
            
            // Invalidate all textures
            for (auto& lineObj : lines) {
                lineObj.textureValid = false;
                if (lineObj.texture) {
                    SDL_DestroyTexture(lineObj.texture);
                    lineObj.texture = nullptr;
                }
            }
            
            updateCursorPosition();
        }

        // Auto-scroll to keep cursor visible
        if (typing) {
            float cursorScreenY = cursorPixelY - scrollY;
            
            if (cursorScreenY < marginY.Value + 10) {
                targetScrollY = cursorPixelY - marginY.Value - 10;
            } else if (cursorScreenY + fontHeight > winH - 10) {
                targetScrollY = cursorPixelY - winH + fontHeight + 50;
            }
            
            float totalHeight = calculateTotalHeight();
            float maxScroll = std::max(0.0f, totalHeight - (winH - marginY.Value));
            targetScrollY = std::max(0.0f, std::min(maxScroll, targetScrollY));
        }

        // Rendering
        SDL_SetRenderDrawColor(ren, 71, 76, 84, 255);
        SDL_RenderClear(ren);
        
        // Draw typing area background
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_Rect typingArea = {marginX.Value - 5, marginY.Value, winW - marginX.Value, winH - marginY.Value};
        SDL_RenderFillRect(ren, &typingArea);

        // Calculate visible line range for culling
        float lineHeight = fontHeight * (lineSpacing.Value / 100.0f);
        int firstVisibleLine = std::max(0, (int)((scrollY - marginY.Value) / lineHeight) - 1);
        int lastVisibleLine = std::min((int)lines.size() - 1, (int)((scrollY + winH - marginY.Value) / lineHeight) + 1);

        // Render visible lines only
for (int i = firstVisibleLine; i <= lastVisibleLine; i++) {
    float line_y = i * lineHeight + marginY.Value - scrollY;

    if (line_y > marginY.Value - lineHeight && line_y < winH) {
        ensureTextureValid(i);

        // Render line number texture (5 pixels before marginX)
        if (lines[i].lineNumberTexture) {
            SDL_Rect lineNumberDst = {
                marginX.Value - 15 - lines[i].lineNumberWidth,
                (int)line_y,
                lines[i].lineNumberWidth,
                lines[i].lineNumberHeight
            };
            SDL_RenderCopy(ren, lines[i].lineNumberTexture, nullptr, &lineNumberDst);
        }

        // Render actual line text
        if (lines[i].texture) {
            SDL_Rect dstLine = { marginX.Value, (int)line_y, lines[i].width, lines[i].height };
            SDL_RenderCopy(ren, lines[i].texture, nullptr, &dstLine);
        }
    }
}



        // Render sliders
        marginX.draw(ren, settingsfont);
        marginY.draw(ren, settingsfont);
        fontsize.draw(ren, settingsfont);
        scroll.draw(ren, settingsfont);
        lineSpacing.draw(ren, settingsfont);
        myButton.draw(ren, settingsfont);

        // Cursor blinking
        Uint32 now = SDL_GetTicks();
        if (now - lastToggle >= blinkInterval) {
            showCursor = !showCursor;
            lastToggle = now;
        }

        // Render cursor
        if (showCursor && typing) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderFillRect(ren, &cursordst);
        }

        SDL_RenderPresent(ren);
    }

    // Cleanup
    for (auto& lineObj : lines) {
        if (lineObj.texture) {
            SDL_DestroyTexture(lineObj.texture);
        }
    }
    
    SDL_StopTextInput();
    TTF_CloseFont(font);
    TTF_CloseFont(settingsfont);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();

    return 0;
}