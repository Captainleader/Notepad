
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "editor_core.hpp"
#include <utility> 
#include <algorithm>
#include <cmath>

int main(int argc, char** argv) {
    // Initialize SDL and TTF
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    // Initialize window and renderer
    SDL_Window* win = SDL_CreateWindow("Notepad", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 1600, 800,SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int winH, winW;
    SDL_GetWindowSize(win, &winW, &winH);
    std::pair<int, int> StartSelection,EndSelection;

    // Initialize sliders
    HorizontalSlider marginX(0, 0, 150, 40, 0, 200, 40);
    HorizontalSlider marginY(200, 0, 150, 40, 0, 200, 70);
    HorizontalSlider fontsize(400, 0, 150, 40, 8, 72, 18);
    HorizontalSlider linespacing(600, 0, 150, 40, 100, 200, 120);
    VerticalSlider scroll(winW - 40, 0, 40, winH, 0, 10000, 0);
    // This is were almost all the "global" variables are 
    TextLayoutContext tx;
    //Set up a tx.font cache to lower
    std::string fontpath ="GoogleSansCode-VariableFont_wght.ttf"; 
    for (size_t i = fontsize.minValue; i < fontsize.maxValue+1; i++) {
        TTF_Font* font = TTF_OpenFont(fontpath.c_str(), i);
        if (!font) {
            std::cout << "Failed to open font: " << TTF_GetError() << "\n";
            return 1;
        }
        tx.fontCache.push_back(font);
    }

    tx.font = tx.fontCache[8];

    TTF_Font* settingsfont = TTF_OpenFont("c:/windows/fonts/arial.TTf", 14);
    if (!settingsfont) {
        std::cout << "Failed to open settingsfont: " << TTF_GetError() << "\n";
        return 1;
    }
    // Initialize buttons
    Button openbutton(ren, settingsfont, {800,0,100,40}, "OpenFile");
    Button savebutton(ren, settingsfont, {920,0,100,40}, "Save");
    // Load a file --------------------------------------------
    std::wstring filePath;
    scroll.Value = scroll.minValue;
    openbutton.draw();
    filePath = OpenFile(tx.lines,tx.font,ren);
    // Initialize cursor logic
    Uint32 lastToggle = SDL_GetTicks();
    const Uint32 blinkInterval = 500;  // milliseconds
    std::string currenttext;
    SDL_StartTextInput();
    SDL_Rect typingArea = {marginX.Value, marginY.Value, winW, winH};
    // Main loop

    bool typing = true;
    bool quit = false;
    SDL_Event e;
    while (!quit) {

        Uint32 frameStart = SDL_GetTicks();     
        SDL_GetWindowSize(win, &winW, &winH);
        Uint32 mouseState =  SDL_GetMouseState(&tx.mouseX, &tx.mouseY);
        tx.Mouse = {tx.mouseX, tx.mouseY};
        scroll.x = winW - 40; // scroll slider x position
        scroll.h = winH;
        typingArea = {marginX.Value, marginY.Value, winW-40-marginX.Value, winH};

        while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                ArrowKeysEvent(e, tx, marginX.Value, marginY.Value);
                BackSpaceKeyEvent(e, tx, marginX.Value, marginY.Value);
                EnterKeyEvent(e, tx, marginX.Value, marginY.Value);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (SDL_PointInRect(&tx.Mouse, &typingArea)) {
                    //tx.getcurrenttext = true;
                    
                    tx.selection = false;
                    StartSelection = getLineAndColumn(tx,marginY.Value,marginX.Value) ;
                    std::cout << "start selection" << std::endl;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (SDL_PointInRect(&tx.Mouse, &typingArea)) {
                    //tx.getcurrenttext = true;
                    tx.selection = false;
                    std::cout << "end selection" << std::endl;
                    std::cout << tx.lines[StartSelection.first].text.substr(0,StartSelection.second) << std::endl;
                    std::cout << tx.lines[EndSelection.first].text.substr(0,EndSelection.second) << std::endl;
                    //StartSelection = NULL;

                }
                break;
        }
        TextInputEvent(e, tx, typing, marginX.Value, marginY.Value);
        openbutton.click_event(e);
        savebutton.click_event(e);
        marginX.handleEvent(e);
        marginY.handleEvent(e);
        fontsize.handleEvent(e);
        scroll.handleEvent(e);
        linespacing.handleEvent(e);
        }


        if (mouseState & SDL_BUTTON_LMASK) {
            if (SDL_PointInRect(&tx.Mouse, &typingArea)) {
                tx.getcurrenttext = true;
                typing = true;
                tx.selection = true;
                    EndSelection = getLineAndColumn(tx,marginY.Value,marginX.Value) ;
                
            } else {
                typing = false;
            }
        }
            

        // Update slider values based on dragging
        marginX.update();
        marginY.update();
        fontsize.update();
        scroll.update();
        linespacing.update();




        // Check BUTTON ********************************************************************************************
        if (openbutton.isClicked) {
            scroll.Value = scroll.minValue;
            openbutton.draw();
            filePath = OpenFile(tx.lines,tx.font,ren);
            openbutton.isClicked = false;

        }
      if (savebutton.isClicked) {  

        std::filesystem::path path(filePath);  // filePath is std::wstring
        std::ofstream out(path);     
        if (!out.is_open()) {
            std::wcout << L"Failed to open file: " << filePath << std::endl;
            return 1;
        }
        for (const auto& ch : tx.lines) {
            out << ch.text << "\n";  // ✅ valid: line.text is std::string
        }

        out.close();

        savebutton.isClicked = false;
      }

      // RENDERING PART ***************************************************************************************************************************
        SDL_SetRenderDrawColor(ren, 71, 76, 84, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderFillRect(ren, &typingArea);
        tx.scrollY = (scroll.Value/10000.0)*(tx.lineHeight*tx.lines.size()+marginY.Value-winH);


        //loop over all tx.lines and render them i represent's the current line in the loop
        for (size_t i = 0; i < tx.lines.size(); i++) {

            int line_y = (i + 1) * tx.lineHeight + marginY.Value - tx.scrollY;

        if (marginY.Value - 10 <= line_y - tx.lines[i].height && line_y <= winH) {
            if (tx.lines[i].needsUpdate) {
                SetLineTexture(tx.font, ren, tx.lines, i);
                tx.lines[i].needsUpdate = false;
            }
            // Render line texture
            SDL_Rect dstLine = {marginX.Value, line_y - tx.lines[i].height, tx.lines[i].width, tx.lines[i].height};

            SDL_RenderCopy(ren, tx.lines[i].texture, NULL, &dstLine);
            if (tx.selection) {
                int startLine, endLine, startCol, endCol;
                if (StartSelection.first < EndSelection.first ) {
                    startLine = StartSelection.first;
                    startCol = StartSelection.second;
                    endLine = EndSelection.first;
                    endCol = EndSelection.second;

                } else if (StartSelection.first > EndSelection.first ) {
                    startLine = EndSelection.first;
                    startCol = EndSelection.second;
                    endLine = StartSelection.first;
                    endCol = StartSelection.second;

                } else if (StartSelection.first == EndSelection.first ) {
                   if (StartSelection.second<   EndSelection.second){
                    startCol = StartSelection.second; endCol = EndSelection.second;
                   }else {endCol = StartSelection.second; startCol = EndSelection.second;}
                }
                bool shouldDraw = false;
                SDL_Rect highlightBox;
                highlightBox.y = line_y - tx.lineHeight;
                highlightBox.h = tx.lineHeight;


                if (i == StartSelection.first && StartSelection.first == EndSelection.first) {
                    TTF_SizeUTF8(tx.font,tx.lines[i].text.substr(0,startCol).c_str(),&highlightBox.x,NULL);
                    TTF_SizeUTF8(tx.font,tx.lines[i].text.substr(0,endCol).c_str(),&highlightBox.w,NULL);
                    highlightBox.w -= highlightBox.x;
                    highlightBox.x += marginX.Value;
                        shouldDraw = true;
                } else if (i == startLine) {
                    TTF_SizeUTF8(tx.font,tx.lines[i].text.substr(0,startCol).c_str(),&highlightBox.x,NULL);
                    highlightBox.w = tx.lines[i].width - highlightBox.x;
                    highlightBox.x += marginX.Value;
                        shouldDraw = true;
                } else if (i > startLine && i < endLine) {
                    highlightBox.x = marginX.Value;
                    highlightBox.w = tx.lines[i].width;
                        shouldDraw = true;
                } else if (i == endLine) {
                    highlightBox.x = marginX.Value;
                    TTF_SizeUTF8(tx.font,tx.lines[i].text.substr(0,endCol).c_str(),&highlightBox.w,NULL);
                        shouldDraw = true;
                }

                if ( shouldDraw) {
                SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(ren, 100, 100, 255, 100); // RGBA
                SDL_RenderFillRect(ren, &highlightBox);
                }  
            }

        }
        }

            if(tx.getcurrenttext) {
                std::pair<int, int> cursorPos = getLineAndColumn(tx,marginY.Value,marginX.Value);
                int cursorX;
                TTF_SizeUTF8(tx.font,tx.lines[cursorPos.first].text.substr(0,cursorPos.second).c_str(), &cursorX,NULL);
                cursorX+= marginX.Value;
                int CursorY = (cursorPos.first+1) * tx.lineHeight + marginY.Value - tx.scrollY;
                int CursorH = fontsize.Value * 12 / 10;
                tx.cursordst = {cursorX, CursorY - CursorH, 1, CursorH};
                tx.getcurrenttext = false;
            }

        // render the sliders SLIDER PART***************************************************************************
        marginX.draw(ren, settingsfont);
        marginY.draw(ren, settingsfont);
        fontsize.draw(ren, settingsfont);
        scroll.draw(ren, settingsfont);
        linespacing.draw(ren, settingsfont);
        openbutton.draw();
        savebutton.draw();

        // check if the font size changed
        if (fontsize.dragging) {
            tx.font = tx.fontCache[fontsize.Value - fontsize.minValue];
            ClearLines(tx.lines);
        }
        if (linespacing.dragging || fontsize.dragging) { tx.lineHeight  = fontsize.Value * linespacing.Value / 100.0; } // check if the tx.linespacing changed
        if (scroll.dragging) { tx.showCursor = false; } // check if the SCROLLING changed
        // calculate the time of execution of each frame
        Uint32 now = SDL_GetTicks();
        if (now - lastToggle >= blinkInterval) {
            tx.showCursor = !tx.showCursor;
            lastToggle = now;
        }
        // Render cursor flicker
        if (tx.showCursor && typing) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderFillRect(ren, &tx.cursordst);
        }
        SDL_RenderPresent(ren);   

    }
// CLOSING PART *******************************************************************************************************************************************
// Clean everything
SDL_StopTextInput();
for (size_t i = 0; i < tx.fontCache.size(); i++) {
    TTF_CloseFont(tx.fontCache[i]);
}
tx.fontCache.clear();
// save up closing
std::filesystem::path path(filePath);  // filePath is std::wstring
std::ofstream out(path);
if (!out.is_open()) {
    std::wcout << L"Failed to open file: " << filePath << std::endl;
    return 1;
}
for (const auto& ch : tx.lines) {
    out << ch.text << "\n";  // ✅ valid: line.text is std::string
    SDL_DestroyTexture(ch.texture);
}
tx.lines.clear();
out.close();
TTF_CloseFont(settingsfont);
SDL_DestroyRenderer(ren);
SDL_DestroyWindow(win);
TTF_Quit();
SDL_Quit();
return 0;
}
