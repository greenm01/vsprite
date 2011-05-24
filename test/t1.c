#include <stdio.h>
#include "../svg.h"

int main(int argc, char **argv) {
  printf("VSprite test #1...\n");

  svg_load("test/nemesis-turret.svg", 2.5, NULL);

  printf("Done w/ test...\n");
  return 0;
}
