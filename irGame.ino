#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>

#define ACTIVATED LOW
const int buttonPin = 3;
int buttonState = 0;
#define IR_RECEIVE_PIN 8
#define IR_SEND_PIN 6

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t data = 0xF5A9B8;
uint8_t len = 32;

int hp = 100;
bool gameOver = false;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("HP:");
  display.setCursor((SCREEN_WIDTH - (display.getCursorX()))/2, 10);
  display.print(hp);
  display.display();
  IrReceiver.begin(IR_RECEIVE_PIN);
  IrSender.begin(IR_SEND_PIN);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == ACTIVATED) {
    IrSender.sendNEC(data, len);
    unsigned long delayStart = millis();
    while (millis() - delayStart < 10) {
    }
  }
  if (IrReceiver.decode()) {
    uint32_t decodedData = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(decodedData);
  if (decodedData == 496348928 && buttonState != ACTIVATED) {
    if (hp > 0) {
      hp -= 2;
    }
    updateDisplay();
  } else {
      Serial.println(decodedData);
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
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("HP:");
  display.setCursor((SCREEN_WIDTH - (display.getCursorX()))/2, 10);
  display.print(hp);
  display.display();
}

void displayGameOver() {
  static unsigned long lastBlinkTime = 0;
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
    }
  }
}

void waitReset() {
  display.setCursor(25, 10);
  display.display();
}