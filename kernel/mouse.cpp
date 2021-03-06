#include "mouse.hpp"

#include "graphics.hpp"

namespace {
  const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
  };
}

void DrawMouseCursor(PixelWriter* pixel_writer, Vector2D<int> position) {
  for (int dy = 0; dy < kMouseCursorHeight; dy++) {
    for (int dx = 0; dx < kMouseCursorWidth; dx++) {
      if (mouse_cursor_shape[dy][dx] == '@') {
        pixel_writer->Write(position.x + dx, position.y + dy, kMouseCursorBoaderColor);
      } else if (mouse_cursor_shape[dy][dx] == '.') {
        pixel_writer->Write(position.x + dx, position.y + dy, kMouseCursorInnerColor);
      } else {
        pixel_writer->Write(position.x + dx, position.y + dy, kMouseTransparentColor);
      }
    }
  }
}