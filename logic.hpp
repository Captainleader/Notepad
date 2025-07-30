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

// Optimized line class with lazy texture loading
class lineObj {
public:
    std::string text;
    SDL_Texture* texture;
    int height;
    int width;
    bool textureValid; // Flag to track if texture needs regeneration
        SDL_Texture* lineNumberTexture = nullptr;
    int lineNumberWidth = 0;
    int lineNumberHeight = 0;
    
    lineObj() : texture(nullptr), height(0), width(0), textureValid(false) {}
    
    ~lineObj() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    
    // Copy constructor
    lineObj(const lineObj& other) : text(other.text), height(other.height), width(other.width), textureValid(false) {
        texture = nullptr; // Don't copy texture, will be regenerated when needed
    }
    
    // Assignment operator
    lineObj& operator=(const lineObj& other) {
        if (this != &other) {
            text = other.text;
            height = other.height;
            width = other.width;
            textureValid = false;
            if (texture) {
                SDL_DestroyTexture(texture);
            }
            texture = nullptr;
        }
        return *this;
    }
};



// Base slider class with improved performance
class Slider {
public:
    int minValue, maxValue;
    int x, y, w, h;
    int Value;
    bool dragging;
    bool needsRedraw; // Flag to avoid unnecessary redraws

    SDL_Color gray = {34, 53, 66, 255};
    SDL_Color blue = {37, 160, 245, 255};
    SDL_Color white = {221, 227, 237, 255};

    Slider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : x(x_), y(y_), w(w_), h(h_), minValue(minVal), maxValue(maxVal), Value(initialVal), dragging(false), needsRedraw(true) {
        if (Value < minValue) Value = minValue;
        if (Value > maxValue) Value = maxValue;
    }

    virtual ~Slider() {}

    virtual void handleEvent(SDL_Event& e) = 0;
    virtual void update() = 0;
    virtual void draw(SDL_Renderer* ren, TTF_Font* font) = 0;
};

// Optimized horizontal slider
class HorizontalSlider : public Slider {
private:
    int lastValue; // Cache to detect value changes
    
public:
    HorizontalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal), lastValue(initialVal) {}

    void handleEvent(SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            if ((x + 5 <= mouseX && mouseX <= x + w - 5) && 
                (y <= mouseY && mouseY <= y + h)) {
                dragging = true;
                needsRedraw = true;
                
                // Immediate value update for responsiveness
                int trackWidth = w - 10;
                int clampedX = std::max(x + 5, std::min(mouseX, x + 5 + trackWidth));
                int newValue = minValue + (clampedX - (x + 5)) * (maxValue - minValue) / trackWidth;
                
                if (newValue != Value) {
                    Value = std::max(minValue, std::min(maxValue, newValue));
                    needsRedraw = true;
                }
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (dragging) {
                dragging = false;
                needsRedraw = true;
            }
        }
    }

    void update() override {
        if (dragging) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            int trackWidth = w - 10;
            int clampedX = std::max(x + 5, std::min(mouseX, x + 5 + trackWidth));
            int newValue = minValue + (clampedX - (x + 5)) * (maxValue - minValue) / trackWidth;
            
            if (newValue != Value) {
                Value = std::max(minValue, std::min(maxValue, newValue));
                needsRedraw = true;
            }
        }
        
        if (Value != lastValue) {
            lastValue = Value;
            needsRedraw = true;
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font) override {
        // Background
        SDL_Rect boxdst = {x, y, w, h};
        SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
        SDL_RenderFillRect(ren, &boxdst);
        
        // Track
        SDL_Rect linedst = {x + 5, y + h / 2 - 5, w - 10, 10};
        SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
        SDL_RenderFillRect(ren, &linedst);
        
        // Thumb
        double value_percent = (double)(Value - minValue) / (maxValue - minValue);
        int thumbX = x + 5 + static_cast<int>((w - 10) * value_percent) - 5;
        SDL_Rect thumbdst = {thumbX, y + h / 2 - 10, 10, 20};
        SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(ren, &thumbdst);

        // Value text (only if changed)
        if (needsRedraw) {
            std::string valueStr = std::to_string(Value);
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, valueStr.c_str(), white);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
                if (texture) {
                    SDL_Rect dsttext = {x + w + 10, y + h / 2 - surface->h / 2, surface->w, surface->h};
                    SDL_RenderCopy(ren, texture, nullptr, &dsttext);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
            needsRedraw = false;
        }
    }
};

