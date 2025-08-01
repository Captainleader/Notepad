#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
template<typename T> 
inline void mylog(T text) {
    std::cout << text << std::endl;
}


//Line class containign text / texture / height / width
class lineObj {
    public:
        std::string text;
        SDL_Texture* texture;
        int height;
        int width;
        bool needsUpdate = true;
    };

//Handle all sliders logic

//Slider/
class Slider {
public:
    int minValue, maxValue;
    int x, y, w, h;
    int Value;
    bool dragging;

    SDL_Color gray = {34, 53, 66, 255};
    SDL_Color blue = {37, 160, 245, 255};
    SDL_Color white = {221, 227, 237, 255};

    Slider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : x(x_), y(y_), w(w_), h(h_), minValue(minVal), maxValue(maxVal), Value(initialVal), dragging(false) {
        // Ensure initial value is within range
        if (Value < minValue) Value = minValue;
        if (Value > maxValue) Value = maxValue;
    }
 //I don't understand what that is
    virtual ~Slider() {}

    virtual void handleEvent(SDL_Event& e) = 0; 
    virtual void update() = 0; 
    virtual void draw(SDL_Renderer* ren, TTF_Font* font) = 0; 
};

// Horizontal Slider class x, y, w, h, min, max, initialValue
class HorizontalSlider : public Slider {
public:
    HorizontalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal) {}

    void handleEvent(SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            // Check if click is within the horizontal track area (allowing for thumb size)
            if ((x + 5 <= mouseX && mouseX <= x + w - 5) && // Adjusted for visual line
                (y <= mouseY && mouseY <= y + h)) {
                dragging = true;
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            dragging = false;
        }
    }

    void update() override {
        if (dragging) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Calculate the position relative to the slider's track
            // The effective track width for the thumb's center is (w - 10) if thumb is 10 wide
            int trackWidth = w - 10; // Assuming thumb width is 10 (5 on each side of the center)
            int clampedX = std::max(x + 5, std::min(mouseX, x + 5 + trackWidth)); // Clamp mouseX within the track limits

            // Calculate Value based on mouse position
            // The value is mapped from the clampedX position to the min/max value range
            Value = minValue + (clampedX - (x + 5)) * (maxValue - minValue) / trackWidth;

            // Ensure Value stays within bounds
            if (Value < minValue) Value = minValue;
            if (Value > maxValue) Value = maxValue;
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font) override {
        SDL_Rect boxdst = {x, y, w, h};
        SDL_Rect linedst = {x + 5, y + h / 2 - 5, w - 10, 10}; // Horizontal line

        // Calculate thumb position
        double value_percent = (double)(Value - minValue) / (maxValue - minValue);
        int thumbX = x + 5 + static_cast<int>((w - 10) * value_percent) - 5; // thumb width is 10, so -5 to center it
        SDL_Rect thumbdst = {thumbX, y + h / 2 - 10, 10, 20}; // Thumb rectangle

        SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
        SDL_RenderFillRect(ren, &boxdst);
        SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
        SDL_RenderFillRect(ren, &linedst);
        SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(ren, &thumbdst);

        // Render value text
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, std::to_string(Value).c_str(), white);
        if (!surface) {
            // Handle error: TTF_RenderUTF8_Blended failed
            return;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        if (!texture) {
            // Handle error: SDL_CreateTextureFromSurface failed
            SDL_FreeSurface(surface);
            return;
        }
        SDL_FreeSurface(surface);

        int textWidth, textHeight;
        TTF_SizeUTF8(font, std::to_string(Value).c_str(), &textWidth, &textHeight);
        SDL_Rect dsttext = {x + w + 10, y + h / 2 - textHeight / 2, textWidth, textHeight}; // Position text to the right
        SDL_RenderCopy(ren, texture, NULL, &dsttext);
        SDL_DestroyTexture(texture);
    }
};

