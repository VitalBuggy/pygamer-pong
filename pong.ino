#include <Adafruit_Arcada.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN   8
#define PIXEL_COUNT 5

#define JOYSTICK_DEADZONE_UB -2
#define JOYSTICK_DEADZONE_LB -6
#define JOYSTICK_SENSITIVITY 2

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
  int x_pos;
  int y_pos;
  int width;
  int height;
  int plat_v;
};

Ball pBall = { 80, 64, 2, 1, 2 };
Platform player_plat = { 0, 50, 3, 30, 0 };
Platform ai_plat = { 157, 50, 3, 30, 0 };

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
  int joyY = arcada.readJoystickY();
  uint8_t pressed_buttons = arcada.readButtons();

  if (pressed_buttons & ARCADA_BUTTONMASK_A) {
    Serial.print("A");
    arcada.display->drawCircle(145, 100, 10, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_B) {
    Serial.print("B");
    arcada.display->drawCircle(120, 100, 10, ARCADA_WHITE);
  }
  
  if (joyY < JOYSTICK_DEADZONE_LB) {
    player_plat.plat_v = JOYSTICK_SENSITIVITY * -1;
    ai_plat.plat_v = JOYSTICK_SENSITIVITY * -1;
  } else if (joyY > JOYSTICK_DEADZONE_UB) {
    player_plat.plat_v = JOYSTICK_SENSITIVITY;
    ai_plat.plat_v = JOYSTICK_SENSITIVITY;
  } else {
    player_plat.plat_v = 0;
    ai_plat.plat_v = 0;
  }
}


void update() {
//  Serial.println(player_plat.y_pos);

  
  if (pBall.y_pos == 128 - pBall.radius || pBall.y_pos == 0 + pBall.radius) {
    pBall.y_vel *= -1;
  }
  

  if (pBall.x_pos == player_plat.x_pos + pBall.radius && (pBall.y_pos > player_plat.y_pos && pBall.y_pos < player_plat.y_pos + player_plat.height)) {
    pBall.x_vel *= -1;
  }

  if (pBall.x_pos == ai_plat.x_pos - ai_plat.width + pBall.radius && (pBall.y_pos > ai_plat.y_pos && pBall.y_pos < ai_plat.y_pos + ai_plat.height)) {
    pBall.x_vel *= -1;
  }

  if ((player_plat.y_pos + player_plat.height < 128 && player_plat.plat_v > 0) || player_plat.y_pos > 0 && player_plat.plat_v < 0) {
    player_plat.y_pos += player_plat.plat_v;
  }

  if ((ai_plat.y_pos + ai_plat.height < 128 && ai_plat.plat_v > 0) || ai_plat.y_pos > 0 && ai_plat.plat_v < 0) {
    ai_plat.y_pos += ai_plat.plat_v;
  }

  
  pBall.x_pos += pBall.x_vel;
  pBall.y_pos += pBall.y_vel;
}

void draw() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->fillCircle(pBall.x_pos, pBall.y_pos, pBall.radius, ARCADA_WHITE);
  arcada.display->fillRect(player_plat.x_pos, player_plat.y_pos, player_plat.width, player_plat.height, ARCADA_WHITE);
  arcada.display->fillRect(ai_plat.x_pos, ai_plat.y_pos, ai_plat.width, ai_plat.height, ARCADA_WHITE);
}

void loop() {
  delay(10);
  if (GAME_STATE == 1) {
    process_input();
    update();
    draw();    
  }
}
