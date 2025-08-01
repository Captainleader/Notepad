
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <windows.h>
#include <commdlg.h>
#include <algorithm>
#include <cmath>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem> 
#include "logic.hpp"
#include "sdl_addon.hpp"


//For the time being I can say this is a almost fully working text Editor
// With auto save when closing file
std::string fontpath ="c:/windows/fonts/georgia.TTf";

// use mylog(text) to debug code
// Opening file at the start of the programm
std::wstring OpenFile(std::vector<lineObj> &lines , TTF_Font * font,SDL_Renderer * ren);

SDL_Color Black = {0, 0, 0, 255};


int main(int argc, char** argv) {
    // Initialize SDL and SDLTTF
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // Initialize window and renderer
    SDL_Window* win = SDL_CreateWindow("Notepad", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 960, 540,SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int winH, winW;
    SDL_GetWindowSize(win, &winW, &winH);

    // Initialize sliders
    HorizontalSlider marginX(0, 0, 150, 40, 0, 200, 40);
    HorizontalSlider marginY(200, 0, 150, 40, 0, 200, 70);
    HorizontalSlider fontsize(400, 0, 150, 40, 8, 72, 18);
    HorizontalSlider lineSpacing(600, 0, 150, 40, 100, 200, 100);
    VerticalSlider scroll(winW - 40, 0, 40, winH, 0, 100, 0);

    std::vector<TTF_Font*> fontCache;
    for (size_t i = fontsize.minValue; i < fontsize.maxValue+1; i++) {
        TTF_Font* font = TTF_OpenFont(fontpath.c_str(), i);
        if (!font) {
            std::cerr << "Failed to open font: " << TTF_GetError() << "\n";
            return 1;
        }
        fontCache.push_back(font);
    }

    TTF_Font* font = fontCache[8];

    TTF_Font* settingsfont = TTF_OpenFont("c:/windows/fonts/arial.TTf", 14);
    if (!settingsfont) {
        std::cerr << "Failed to open settingsfont: " << TTF_GetError() << "\n";
        return 1;
    }

    SDL_Rect cursordst;


    // load a file --------------------------------------------
    std::vector<lineObj> lines;
    std::wstring filePath = OpenFile(lines,font,ren);
    mylog("file opened");
    

    // initialize cursor logic
    bool showCursor = true;
    Uint32 lastToggle = SDL_GetTicks();
    const Uint32 blinkInterval = 500;  // milliseconds


    int scrollY;

    std::string mytext;


    bool getcurrenttext = true;
    std::string currenttext;


    int line = 0;
    int column = 0;
    int lineSpacingPercentage = 18;
    int totallineheight = lines.size()*lineSpacingPercentage;

    SDL_StartTextInput();

    // mouse coord declaration
    int mouseX, mouseY;
    SDL_Point Mouse = {mouseX, mouseY};
    SDL_Rect typingArea = {marginX.Value - 5, marginY.Value, winW, winH};
    // Main loop
    bool typing = true;
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        
        SDL_GetWindowSize(win, &winW, &winH);
        
        SDL_GetMouseState(&mouseX, &mouseY);
        Mouse = {mouseX, mouseY};
        scroll.x = winW - 40;
        typingArea = {marginX.Value - 5, marginY.Value, winW, winH};
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_TEXTINPUT: {

                    if (typing) {

                    lines[line].text.insert(column, e.text.text);
                    lines[line].needsUpdate = true;
                    update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 1,0, showCursor,lineSpacingPercentage);
                    }

                    break;
                } 
                case SDL_KEYDOWN:

                    if (e.key.keysym.sym == SDLK_LEFT) {
                        update_cursor_position(
                            font, lines, column, line, cursordst, marginX.Value,
                            -1, 0, showCursor, 
                            lineSpacingPercentage);
                    }

                    if (e.key.keysym.sym == SDLK_RIGHT) {
                        update_cursor_position(
                            font, lines, column, line, cursordst, marginX.Value,
                            1, 0, showCursor, 
                            lineSpacingPercentage);
                    }
                    //Handle deleting stuff
                    if (e.key.keysym.sym == SDLK_BACKSPACE) {

                        std::string tempText = lines[line].text;
                        if (column > 0) {

                            lines[line].text = tempText.substr(0, column - 1) +tempText.substr(column);
                            lines[line].needsUpdate = true;
                            update_cursor_position(font, lines, column, line,cursordst, marginX.Value, -1,0, showCursor, lineSpacingPercentage);
                            
                        } else {

                            if (line > 0 && line < lines.size()) {

                                column = lines[line-1].text.size();
                                lines[line-1].text += lines[line].text;
                                lines.erase(lines.begin() + line);
                                lines[line-1].needsUpdate = true;
                                update_cursor_position(font, lines, column, line, cursordst,marginX.Value, 0, -1, showCursor, lineSpacingPercentage);
                                
                            }
                        }
                        showCursor = true;
                    }


                    if (e.key.keysym.sym == SDLK_RETURN) {

                        lineObj emptyline;
                        mylog("Enter key was pressed");
                        lines.insert(lines.begin() + line, emptyline);
                        update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 0, 1, showCursor, lineSpacingPercentage);
                    }
                
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (SDL_PointInRect(&Mouse,&typingArea)) {
                        getcurrenttext = true;
                        typing = true;
                    } else {
                        typing = false;
                    }

                    break;
                default:
                    break;
            }
            // Call handleEvent for each slider
            marginX.handleEvent(e);
            marginY.handleEvent(e);
            fontsize.handleEvent(e);
            scroll.handleEvent(e);
            lineSpacing.handleEvent(e);
        }

        // Update slider values based on dragging
        marginX.update();
        marginY.update();
        fontsize.update();
        scroll.update();
        lineSpacing.update();


        scrollY = (scroll.Value / 100.0) * (totallineheight + marginY.Value);

        SDL_SetRenderDrawColor(ren, 71, 76, 84, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderFillRect(ren, &typingArea);

        //loop over all lines
        for (size_t i = 0; i < lines.size(); i++) {
            // physical position of line at the bottoù
            int line_y = i*lineSpacingPercentage + marginY.Value - scrollY;

            if (marginY.Value-5 <= line_y - lines[i].height && line_y <= winH ) {

                if (lines[i].needsUpdate) {
                    update_single_line_texture(font, ren, lines, i);
                    lines[i].needsUpdate = false;
                }
                
                SDL_Rect dstLine = {marginX.Value, line_y - lines[i].height, lines[i].width, lines[i].height};
                SDL_RenderCopy(ren, lines[i].texture, NULL, &dstLine);
            }


            // If I clicked somewhere in the typing area
            if (mouseY < line_y && getcurrenttext) {
                showCursor = true;
                int cursorX;
                int mouseRelativeX = mouseX - marginX.Value;
                line = i; // "i" and "line" represent current line row

                if (lines[i].width <= mouseRelativeX) 
                {
                    int tempWidth = 0;
                    TTF_SizeUTF8(font, lines[i].text.c_str(), &tempWidth, nullptr);
                    column = lines[i].text.size() ;
                    cursorX = marginX.Value + tempWidth;
                }
                else if ( mouseRelativeX <= 0) {

                    column = 0;
                    cursorX = marginX.Value;
                } else {
                    for (size_t j = 0; j <= lines[i].text.size(); j++) {
                        int tempWidth = 0;
                        std::string substr = lines[i].text.substr(0, j);
                        // get the width of the the current subset of chars
                        TTF_SizeUTF8(font, substr.c_str(), &tempWidth, nullptr);
                        if (mouseRelativeX < tempWidth) {
                            column = j-1 ;
                            substr = lines[i].text.substr(0, j-1);
                            TTF_SizeUTF8(font, substr.c_str(), &tempWidth, nullptr);
                            cursorX = marginX.Value + tempWidth;
                            break;
                        }
                    }
                }
                cursordst = {cursorX, line_y - fontsize.Value*12/10, fontsize.Value/10, fontsize.Value};
                getcurrenttext = false;
            }
        }

        // render the sliders
        marginX.draw(ren, settingsfont);
        marginY.draw(ren, settingsfont);
        fontsize.draw(ren, settingsfont);
        scroll.draw(ren, settingsfont);
        lineSpacing.draw(ren, settingsfont);

        // check if the font size changed
        if (fontsize.dragging) {
            font = fontCache[fontsize.Value - fontsize.minValue];
            ClearVector(lines);
        }

        // check if the linespacing changed
        if (lineSpacing.dragging || fontsize.dragging) {
            lineSpacingPercentage = fontsize.Value * lineSpacing.Value / 100.0;
            totallineheight = lines.size()*lineSpacingPercentage;
        }

        // calculate the time of execution of each frame
        Uint32 now = SDL_GetTicks();
        if (now - lastToggle >= blinkInterval) {
            showCursor = !showCursor;
            lastToggle = now;
        }

        // Render cursor flicker
        if (showCursor && typing) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderFillRect(ren, &cursordst);
        }

        // Presnet everything at last
        SDL_RenderPresent(ren);
    }



    // Clean everything
    SDL_StopTextInput();
    for (size_t i = 0; i < fontCache.size(); i++) {
        TTF_CloseFont(fontCache[i]);
    }
    fontCache.clear();
    //save up closing
    std::filesystem::path path(filePath);  // filePath is std::wstring
    std::ofstream out(path);     
    if (!out.is_open()) {
        std::wcerr << L"Failed to open file: " << filePath << std::endl;
        return 1;
    }
    for (const auto& line : lines) {
        out << line.text << "\n";  // ✅ valid: line.text is std::string
        SDL_DestroyTexture(line.texture);
    }
    lines.clear();

    out.close();

    TTF_CloseFont(settingsfont);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// Open a file
std::wstring OpenFile(std::vector<lineObj>& lines , TTF_Font * font,SDL_Renderer * ren) {

    SDL_Color Black = {0, 0, 0, 255};

    std::vector<lineObj>().swap(lines);

    auto res = OpenFileDialog();

    std::wcout << res.filePath << std::endl;

    std::ifstream inputFile(res.filePath.c_str());

    std::string fileLine;
    if (!inputFile.is_open()) {
    std::wcerr << L"Failed to open file: " << res.filePath << std::endl;
    return L""; // or handle the error
    }
    while (std::getline(inputFile, fileLine)) {

        lineObj newLine;
        newLine.text = fileLine;

        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, newLine.text.c_str(), Black);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        SDL_FreeSurface(surface);

        newLine.texture = texture;
        int height, width;
        TTF_SizeUTF8(font, newLine.text.c_str(), &width, &height);

        newLine.height = height;

        newLine.width = width;

        lines.push_back(newLine);
        
    }
    inputFile.close();
    return res.filePath;
}
