#ifndef DISPLAY_H
#define DISPLAY_H

#include "LEDBackpack.h"

class Display {
 public:
  Display(void);
  void init();
  void renderString(String to_display);
  void renderCharArray(char * to_display);
  void clear();
 private:
  AlphaNum4 a1_4;
  AlphaNum4 a5_8;
};

#endif
