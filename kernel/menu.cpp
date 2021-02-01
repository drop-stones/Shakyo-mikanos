#include "menu.hpp"
#include <cstdio>

//Menu::Menu(PixelWriter& writer, const int width, const int height)
//  : writer_{writer}, width_{width}, height_{height} {}
//
//void Menu::DrawMenu() {
//}
//
//void Menu::HideMenu() {
//}

/*
namespace {
  const char power_button_shape[kMenuHeight][kMenuWidth + 1] = {
    "         @@         ",
    "         @@         ",
    "         @@         ",
    "      @@ @@ @@      ",
    "    @@   @@  @@     ",
    "   @@@   @@   @@@   ",
    "  @@@    @@    @@@  ",
    "  @@     @@     @@  ",
    " @@@            @@@ ",
    " @@              @@ ",
    " @@              @@ ",
    " @@              @@ ",
    " @@@            @@@ ",
    "  @@            @@  ",
    "   @@          @@   ",
    "    @@@      @@@    ",
    "      @@@  @@@      ",
    "       @@@@@@       ",
    "                    ",
    "                    ",
  };
}
*/

extern const uint8_t _binary_power_button_bin_start;
extern const uint8_t _binary_power_button_bin_end;
extern const uint8_t _binary_power_button_bin_size;

PixelColor GetPowerButtonPixelColor(int x, int y) {
  MenuPixel* p = (MenuPixel*)&_binary_power_button_bin_start + 4 + x + kMenuWidth * y;
  PixelColor c {p->r, p->g, p->b};
  return c;
}

void DrawMenu(PixelWriter* pixel_writer, Vector2D<int> position) {
  //FillRectangle(*pixel_writer, position, {kMenuWidth, kMenuHeight}, {255, 255, 255});
  for (int x = 0; x < kMenuWidth; x++) {
    for (int y = 0; y < kMenuHeight; y++) {
      const PixelColor c = GetPowerButtonPixelColor(x, y);
      pixel_writer->Write(x, y, c);
    }
  }
}