// Optimized vertical slider
class VerticalSlider : public Slider {
private:
    int lastValue;
    
public:
    VerticalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal)
        : Slider(x_, y_, w_, h_, minVal, maxVal, initialVal), lastValue(initialVal) {}

    void handleEvent(SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            if ((x <= mouseX && mouseX <= x + w) &&
                (y + 5 <= mouseY && mouseY <= y + h - 5)) {
                dragging = true;
                needsRedraw = true;
                
                // Immediate value update
                int trackHeight = h - 10;
                int clampedY = std::max(y + 5, std::min(mouseY, y + 5 + trackHeight));
                int newValue = minValue + (clampedY - (y + 5)) * (maxValue - minValue) / trackHeight;
                
                if (newValue != Value) {
                    Value = std::max(minValue, std::min(maxValue, newValue));
                    needsRedraw = true;
                }
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (dragging) {
                dragging = false;
                needsRedraw = true;
            }
        }
    }

    void update() override {
        if (dragging) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            int trackHeight = h - 10;
            int clampedY = std::max(y + 5, std::min(mouseY, y + 5 + trackHeight));
            int newValue = minValue + (clampedY - (y + 5)) * (maxValue - minValue) / trackHeight;
            
            if (newValue != Value) {
                Value = std::max(minValue, std::min(maxValue, newValue));
                needsRedraw = true;
            }
        }
        
        if (Value != lastValue) {
            lastValue = Value;
            needsRedraw = true;
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font) override {
        // Background
        SDL_Rect boxdst = {x, y, w, h};
        SDL_SetRenderDrawColor(ren, gray.r, gray.g, gray.b, gray.a);
        SDL_RenderFillRect(ren, &boxdst);
        
        // Track
        SDL_Rect linedst = {x + w / 2 - 5, y + 5, 10, h - 10};
        SDL_SetRenderDrawColor(ren, blue.r, blue.g, blue.b, blue.a);
        SDL_RenderFillRect(ren, &linedst);
        
        // Thumb
        double value_percent = (double)(Value - minValue) / (maxValue - minValue);
        int thumbY = y + 5 + static_cast<int>((h - 10) * value_percent) - 5;
        SDL_Rect thumbdst = {x + w / 2 - 10, thumbY, 20, 10};
        SDL_SetRenderDrawColor(ren, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(ren, &thumbdst);

        // Value text (only if changed)
        if (needsRedraw) {
            std::string valueStr = std::to_string(Value);
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, valueStr.c_str(), white);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
                if (texture) {
                    SDL_Rect dsttext = {x + w / 2 - surface->w / 2, y + h + 10, surface->w, surface->h};
                    SDL_RenderCopy(ren, texture, nullptr, &dsttext);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
            needsRedraw = false;
        }
    }
};

// Optimized texture update functions - now handles lazy loading
inline void updatetexture(TTF_Font* font, SDL_Renderer* ren, std::vector<lineObj>& lines, int& totallineheight) {
    SDL_Color Black = {0, 0, 0, 255};
    totallineheight = 0;
    
    for (size_t i = 0; i < lines.size(); i++) {
        lines[i].textureValid = false; // Mark for regeneration
        if (lines[i].texture) {
            SDL_DestroyTexture(lines[i].texture);
            lines[i].texture = nullptr;
        }
        
        // Just calculate height for layout, texture will be created when needed
        int height = TTF_FontHeight(font);
        lines[i].height = height;
        totallineheight += height;
        
        // Width will be calculated when texture is created
        lines[i].width = 0;
    }
}

// Only updates a single line's texture when needed
inline void update_single_line_texture(TTF_Font* font, SDL_Renderer* ren, std::vector<lineObj>& lines, int line, const std::string& newtext) {
    if (line < 0 || line >= lines.size()) return;
    
    lines[line].text = newtext;
    lines[line].textureValid = false;
    
    // Destroy old texture
    if (lines[line].texture) {
        SDL_DestroyTexture(lines[line].texture);
        lines[line].texture = nullptr;
    }
    
    // Don't create new texture immediately - will be created when needed for rendering
    lines[line].width = 0;
    lines[line].height = TTF_FontHeight(font);
}