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

//use mylog(text) to debug code


SDL_Color Black = {0, 0, 0, 255 };
int totallineheight = 0;    

int main(int argc, char ** argv) {


    // Initialize SDL and SDLTTF
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // Initialize window and renderer
    SDL_Window * win = SDL_CreateWindow("Notepad", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE);
    SDL_Renderer * ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int winH , winW;
    SDL_GetWindowSize(win, &winW, &winH);

    //Initialize sliders
    HorizontalSlider marginX(0, 0, 150, 40, 0, 200, 40);
    HorizontalSlider marginY(200, 0, 150, 40, 0, 200, 70);
    HorizontalSlider fontsize(400, 0, 150, 40, 8, 72, 18);
    HorizontalSlider lineSpacing(600, 0, 150, 40, 100, 200, 100);
    VerticalSlider scroll(winW-40, 0, 40, winH, 0, 100, 0);
    
    std::vector <TTF_Font*> fontCache;
    for (size_t i = fontsize.minValue; i < fontsize.maxValue; i++)
    {
        TTF_Font * font = TTF_OpenFont("c:/windows/fonts/consola.TTf", i);
        if (!font) {
        std::cerr << "Failed to open font: " << TTF_GetError() << "\n";
        return 1;    
        }
        fontCache.push_back(font);
    }
    
    TTF_Font * font = TTF_OpenFont("c:/windows/fonts/consola.TTf", 18);

    TTF_Font * settingsfont = TTF_OpenFont("c:/windows/fonts/arial.TTf", 14);
    if (!settingsfont) {
        std::cerr << "Failed to open settingsfont: " << TTF_GetError() << "\n";
        return 1;
    }





    SDL_Rect cursordst; // = {0, 0,0,0};
    OpenFileDialog();
    FileDialogResult myfile;
    // load a file
    std::vector<lineObj> lines;
    std::wifstream inputFile(myfile.filePath.c_str());
    std::wstring fileLine;
    while (std::getline(inputFile, fileLine)) {
        lineObj newLine;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string temptext = converter.to_bytes(fileLine);
        newLine.text = temptext;
        SDL_Surface * surface = TTF_RenderUTF8_Blended(font, newLine.text.c_str(), Black);
        SDL_Texture * texture = SDL_CreateTextureFromSurface(ren, surface);
        SDL_FreeSurface(surface);
        newLine.texture = texture; 
        int height, width;
        TTF_SizeUTF8(font, newLine.text.c_str(), & width, & height);
        newLine.height = height;
        totallineheight += height;
        newLine.width = width; 

        lines.push_back(newLine);
    }
    inputFile.close();


    // initialize cursor logic
    bool showCursor = true;
    Uint32 lastToggle = SDL_GetTicks();
    const Uint32 blinkInterval = 500; // milliseconds

    int scrollY;
    


    

    std::string mytext;





    bool getcurrenttext = true;
    int oldsize = 18;
    std::string currenttext;
    int line = 0;
    int column = 0;



    SDL_StartTextInput();
    bool quit = false;
    SDL_Event e;


    //Main loop
    bool typing = true;
    while (!quit) {

        Uint32 frameStart = SDL_GetTicks();

        int mouseX, mouseY;
        SDL_GetWindowSize(win, &winW, &winH);
        SDL_GetMouseState(&mouseX, &mouseY);
        scroll.x = winW-40;

        while (SDL_PollEvent( & e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_TEXTINPUT:{

                    lines[line].text.insert(column, e.text.text);
                    update_single_line_texture( font, ren, lines, line, lines[line].text);
                    update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 1, 0, showCursor, oldsize, lineSpacing.Value /100.0 );
                    break;}
                case SDL_KEYDOWN:

                    if (e.key.keysym.sym == SDLK_LEFT) {

                       update_cursor_position(font, lines, column, line, cursordst, marginX.Value, -1, 0, showCursor, oldsize,lineSpacing.Value /100.0 );                   
                    }

                    if (e.key.keysym.sym == SDLK_RIGHT) {

                        update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 1, 0, showCursor, oldsize,lineSpacing.Value /100.0 );
                    }

                    if (e.key.keysym.sym == SDLK_BACKSPACE ) {
                        std::string tempText = lines[line].text ;
                        if (!lines[line].text.empty()) {

                            lines[line].text = tempText.substr(0, column-1) + tempText.substr(column);
                            update_single_line_texture( font, ren, lines, line, lines[line].text);
                            update_cursor_position(font, lines, column, line, cursordst, marginX.Value, -1, 0,showCursor, oldsize,lineSpacing.Value /100.0 );

                        }else {

                            if (line >= 0 && line < lines.size()) {

                                lines.erase(lines.begin() + line); 
                                update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 0, -1,showCursor, oldsize,lineSpacing.Value /100.0 );
                                column = lines[line].text.size();
                            }
                            
                        }
                    if (e.key.keysym.sym == SDLK_RETURN ) {
                        lineObj emptyline ;
                        lines.insert(lines.begin() + line, emptyline);
                       update_cursor_position(font, lines, column, line, cursordst, marginX.Value, 0, 1,showCursor, oldsize,lineSpacing.Value /100.0 );
                    }    
                        
                        /*if (index >= 0 && index < vec.size()) {
                            vec.erase(vec.begin() + index);  // removes element at index 2 (30)
                        }*/
                    }
                    /*else if (e.key.keysym.sym == SDLK_RETURN)
                    {

                    }*/
                    break; 

                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (marginX.Value < mouseX && mouseX < winW - 40 && mouseY > marginY.Value && mouseY< winH ) {
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
        
        scrollY = (scroll.Value / 100.0)* (totallineheight+marginY.Value);
        
        SDL_SetRenderDrawColor(ren, 71, 76, 84, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_Rect typingArea = {marginX.Value-5,marginY.Value,winW,winH};
        SDL_RenderFillRect(ren, &typingArea);

        SDL_Color Black = {0, 0, 0, 255 };
        
         // refers to each line position in the loop
        for (size_t i = 0; i < lines.size(); i++) {
            int line_y = i*oldsize*lineSpacing.Value /100.0 + marginY.Value - scrollY ; 
            // line rendering condition
            if ( marginY.Value < line_y ) {

                SDL_Rect dstLine = {marginX.Value, line_y -lines[i].height, lines[i].width, lines[i].height};
                SDL_RenderCopy(ren, lines[i].texture, NULL, & dstLine);
            }


            if (line_y   > mouseY && getcurrenttext) {
                           
                
                int cursorX = marginX.Value;
                int textX = marginX.Value;
                int mouseRelativeX = mouseX - marginX.Value;

                int bestX = textX;
                int minDiff = INT_MAX;

                for (size_t j = 0; j <= lines[i].text.size(); j++) {
                    int tempWidth = 0;
                    std::string substr = lines[i].text.substr(0, j);
                    TTF_SizeUTF8(font, substr.c_str(), &tempWidth, nullptr);

                    int diff = abs(tempWidth - mouseRelativeX);
                    if (diff < minDiff) {
                        minDiff = diff;
                        bestX = textX + tempWidth;
                        column = j;
                    }
                }

                // Clamp the cursor to the line width
                int lineWidth = lines[i].width;
                line = i;
                if (mouseX > marginX.Value + lineWidth) {
                    cursorX = marginX.Value + lineWidth;
                    column = lines[i].text.size();
                } else if (mouseX < marginX.Value) {
                    cursorX = marginX.Value;
                    column = 0;
                } else {
                    cursorX = bestX;
                }
                cursordst = {cursorX, line_y-oldsize,2, oldsize};
                getcurrenttext = false;

            } 
        }
        // render the sliders 
        marginX.draw(ren, settingsfont);
        marginY.draw(ren, settingsfont);
        fontsize.draw(ren, settingsfont);
        scroll.draw(ren, settingsfont);
        lineSpacing.draw(ren, settingsfont);

        //check if the font size changed
        /*if (fontsize.dragging) {
            font = fontCache[fontsize.Value -fontsize.minValue]; 
        }
        */
        //calculate the time of execution of each frame
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


        SDL_RenderPresent(ren);


      
    }

    // Clean everything
    SDL_StopTextInput();
    TTF_CloseFont(font);
    TTF_CloseFont(settingsfont);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
