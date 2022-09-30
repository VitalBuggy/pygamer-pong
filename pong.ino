#include <Adafruit_Arcada.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN   8
#define PIXEL_COUNT 5

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_Arcada arcada;

/*
* Variable controlling the state of the game
* 0 -> Start menu
* 1 -> Game running
* 2 -> Pause
*/
unsigned short int GAME_STATE = 1;

struct Ball {
  int x_pos;
  int y_pos;
  int x_vel;
  int y_vel;
  int radius;
};

struct Platform {
  int y_pos;
  int x_pos;
  int width;
  int height;
};

Ball pBall = { 20, 20, 0, 0, 5 };
Platform player_plat = {10, 50, 10, 20};
Platform ai_plat = {10, 50, 10, 20};

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show();

  for (int i=0; i<250; i++) {
    arcada.setBacklight(i);
    delay(10);
  }
  
  arcada.displayBegin();
}

void process_input() {
  
}

void update() {
  
}

void draw() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->fillCircle(pBall.x_pos, pBall.y_pos, pBall.radius, ARCADA_WHITE);
  arcada.display->fillRect(player_plat.x_pos, player_plat.y_pos, player_plat.width, player_plat.height, ARCADA_WHITE);
  delay(10);
}

void loop() {
  if (GAME_STATE == 1) {
    process_input();
    update();
    draw();    
  }
}
