#include <stdio.h>
#include "../sprite.h"

int main(int argc, char **argv) {
  printf("VSprite test #1...\n");

  Sprite *sprite = sprite_new("test/nemesis-turret.svg", 2.5);
  
  printf("Done w/ test...\n");
  printf("height = %f, width = %f\n", sprite->height, sprite->width);
  return 0;
}
