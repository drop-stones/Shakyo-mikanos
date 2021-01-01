#include "wallpaper.hpp"
#include "font.hpp"
#include <cstdio>

extern const uint8_t _binary_wallpaper_img_bin_start;
extern const uint8_t _binary_wallpaper_img_bin_end;
extern const uint8_t _binary_wallpaper_img_bin_size;

WallPaper::WallPaper(PixelWriter& writer, const int kFrameWidth, const int kFrameHeight)
    : writer_{writer}, kFrameWidth_{kFrameWidth}, kFrameHeight_{kFrameHeight} {
  wallpaperHeight_  = *(int*)(&_binary_wallpaper_img_bin_start);
  wallpaperWidth_ = *(int*)(&_binary_wallpaper_img_bin_start + 4);
  wallpaperColor_  = *(int*)(&_binary_wallpaper_img_bin_start + 8);

  firstPixel_ = (WallPaperPixel*)&_binary_wallpaper_img_bin_start + 12;
  int overhangWidth, overhangHeight;
  overhangWidth = wallpaperWidth_ - kFrameWidth_;
  if (overhangWidth > 0) {
    // wallpaper is wider
    firstPixel_ += overhangWidth / 2;
  }
  overhangHeight = wallpaperHeight_ - kFrameHeight_;
  if (overhangHeight > 0) {
    // wallpaper is higher
    firstPixel_ += (overhangHeight / 2) * wallpaperWidth_;
  }

  // print debug
  char buf[1024];
  sprintf(buf, "kFrameWidth = %d, kFrameHeight = %d", kFrameWidth_, kFrameHeight_);
  WriteString(writer_, 100, 100, buf, {0, 0, 0});
  sprintf(buf, "wallpaperWidth_ = %d, wallpaperHeight_ = %d, wallpaperColor_ = %d", wallpaperWidth_, wallpaperHeight_, wallpaperColor_);
  WriteString(writer_, 100, 116, buf, {0, 0, 0});
  sprintf(buf, "firstPixel_: b = %d, g = %d, r = %d", firstPixel_->b, firstPixel_->g, firstPixel_->r);
  WriteString(writer_, 100, 132, buf, {0, 0, 0});
}

PixelColor WallPaper::GetPixelColor(int x, int y) {
  WallPaperPixel* p = firstPixel_ + x + wallpaperWidth_ * y;
  PixelColor c {p->r, p->g, p->b};
  return c;
}

void WallPaper::WriteWallPaper() {
  for (int x = 0; x < kFrameWidth_; x++) {
    for (int y = 0; y < kFrameHeight_; y++) {
      const PixelColor c = GetPixelColor(x, y);
      writer_.Write(x, y, c);
    }
  }
}