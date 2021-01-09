#pragma once

#include "graphics.hpp"

struct WallPaperPixel {
  uint8_t b, g, r;
};

class WallPaper {
public:
  WallPaper(PixelWriter& writer, const int kFrameWidth, const int kFrameHeight);
  void WriteWallPaper();

private:
  PixelColor GetPixelColor(int x, int y);

  PixelWriter& writer_;
  const int kFrameWidth_, kFrameHeight_;
  WallPaperPixel* firstPixel_;
  int wallpaperWidth_, wallpaperHeight_, wallpaperColor_;
};

extern char wallpaper_buf[sizeof(WallPaper)];
extern WallPaper* wallpaper;