#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>
#include <avr/sleep.h>

#define ACTIVATED LOW
const int buttonPin = 3;
int buttonState = 0;
#define IR_RECEIVE_PIN 8
#define IR_SEND_PIN 6

#define TIMEOUT 5000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t data = 0xF5A9B8;
uint8_t len = 32;

int hp = 100;
bool gameOver = false;
int32_t lastReceiveTime = 0;
const int32_t receiveDelay = 250;

void updateDisplay() {
  display.clearDisplay();

  float size = 24.0f * ((int)((hp + 9) / 10) * 0.1f);
  
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  for (float angle = 0.0f; angle <= 360.0f; angle += 0.5f) {
    float radian = angle * PI / 180.0f;
    float x = 16.0f * pow(sin(radian), 3.0f);
    float y = -1 * (13.0f * cos(radian) - 5.0f * cos(2 * radian) - 2.0f * cos(3 * radian) - cos(4 * radian));
    x *= size / 24.0f;
    y *= size / 24.0f;
    display.drawPixel(centerX + x, centerY + y, WHITE);
  }

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("HP: ");
  display.print(hp);
  display.display();
}


void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextColor(WHITE);
  updateDisplay();
  IrReceiver.begin(IR_RECEIVE_PIN);
  IrSender.begin(IR_SEND_PIN);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == ACTIVATED) {
    lastReceiveTime = millis();
    IrSender.sendNEC(data, len);
    int32_t delayStart = millis();
    while (millis() - delayStart < 10) {
      displayInvulnerable();
    }
  } else {
    if (millis() - lastReceiveTime < TIMEOUT) {
      updateDisplay();
    }
  }

  if (IrReceiver.decode()) {
    uint32_t decodedData = IrReceiver.decodedIRData.decodedRawData;
    int32_t currentTime = millis();
    if (decodedData == 496348928 && buttonState != ACTIVATED && hp > 0 && 
        currentTime - lastReceiveTime >= receiveDelay) {
      hp -= 10;
      updateDisplay();
      lastReceiveTime = currentTime;
    }
    IrReceiver.resume();
  }

  if (hp <= 0 && !gameOver) {
    gameOver = true;
    displayGameOver();
  }

  if (gameOver) {
    waitReset();
  }

  if (millis() - lastReceiveTime > TIMEOUT) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();
  } else {
    display.ssd1306_command(SSD1306_DISPLAYON);
  }
}


void displayInvulnerable() {
  int16_t x, y;
  uint16_t w, h;
  display.clearDisplay();
  display.setTextSize(2);
  display.getTextBounds("Damage", 0, 0, &x, &y, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2 - h + 8);
  display.println("Damage");
  display.getTextBounds("Immunity", 0, 0, &x, &y, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2 + 9);
  display.println("Immunity");
  display.display();
}

void displayGameOver() {
  static int32_t lastBlinkTime = 0;
  static bool displayText = true;
  int16_t x, y;
  uint16_t w, h;
  while(gameOver) {
    if (millis() - lastBlinkTime > 500) {
      lastBlinkTime = millis();
      display.clearDisplay();
      if (displayText) {
        display.setTextSize(2);
        display.getTextBounds("GAME OVER", 0, 0, &x, &y, &w, &h);
        display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2 - h + 8);
        display.println("GAME OVER");
      }
      display.setTextSize(1);
      display.getTextBounds("Press button to reset", 0, 0, &x, &y, &w, &h);
      display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2 + 12);
      display.println("Press button to reset");
      display.display();
      displayText = !displayText;
    }
    buttonState = digitalRead(buttonPin);
    if (buttonState == ACTIVATED) {
      hp = 100;
      gameOver = false;
      display.setTextSize(2);
      updateDisplay();
      delay(1000);
    }
  }
}

void waitReset() {
  display.setCursor(25, 10);
  display.display();
}