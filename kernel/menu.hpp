#pragma once

#include "graphics.hpp"
#include "font.hpp"

//class Menu {
//public:
//  Menu(PixelWriter& writer, const int width, const int height);
//  void DrawMenu();
//  void HideMenu();
//
//private:
//  PixelWriter& writer_;
//  const int width_, height_;
//};
//
//extern char menu_buf[sizeof(Menu)];
//extern Menu* menu;

struct MenuPixel {
  uint8_t b, g, r;
};

const int kMenuWidth  = 50;
const int kMenuHeight = 50;
const PixelColor kMenuBGColor {100, 100, 100};

void DrawMenu(PixelWriter* pixel_writer, Vector2D<int> position);