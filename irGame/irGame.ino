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

#define TIMEOUT 30000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t regenCode = 0x55CDFC;
uint32_t hitCode = 0xF5A9B8;
uint32_t reviveCode = 0xFFFFFF;
uint8_t len = 32;

int hp = 100;
bool gameOver = false;
int32_t lastReceiveTime = 0;
const int32_t receiveDelay = 250;

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);

  int heartWidth = 8;
  int heartSpacing = SCREEN_WIDTH - heartWidth * 2 - 4;
  int heartY = (SCREEN_HEIGHT - 16) / 2;
  display.setCursor(4, heartY);
  display.setTextSize(2);
  display.write(0x03);
  display.setCursor(SCREEN_WIDTH - 12, heartY);
  display.write(0x03);
  
  display.setTextSize(2);
  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds("HP:", 0, 0, &x, &y, &w, &h);
  
  int digitLength = strlen(String(hp).c_str());
  int valueWidth = digitLength * 14;
  int hpTextX = (heartSpacing - w - valueWidth) / 2 + heartWidth + 2;
  int hpValueX = hpTextX + w;
  
  if (digitLength == 2) {
    hpValueX += 2;
  } else if (digitLength == 3) {
    hpValueX += 6;
  }
  
  display.setCursor(hpTextX, (SCREEN_HEIGHT - h) / 2);
  display.print("HP:");
  display.setCursor(hpValueX, (SCREEN_HEIGHT - h) / 2);
  display.print(hp);
  display.display();
}

void setup() {
  Serial.begin(9600);
  Serial.print("Check out the GitHub to view updates or contribute!\nhttps://github.com/jbohack/DEFCON-Infrared-Game\n");
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
    IrSender.sendNECMSB(hitCode, len);
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
    Serial.println(decodedData);
    if (decodedData == 496348928 && buttonState != ACTIVATED && hp > 0 && 
        currentTime - lastReceiveTime >= receiveDelay) {
      hp -= 10;
      updateDisplay();
      lastReceiveTime = currentTime;
    } else if (decodedData == 1068739072 && hp > 0 && 
        currentTime - lastReceiveTime >= receiveDelay) {
      hp = 100;
      updateDisplay();
      lastReceiveTime = currentTime;
    }
    IrReceiver.resume();
  }

  if (hp <= 0 && !gameOver) {
    gameOver = true;
    displayGameOver();
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
    uint32_t decodedData = 0;
    if (IrReceiver.decode()) {
      decodedData = IrReceiver.decodedIRData.decodedRawData;
      Serial.println(decodedData);
      IrReceiver.resume();
    }
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
    if (buttonState == ACTIVATED || decodedData == 4294967040) {
      hp = 100;
      gameOver = false;
      display.setTextSize(2);
      updateDisplay();
      IrSender.sendNECMSB(regenCode, len);
      delay(1000);
      IrSender.sendNECMSB(regenCode, len);
      delay(1000);
      IrSender.sendNECMSB(regenCode, len);
      delay(1000);
    }
  }
}
