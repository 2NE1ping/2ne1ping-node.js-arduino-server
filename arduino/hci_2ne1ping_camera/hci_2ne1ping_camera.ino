#include <cm1106_i2c.h>
#include <LiquidCrystal_I2C.h>

CM1106_I2C cm1106_i2c;
LiquidCrystal_I2C lcd(0x27, 20, 4);

int redPin = 11;
int greenPin = 10;
int bluePin = 9;
int buzzer = 13;
int yellowLed = 6; // 영상

// 음계 주파수
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392

int melody[] = {NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, 
                NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4};
int durations[] = {250, 250, 250, 250, 250, 250, 250, 250, 
                   250, 250, 250, 250, 250, 250, 250};

unsigned long previousMillis1 = 0;
unsigned long previousMillis5 = 0;
const long interval1 = 250; // 1초 간격
const long interval5 = 5000; // 5초 간격
bool ledState = LOW;

void setColor(int red, int green, int blue) {
  analogWrite(redPin, 255 - red);
  analogWrite(greenPin, 255 - green);
  analogWrite(bluePin, 255 - blue);
}

void playMelody() {
  for (int i = 0; i < 15; i++) {
    int noteDuration = durations[i];
    tone(buzzer, melody[i], noteDuration);
    delay(noteDuration * 1.30);
    noTone(buzzer);
  }
}

void setup() {
  cm1106_i2c.begin();
  Serial.begin(9600);
  delay(1000);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(yellowLed, OUTPUT);

  setColor(0, 0, 0); // 초기 상태: LED 끄기

  lcd.init();  // LCD 초기화
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CO2 Monitoring...");
}

void loop() {
  unsigned long currentMillis = millis();

  // 5초마다 실행되는 작업 (CO2 측정)
  if (currentMillis - previousMillis5 >= interval5) {
    previousMillis5 = currentMillis;

    uint8_t ret = cm1106_i2c.measure_result();

    lcd.clear(); // 화면 초기화
    lcd.setCursor(0, 0);
    lcd.print("CO2(ppm):");

    if (ret == 0) {
      int co2 = cm1106_i2c.co2;

      lcd.setCursor(10, 0);
      lcd.print(co2);

      // 시리얼 모니터에 CO2 값 출력
      Serial.println(co2);

      if (co2 < 1000) {
        setColor(0, 0, 255); // 파란색: 안전
        lcd.setCursor(0, 1);
        lcd.print("Status: Safe");
      } else if (co2 >= 1000 && co2 <= 2000) {
        setColor(255, 255, 0); // 노란색: 주의
        lcd.setCursor(0, 1);
        lcd.print("Status: Warn..!");
      } else {
        setColor(255, 0, 0); // 빨간색: 위험
        lcd.setCursor(0, 1);
        lcd.print("Status: Danger!!!");
        playMelody();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("CO2 Read Error");
      setColor(0, 0, 0); // LED 끄기

      // 시리얼 모니터에 에러 출력
      Serial.println("CO2 Read Error");
    }
  }

  // 1초마다 실행되는 작업 (LED 깜빡임)
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis;
    ledState = !ledState;
    digitalWrite(yellowLed, ledState);
  }
}