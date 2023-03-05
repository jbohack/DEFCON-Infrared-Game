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

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  IrReceiver.begin(IR_RECEIVE_PIN);
  IrSender.begin(IR_SEND_PIN);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin,HIGH);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == ACTIVATED) {
    IrSender.sendNEC(data, len);
    delay(100);
  }
  if (IrReceiver.decode()) {
    Serial.println(IrReceiver.decodedIRData.decodedRawData);
    if (IrReceiver.decodedIRData.decodedRawData == 496348928) {
      hp--;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("HP: ");
      display.print(hp);
      display.display();
      delay(10);
      display.clearDisplay();
      display.display();
if (hp <= 0) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("GAME OVER");
  display.display();
  while (true) {
    // wait for a reset
    buttonState = digitalRead(buttonPin);
    if (buttonState == ACTIVATED) {
      hp = 100;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("HP: ");
      display.print(hp);
      display.display();
      break;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("GAME OVER");
    display.display();
    delay(500);
    display.clearDisplay();
    display.display();
    delay(500);
  }
}
    } else{
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(IrReceiver.decodedIRData.decodedRawData);
        display.display();
        delay(2000);
        display.clearDisplay();
    }
    IrReceiver.resume();
  }
}
