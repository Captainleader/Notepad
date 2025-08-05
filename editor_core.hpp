#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <vector>

class lineObj {
    public:
        std::string text;
        SDL_Texture* texture;
        int height;
        int width;
        bool needsUpdate = true;
};

struct TextLayoutContext {
    int mouseX;
    int mouseY;
    bool selection = false;
    int scrollY = 0;
    int lineHeight = 18;
    int line = 0;
    int column = 0;
    bool showCursor = true;
    bool getcurrenttext = true;
    
    std::vector<lineObj> lines;
    std::vector<TTF_Font*> fontCache;
    TTF_Font* font;
    SDL_Rect cursordst;
    SDL_Point Mouse = {mouseX, mouseY};
};

void TextInputEvent(SDL_Event e, TextLayoutContext& tx,bool typing,int margin_x,int margin_y);
void Set_CursorPos(TextLayoutContext& tx,int margin_x, int margin_y, int x , int y);
void EnterKeyEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y);
void BackSpaceKeyEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y);
void ArrowKeysEvent(SDL_Event e, TextLayoutContext& tx, int margin_x, int margin_y);
void TextInputEvent(SDL_Event e, TextLayoutContext& tx,bool typing,int margin_x,int margin_y);
std::pair<int, int> getLineAndColumn(TextLayoutContext& tx, int margin_y,int margin_x);
class Slider {
public:
    int minValue, maxValue;
    int x, y, w, h;
    int Value;
    bool dragging;

    SDL_Color gray = {34, 53, 66, 255};
    SDL_Color blue = {37, 160, 245, 255};
    SDL_Color white = {221, 227, 237, 255};

    Slider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal);
    virtual ~Slider();

    virtual void handleEvent(SDL_Event& e) = 0;
    virtual void update() = 0;
    virtual void draw(SDL_Renderer* ren, TTF_Font* font) = 0;
};

class HorizontalSlider : public Slider {
public:
    HorizontalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal);

    void handleEvent(SDL_Event& e) override;
    void update() override;
    void draw(SDL_Renderer* ren, TTF_Font* font) override;
};

class VerticalSlider : public Slider {
public:
    VerticalSlider(int x_, int y_, int w_, int h_, int minVal, int maxVal, int initialVal);

    void handleEvent(SDL_Event& e) override;
    void update() override;
    void draw(SDL_Renderer* ren, TTF_Font* font) override;
};

struct Button {
    SDL_Renderer *ren;           // Where to draw
    SDL_Rect box;                // Main button box
    SDL_Rect shadow_box;         // Slight offset for shadow effect
    SDL_Rect title_box;          // For positioning the text
    std::string title;           // Text shown
    TTF_Font *font;              // Font used
    SDL_Texture *texture;        // Pre-rendered text
    SDL_Color color;             // Current color
    bool isClicked = false;      // Track state

    Button(SDL_Renderer*, TTF_Font*, SDL_Rect, std::string);
    
    void draw();                 // Render button
    void click_event(SDL_Event&);// Handle mouse press/release
};
void SetLineTexture(TTF_Font* font, SDL_Renderer* ren, std::vector<lineObj>& lines, int line);
std::wstring OpenFile(std::vector<lineObj>& lines, TTF_Font* font, SDL_Renderer* ren);
void ClearLines(std::vector<lineObj>& lines);