// Vertical Slider class
class VerticalSlider : public Slider {
public:
    VerticalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal) {}

    void handleEvent(SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            // Check if click is within the vertical track area
            if ((x <= mouseX && mouseX <= x + w) &&
                (y + 5 <= mouseY && mouseY <= y + h - 5)) { // Adjusted for visual line
                dragging = true;
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            dragging = false;
        }
    }

    void update() override {
        if (dragging) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Calculate the position relative to the slider's track
            int trackHeight = h - 10; // Assuming thumb height is 10
            int clampedY = std::max(y + 5, std::min(mouseY, y + 5 + trackHeight));

            // Calculate Value based on mouse position (inverted for vertical: higher Y means lower value usually)
            // Or, if higher Y means higher value, keep it as is. Let's assume higher Y means lower value for typical vertical sliders.
            // If you want higher Y to mean higher value, change clampedY to (y + 5 + trackHeight - clampedY) in the calculation.
            Value = minValue + (clampedY - (y + 5)) * (maxValue - minValue) / trackHeight;
            // For a typical vertical slider where top is min and bottom is max, you might do:
            // Value = maxValue - ((clampedY - (y + 5)) * (maxValue - minValue) / trackHeight);

            // Ensure Value stays within bounds
            if (Value < minValue) Value = minValue;
            if (Value > maxValue) Value = maxValue;
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font) override {
        SDL_Rect boxdst = {x, y, w, h};
        SDL_Rect linedst = {x + w / 2 - 5, y + 5, 10, h - 10}; // Vertical line

        // Calculate thumb position
        double value_percent = (double)(Value - minValue) / (maxValue - minValue);
        int thumbY = y + 5 + static_cast<int>((h - 10) * value_percent) - 5; // thumb height is 10, so -5 to center it
        SDL_Rect thumbdst = {x + w / 2 - 10, thumbY, 20, 10}; // Thumb rectangle

        SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
        SDL_RenderFillRect(ren, &boxdst);
        SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
        SDL_RenderFillRect(ren, &linedst);
        SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(ren, &thumbdst);

        // Render value text
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, std::to_string(Value).c_str(), white);
        if (!surface) {
            // Handle error: TTF_RenderUTF8_Blended failed
            return;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        if (!texture) {
            // Handle error: SDL_CreateTextureFromSurface failed
            SDL_FreeSurface(surface);
            return;
        }
        SDL_FreeSurface(surface);

        int textWidth, textHeight;
        TTF_SizeUTF8(font, std::to_string(Value).c_str(), &textWidth, &textHeight);
        SDL_Rect dsttext = {x + w / 2 - textWidth / 2, y + h + 10, textWidth, textHeight}; // Position text below
        SDL_RenderCopy(ren, texture, NULL, &dsttext);
        SDL_DestroyTexture(texture);
    }
};
/*********************************************END OF SLIDER LOGIC********************************************** */



// only updates the information of one line call updatetexture() to handle all lines
inline void update_single_line_texture(TTF_Font* font, SDL_Renderer* ren, std::vector<lineObj>& lines, int line) {
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


inline void update_cursor_position(TTF_Font * font, std::vector<lineObj>& lines,
     int & column, int & line, SDL_Rect &cursor,
     int margin, int moveX, int moveY, bool &showCursor,
      int lineSpacingPercentage) {
    // 'i' is the current cursor position in a line
    // 'move' is how many chars it moves 1 , 2 , -1 etc...
    // 'line' is the line row
    int w,h;
    if (moveX != 0) {
        if (moveX > 0) {

            if (column == lines[line].text.size()) {
                line += 1 ;
                column = 0;
                w = 0;
                cursor.y += lineSpacingPercentage;
            } else if ( column < lines[line].text.size()) {
                column += moveX;
                TTF_SizeUTF8(font, lines[line].text.substr(0,column).c_str() , &w , nullptr);
            }
        } else {
            
            if (column == 0) {
                cursor.y -= lineSpacingPercentage;
                line -= 1 ;
                column = lines[line].text.size();
                TTF_SizeUTF8(font, lines[line].text.c_str() , &w , nullptr);
            } else if (column > 0) {
                column += moveX; // because moveX is already negative lmao
                TTF_SizeUTF8(font, lines[line].text.substr(0,column).c_str() , &w , nullptr);
            }
        }
        cursor.x = w + margin;
    }
    if (moveY != 0) {
        if (moveY > 0 && line < lines.size() ) {
            line += 1;
            cursor.y += lineSpacingPercentage;
        } else if ( moveY < 0 && line > 0) {
            line -= 1;
            cursor.y -= lineSpacingPercentage;
        } 
        

    }

    showCursor = true;
}

// Clear the lines vector

inline void ClearVector(std::vector<lineObj>& lines) {
    for (auto& line : lines) {
        line.needsUpdate = true;
    }
}


// A button struct

struct Button
{
    SDL_Renderer *ren;
    SDL_Rect box;
    
    SDL_Rect title_box;
    std::string title;
    bool isClicked = false;
    SDL_Texture *texture;
    TTF_Font *font;
    SDL_Rect shadow_box;
    Button(SDL_Renderer *ren2, TTF_Font *font2, SDL_Rect mybox, std::string text){
        box = mybox; title = text; ren = ren2; font = font2;
        SDL_Surface * surface =  TTF_RenderUTF8_Blended(font, title.c_str(), {255, 255, 255, 255} );
        
        texture = SDL_CreateTextureFromSurface(ren,surface);
        title_box.h = surface->h;  title_box.w = surface->w;
        title_box.x = box.x + (box.w - title_box.w )/2;
        title_box.y = box.y + (box.h - title_box.h )/2;

        shadow_box = box;
        shadow_box.w += 2;
        shadow_box.h += 2;
    }
    
    SDL_Color color = {76, 175, 80, 255};
    

    void draw() {
        SDL_SetRenderDrawColor(ren, 55, 125, 72, 255);
        SDL_RenderFillRect(ren, &shadow_box);

        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(ren, &box); 
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderCopy(ren, texture, NULL, &title_box);  
    }
    void click_event(SDL_Event & e){
    
        SDL_Point Mouse = {e.button.x, e.button.y};
        if (SDL_PointInRect(&Mouse,&box) && e.type == SDL_MOUSEBUTTONDOWN) {

            color = {69, 151, 160, 255};
            isClicked = true;
            

        } else if (e.type == SDL_MOUSEBUTTONUP) {

            color = {76, 175, 80, 255};
        } 
    } 

    
};
