#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_NeoPixel.h>

uint32_t buttons, last_buttons;

#define PIXEL_PIN   8
#define PIXEL_COUNT 5

#define JOYSTICK_DEADZONE_UB -2
#define JOYSTICK_DEADZONE_LB -6
#define JOYSTICK_SENSITIVITY 4

#define BUTTON_CLOCK 48
#define BUTTON_DATA 49
#define BUTTON_LATCH 50

#define DEFAULT_BALL_X 80
#define DEFAULT_BALL_Y 64

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

/*
* Variable controlling the state of the game
* 0 -> Start menu
* 1 -> Game running
* 2 -> Pause
*/
unsigned short int GAME_STATE = 0;

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

Ball pBall = { 80, 64, 2, 2, 2 };
Platform player_plat = { 0, 50, 3, 30, 0 };
Platform ai_plat = { 157, 50, 3, 30, 0 };

volatile uint16_t milliseconds = 0;
void timercallback() {
  analogWrite(13, milliseconds);  // pulse the LED
  if (milliseconds == 0) {
    milliseconds = 255;
  } else {
    milliseconds--;
  }
}

uint8_t read_buttons() {
  uint8_t result = 0;
  
  pinMode(BUTTON_CLOCK, OUTPUT);
  digitalWrite(BUTTON_CLOCK, HIGH);
  pinMode(BUTTON_LATCH, OUTPUT);
  digitalWrite(BUTTON_LATCH, HIGH);
  pinMode(BUTTON_DATA, INPUT);
  
  digitalWrite(BUTTON_LATCH, LOW);
  digitalWrite(BUTTON_LATCH, HIGH);
  
  for(int i = 0; i < 8; i++) {
    result <<= 1;
    //Serial.print(digitalRead(BUTTON_DATA)); Serial.print(", ");
    result |= digitalRead(BUTTON_DATA);
    digitalWrite(BUTTON_CLOCK, HIGH);
    digitalWrite(BUTTON_CLOCK, LOW);
  }
  Serial.println();
  return result;
}

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show();

  for (int i=0; i<250; i++) {
    arcada.setBacklight(i);
    delay(10);
  }

  arcada.displayBegin();

  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  }
  Serial.println("Reading QSPI ID");
  Serial.print("JEDEC ID: 0x"); Serial.println(Arcada_QSPI_Flash.getJEDECID(), HEX);
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->print("QSPI Flash JEDEC 0x"); arcada.display->println(Arcada_QSPI_Flash.getJEDECID(), HEX);

  arcada.display->setTextWrap(true);

  buttons = last_buttons = 0;
  arcada.timerCallback(1000, timercallback);
}

void process_input() {
  int joyY = arcada.readJoystickY();

  if(read_buttons() == 8) {
    // puase game
    GAME_STATE = 2;
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

uint8_t PLAYER_SCORE, AI_SCORE = 0;

void update() {
  if (pBall.x_pos + pBall.radius <= 0) {
    AI_SCORE += 1;
    pBall.x_pos = DEFAULT_BALL_X;
    pBall.y_pos = DEFAULT_BALL_Y;
  }

  if (pBall.x_pos - pBall.radius >= 160) {
    PLAYER_SCORE += 1;
    pBall.x_pos = DEFAULT_BALL_X;
    pBall.y_pos = DEFAULT_BALL_Y;
  }
  
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

  Serial.println(PLAYER_SCORE);
  Serial.println(AI_SCORE);
  Serial.println();
  
  pBall.x_pos += pBall.x_vel;
  pBall.y_pos += pBall.y_vel;
}

void draw() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->fillCircle(pBall.x_pos, pBall.y_pos, pBall.radius, ARCADA_WHITE);
  arcada.display->fillRect(player_plat.x_pos, player_plat.y_pos, player_plat.width, player_plat.height, ARCADA_WHITE);
  arcada.display->fillRect(ai_plat.x_pos, ai_plat.y_pos, ai_plat.width, ai_plat.height, ARCADA_WHITE);
}

void process_input_menu() {
}

void update_menu() {
  Serial.println(read_buttons());
  if (read_buttons() == 16) {
    GAME_STATE = 1;
  }
}

void draw_menu() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->setCursor(66, 40);
  arcada.display->print("PONG");
  arcada.display->setCursor(20, 50);
  arcada.display->print("Press START to begin");
   
}

void process_input_pause() {
  if (read_buttons() == 16) {
    GAME_STATE = 1; 
  }
}

void update_pause() {
  
}

void draw_pause() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->setCursor(66, 40);
  arcada.display->print("PAUSED");
  arcada.display->setCursor(20, 50);
  arcada.display->print("Press START to unpause");
}

void loop() {
  delay(25);
  if (GAME_STATE == 0) {
    process_input_menu();
    update_menu();
    draw_menu();
  } else if (GAME_STATE == 1) {
    process_input();
    update();
    draw();    
  } else if (GAME_STATE == 2) {
    process_input_pause();
    update_pause();
    draw_pause();
  }
}
