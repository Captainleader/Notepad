#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <filesystem>
#include "winAPI_stuff.hpp"
#include "editor_core.hpp"
#include <utility> 

template<typename T> 
inline void mylog(T text) {
    std::cout << text << std::endl;
}




//Handle all sliders logic

//Slider/
Slider::Slider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
    : x(x_), y(y_), w(w_), h(h_), minValue(minVal), maxValue(maxVal), Value(initialVal), dragging(false) {
    if (Value < minValue) Value = minValue;
    if (Value > maxValue) Value = maxValue;
}

Slider::~Slider() {}

// HorizontalSlider implementation
HorizontalSlider::HorizontalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
    : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal) {}

void HorizontalSlider::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = e.button.x;
        int mouseY = e.button.y;

        if ((x + 5 <= mouseX && mouseX <= x + w - 5) &&
            (y <= mouseY && mouseY <= y + h)) {
            dragging = true;
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        dragging = false;
    }
}

void HorizontalSlider::update() {
    if (dragging) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int trackWidth = w - 10;
        int clampedX = std::max(x + 5, std::min(mouseX, x + 5 + trackWidth));

        Value = minValue + (clampedX - (x + 5)) * (maxValue - minValue) / trackWidth;

        if (Value < minValue) Value = minValue;
        if (Value > maxValue) Value = maxValue;
    }
}

void HorizontalSlider::draw(SDL_Renderer* ren, TTF_Font* font) {
    SDL_Rect boxdst = {x, y, w, h};
    SDL_Rect linedst = {x + 5, y + h / 2 - 5, w - 10, 10};

    double value_percent = (double)(Value - minValue) / (maxValue - minValue);
    int thumbX = x + 5 + static_cast<int>((w - 10) * value_percent) - 5;
    SDL_Rect thumbdst = {thumbX, y + h / 2 - 10, 10, 20};

    SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
    SDL_RenderFillRect(ren, &boxdst);
    SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
    SDL_RenderFillRect(ren, &linedst);
    SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
    SDL_RenderFillRect(ren, &thumbdst);

    std::string valStr = std::to_string(Value);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, valStr.c_str(), white);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textWidth, textHeight;
    TTF_SizeUTF8(font, valStr.c_str(), &textWidth, &textHeight);
    SDL_Rect dsttext = {x + w + 10, y + h / 2 - textHeight / 2, textWidth, textHeight};
    SDL_RenderCopy(ren, texture, NULL, &dsttext);
    SDL_DestroyTexture(texture);
}

// VerticalSlider implementation
VerticalSlider::VerticalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
    : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal) {}

void VerticalSlider::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = e.button.x;
        int mouseY = e.button.y;

        if ((x <= mouseX && mouseX <= x + w) &&
            (y + 5 <= mouseY && mouseY <= y + h - 5)) {
            dragging = true;
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        dragging = false;
    }
}

void VerticalSlider::update() {
    if (dragging) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int trackHeight = h - 10;
        int clampedY = std::max(y + 5, std::min(mouseY, y + 5 + trackHeight));

        Value = minValue + (clampedY - (y + 5)) * (maxValue - minValue) / trackHeight;

        if (Value < minValue) Value = minValue;
        if (Value > maxValue) Value = maxValue;
    }
}

void VerticalSlider::draw(SDL_Renderer* ren, TTF_Font* font) {
    SDL_Rect boxdst = {x, y, w, h};
    SDL_Rect linedst = {x + w / 2 - 5, y + 5, 10, h - 10};

    double value_percent = (double)(Value - minValue) / (maxValue - minValue);
    int thumbY = y + 5 + static_cast<int>((h - 10) * value_percent) - 5;
    SDL_Rect thumbdst = {x + w / 2 - 10, thumbY, 20, 10};

    SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
    SDL_RenderFillRect(ren, &boxdst);
    SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
    SDL_RenderFillRect(ren, &linedst);
    SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
    SDL_RenderFillRect(ren, &thumbdst);

    std::string valStr = std::to_string(Value);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, valStr.c_str(), white);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textWidth, textHeight;
    TTF_SizeUTF8(font, valStr.c_str(), &textWidth, &textHeight);
    SDL_Rect dsttext = {x + w / 2 - textWidth / 2, y + h + 10, textWidth, textHeight};
    SDL_RenderCopy(ren, texture, NULL, &dsttext);
    SDL_DestroyTexture(texture);
}/*********************************************END OF SLIDER LOGIC********************************************** */



