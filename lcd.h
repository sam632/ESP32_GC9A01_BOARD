#include <SPI.h>
#include <TFT_eSPI.h>
#include "custom_fonts.h"

void fillArc(TFT_eSPI &tft, int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 6; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to segement start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;

    if(i == start_angle + seg * seg_count - inc) {
      int xc = (x2 + x3) / 2;
      int yc = (y2 + y3) / 2;
      // int rad = sqrt(pow(x2-xc,2) + pow(y2-yc,2));
      tft.fillCircle(xc, yc, w / 2, colour);
    }
  }
}

void initDisplay(TFT_eSPI &tft) {

  tft.fillCircle(120, 120, 120, CLOCK_BG);
  tft.fillCircle(120, 120, 115, BACKGROUND);

  tft.setFreeFont(&Roboto_Black_50);
  tft.setTextColor(TEXT_COLOUR, BACKGROUND);
  tft.setTextSize(2);
  tft.drawString("`", 148, 70, 4);
  tft.setTextSize(1);
  tft.drawString("C", 168, 70);

  int rect_x = 72;
  int rect_y = 190;
  int t_offset = 32;
  tft.fillRoundRect(rect_x, rect_y, 240 - 2 * rect_x, 240 - rect_y, 8, CLOCK_BG);
  tft.fillTriangle(rect_x - t_offset, 240, rect_x + 3, rect_y + 2, rect_x, 240, CLOCK_BG);
  tft.fillTriangle(240 - rect_x, 240, 240 - rect_x - 3, rect_y + 2, 240 - rect_x + t_offset, 240, CLOCK_BG);
}

void updateTempDial(TFT_eSPI &tft, String temperature) {

    int max_seg = 47;
    float temp_range = 20.0;

    fillArc(tft, 120, 120, 216, max_seg, 112, 112, 14, BACKGROUND);
    int arc = (int)(::atof(temperature.c_str()) - 10) * max_seg/ temp_range;
    if( arc > max_seg ) {
      arc = max_seg;
    }
    unsigned int colour = TFT_GREEN;
    if (arc < 14) {
      colour = TFT_BLUE;
    }
    else if (arc > 36 ) {
      colour = TFT_RED;
    }
    fillArc(tft, 120, 120, 216, arc, 110, 110, 10, colour);
}

void updateTemp(TFT_eSprite &sprite, TFT_eSPI &tft, bool power, String temperature) {

  if (power) {
    sprite.createSprite(113, 55);
    sprite.fillRect(0, 0, 113, 55, BACKGROUND);  // Clear

    sprite.setTextSize(1);
    sprite.setFreeFont(&Roboto_Black_50);
    sprite.setTextColor(TEXT_COLOUR, BACKGROUND);

    sprite.drawString(temperature, 3, 3);
    sprite.pushSprite(38, 70);
    sprite.deleteSprite();

    updateTempDial(tft, temperature);
  }
}

void updateHumidity(TFT_eSprite &sprite, bool power, String humidity) {

  if (power) {
    sprite.createSprite(105, 48);
    sprite.fillRect(0, 0, 105, 48, BACKGROUND);  // Clear

    sprite.setTextSize(1);
    sprite.setFreeFont(&Roboto_Black_50);
    sprite.setTextColor(TFT_RED, BACKGROUND);

    sprite.drawString(humidity + "%", 3, 0);
    sprite.pushSprite(67, 125);
    sprite.deleteSprite();
  }
}

void updateTime(TFT_eSprite &sprite, bool power, String curr_time) {

  if (power) {
    sprite.createSprite(92, 27);
    sprite.fillRect(0, 0, 92, 27, CLOCK_BG);
    sprite.setTextColor(TFT_WHITE, CLOCK_BG);
    sprite.setFreeFont(&DSEG7_Classic_Mini_Regular_26);

    sprite.drawString(curr_time, 0, 0);
    sprite.pushSprite(73, 200);
    sprite.deleteSprite();
  }
}