// only updates the information of one line call updatetexture() to handle all lines
void SetLineTexture(TTF_Font* font, SDL_Renderer* ren, std::vector<lineObj>& lines, int line) {
    SDL_Color Black = {0, 0, 0, 255}; 
    if (!lines[line].text.empty())
    {
        SDL_DestroyTexture(lines[line].texture);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, lines[line].text.c_str(), Black);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        lines[line].texture = texture;
        lines[line].width = surface->w;
        lines[line].height =surface->h;
        SDL_FreeSurface(surface);
    } else {
        SDL_DestroyTexture(lines[line].texture);
        lines[line].texture = nullptr; 
        lines[line].width = 0 ;
        lines[line].height = 0 ;
    }

}

void update_cursor_position(TTF_Font *font, std::vector<lineObj> &lines, int &column, int &line, SDL_Rect &cursor, int margin, int moveX, int moveY, bool &showCursor, int lineHeight, int margin_y, int scroll) {
    int w, h;
    if (moveX != 0) {
        if (moveX > 0) {
            if (column == lines[line].text.size()) {
                line += 1;
                column = 0;
                w = 0;
                cursor.y += lineHeight;
            } else if (column < lines[line].text.size()) {
                column += moveX;
                TTF_SizeUTF8(font, lines[line].text.substr(0, column).c_str(), &w, nullptr);
            }
        } else {
            if (column == 0) {
                cursor.y -= lineHeight;
                line -= 1;
                column = lines[line].text.size();
                TTF_SizeUTF8(font, lines[line].text.c_str(), &w, nullptr);
            } else if (column > 0) {
                column += moveX;  // because moveX is already negative lmao
                TTF_SizeUTF8(font, lines[line].text.substr(0, column).c_str(), &w, nullptr);
            }
        }
        cursor.x = w + margin;
    }
    if (moveY != 0) {
        int newLine = line + moveY;
        if (newLine >= 0 && newLine < (int)lines.size()) {
            line = newLine;
            // Calculate cursor.y explicitly instead of +=/-=
            cursor.y = margin_y + line * lineHeight * -scroll;
        }
    }

    showCursor = true;
}

void ClearLines(std::vector<lineObj>& lines) {
    for (auto& line : lines) {
        line.needsUpdate = true;
    }
}

Button::Button(SDL_Renderer *ren2, TTF_Font *font2, SDL_Rect mybox, std::string text)
    : ren(ren2), font(font2), box(mybox), title(text)
{
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, title.c_str(), {255, 255, 255, 255});
    texture = SDL_CreateTextureFromSurface(ren, surface);
    title_box = {
        box.x + (box.w - surface->w) / 2,
        box.y + (box.h - surface->h) / 2,
        surface->w, surface->h
    };
    shadow_box = {box.x, box.y, box.w + 2, box.h + 2};
    SDL_FreeSurface(surface);
}

void Button::draw() {
    SDL_SetRenderDrawColor(ren, 55, 125, 72, 255);
    SDL_RenderFillRect(ren, &shadow_box);
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(ren, &box);
    SDL_RenderCopy(ren, texture, NULL, &title_box);
}

void Button::click_event(SDL_Event &e) {
    SDL_Point mouse = {e.button.x, e.button.y};
    if (SDL_PointInRect(&mouse, &box) && e.type == SDL_MOUSEBUTTONDOWN) {
        color = {69, 151, 160, 255};
        isClicked = true;
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        color = {76, 175, 80, 255};
    }
}

std::wstring OpenFile(std::vector<lineObj>& lines, TTF_Font* font, SDL_Renderer* ren) {
    SDL_Color Black = {0, 0, 0, 255};
    std::vector<lineObj>().swap(lines);
    auto res = OpenFileDialog();
    std::wcout << res.filePath << std::endl;
    std::ifstream inputFile(res.filePath.c_str());
    std::string fileLine;
    if (!inputFile.is_open()) {
        std::wcerr << L"Failed to open file: " << res.filePath << std::endl;
        return L"";  // or handle the error
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

void Set_CursorPos(TextLayoutContext& tx, int margin_x, int margin_y, int x, int y) { update_cursor_position(tx.font, tx.lines, tx.column, tx.line, tx.cursordst, margin_x, x, y, tx.showCursor, tx.lineHeight, margin_y, tx.scrollY); }
// Handle events ***********************************************************************************************************************************************
void EnterKeyEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y) {
    if (e.key.keysym.sym == SDLK_RETURN) {
        lineObj newline;
        std::string remainder = tx.lines[tx.line].text.substr(tx.column);
        tx.lines[tx.line].text = tx.lines[tx.line].text.substr(0, tx.column);
        tx.lines[tx.line].needsUpdate = true;
        newline.texture = nullptr;  // DO NOT REMOVE THISSSS
        newline.needsUpdate = true;  // DO NOT REMOVE THISSSS

        if (tx.line + 1 < tx.lines.size()) {
            tx.lines.insert(tx.lines.begin() + tx.line + 1, newline);
        } else {
            tx.lines.push_back(newline);
        }
        
        tx.lines[tx.line + 1].text = remainder;
        tx.lines[tx.line + 1].needsUpdate = true;
        tx.line += 1;
        tx.cursordst.y =  tx.line*tx.lineHeight + margin_y - tx.scrollY;
        tx.column = 0;
        tx.cursordst.x = margin_x;
        tx.showCursor = true;
    }
}

void BackSpaceKeyEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y) {
    if (e.key.keysym.sym == SDLK_BACKSPACE) {
        std::string tempText = tx.lines[tx.line].text;
        if (tx.column > 0) {
            tx.lines[tx.line].text = tempText.substr(0, tx.column - 1) + tempText.substr(tx.column);
            tx.lines[tx.line].needsUpdate = true;
            tx.column -= 1;
            TTF_SizeUTF8(tx.font,tempText.substr(0, tx.column).c_str(),&tx.cursordst.x,nullptr);
            tx.cursordst.x += margin_x ;

        } else {
            if (tx.line > 0 && tx.line < tx.lines.size()) {

                tx.column = tx.lines[tx.line - 1].text.size();
                TTF_SizeText(tx.font, tx.lines[tx.line - 1].text.c_str(), &tx.cursordst.x, nullptr);
                tx.cursordst.x += margin_x;
                tx.lines[tx.line - 1].text += tx.lines[tx.line].text;
                tx.lines.erase(tx.lines.begin() + tx.line);
                tx.lines[tx.line - 1].needsUpdate = true;
                tx.line -= 1;
                tx.cursordst.y =  tx.line*tx.lineHeight + margin_y - tx.scrollY;
            }
        }
        tx.showCursor = true;
    }
}

void ArrowKeysEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y) {
    if (e.key.keysym.sym == SDLK_LEFT && (tx.line != 0 || tx.column != 0)) {
        Set_CursorPos(tx, margin_x, margin_y, -1, 0);
    } else if (e.key.keysym.sym == SDLK_RIGHT && (tx.line < tx.lines.size() || tx.column < tx.lines[tx.line].text.size())) {
        Set_CursorPos(tx, margin_x, margin_y, 1, 0);
    }

    else if (e.key.keysym.sym == SDLK_UP && (tx.line > 0)) {
        std::cout << "Upper key clicked";
    }
}

void TextInputEvent(SDL_Event e, TextLayoutContext& tx,bool typing,int margin_x,int margin_y) {
    if (e.type == SDL_TEXTINPUT && typing) {
        tx.lines[tx.line].text.insert(tx.column, e.text.text);
        tx.lines[tx.line].needsUpdate = true;
        Set_CursorPos(tx,margin_x,margin_y,1,0);
    } 
}
/*void LoadFontCache(TextLayoutContext& tx){
    
    //Set up a tx.font cache to lower
    std::string fontpath ="c:/windows/fonts/calibri.TTf"; 
    for (size_t i = tx.fontsize.minValue; i < tx.fontsize.maxValue+1; i++) {
        TTF_Font* font = TTF_OpenFont(fontpath.c_str(), i);
        if (!font) {
            std::cout << "Failed to open font: " << TTF_GetError() << "\n";
            return 1;
        }
        tx.fontCache.push_back(font);
    }

    tx.font = tx.fontCache[8];
}*/
std::pair<int, int> getLineAndColumn(TextLayoutContext& tx, int margin_y,int margin_x){

    int line = (tx.mouseY+tx.scrollY-margin_y)/tx.lineHeight;//get the line index corresponding to my mouse Y
    std::string temptxt = tx.lines[line].text; //get the line corresponding to my mouse Y
    int column = 0;
    int tempW;
    while (column < temptxt.size()) {
        TTF_SizeUTF8(tx.font,temptxt.substr(0,column).c_str(),&tempW,NULL);
        if (tx.mouseX <  tempW+margin_x) {
            break;
        }
        column++;
    }
    if (column < 0 ) column = 0;
    return {line, column};
}
