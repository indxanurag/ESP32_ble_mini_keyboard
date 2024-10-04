#include <BleKeyboard.h>
#include <Adafruit_NeoPixel.h>
#include <esp_sleep.h>

#define right_key 13
#define left_key 12
#define up_key 14
#define down_key 27
#define middle_key 26
#define LED_PIN 23
#define LED_COUNT 1
#define LONG_PRESS_DURATION 3000 // 3 seconds in milliseconds

BleKeyboard bleKeyboard;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

RTC_DATA_ATTR unsigned long lastConnectedTime = 0;
RTC_DATA_ATTR bool wasConnected = false;
RTC_DATA_ATTR int bootCount = 0;

unsigned long buttonPressStartTime = 0;
bool isButtonPressed = false;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void enterDeepSleep() {
  // Turn off the LED before sleeping
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();

  // Go to deep sleep mode
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow time for Serial Monitor to open
  
  // Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Print the wakeup reason for ESP32
  print_wakeup_reason();

  bleKeyboard.begin();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  pinMode(right_key, INPUT);
  pinMode(left_key, INPUT);
  pinMode(up_key, INPUT);
  pinMode(down_key, INPUT);
  pinMode(middle_key, INPUT); // Middle key is now input without pull-up

  // Configure the middle key for waking up from sleep mode on HIGH signal
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 1); // 1 = wake up when HIGH

  if (!wasConnected) {
    lastConnectedTime = millis();
  }
}

void loop() {
  unsigned long currentTime = millis();

  if (bleKeyboard.isConnected()) {
    wasConnected = true;
    lastConnectedTime = currentTime; // Reset the timer if connected

    // Set LED to green if connected
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();

    if (digitalRead(up_key) == HIGH && digitalRead(middle_key) == HIGH){
      strip.setPixelColor(0, strip.Color(0, 255, 255));
      strip.show();      
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      delay(100);
    } else if (digitalRead(down_key) == HIGH && digitalRead(middle_key) == HIGH){
      strip.setPixelColor(0, strip.Color(0, 255, 255));
      strip.show(); 
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      delay(100);
    } else if (digitalRead(left_key) == HIGH && digitalRead(middle_key) == HIGH){
      strip.setPixelColor(0, strip.Color(0, 255, 255));
      strip.show();
      bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
      delay(500);
    } else if (digitalRead(right_key) == HIGH && digitalRead(middle_key) == HIGH){
      strip.setPixelColor(0, strip.Color(0, 255, 255));
      strip.show();
      bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
      delay(500);
    } else if (digitalRead(right_key) == HIGH) {
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      bleKeyboard.write(KEY_RIGHT_ARROW);
      delay(100);
    } else if (digitalRead(left_key) == HIGH) {
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      bleKeyboard.write(KEY_LEFT_ARROW);
      delay(100);
    } else if (digitalRead(up_key) == HIGH) {
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      bleKeyboard.write(KEY_UP_ARROW);
      delay(100);
    } else if (digitalRead(down_key) == HIGH) {
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      bleKeyboard.write(KEY_DOWN_ARROW);
      delay(100);
    } else if (digitalRead(middle_key) == HIGH) {
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      delay(500);
    } else {
      // Set LED back to green if no key is pressed
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.show();
    }
  } else {
    if (wasConnected) {
      wasConnected = false;
      lastConnectedTime = currentTime;
    }

    // Blink red if not connected
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
    delay(500);

    // Check if the ESP32 should go to sleep
    if (currentTime - lastConnectedTime > 20000) {
      enterDeepSleep();
    }
  }

  // Check for long press on up and down keys simultaneously
  if (digitalRead(up_key) == HIGH && digitalRead(down_key) == HIGH) {
    if (!isButtonPressed) {
      buttonPressStartTime = currentTime;
      isButtonPressed = true;
    } else if (currentTime - buttonPressStartTime >= LONG_PRESS_DURATION) {
      strip.setPixelColor(0, strip.Color(255, 0, 255));
      strip.show();
      delay(500);
      strip.setPixelColor(0, strip.Color(255, 255, 0));
      strip.show();
      delay(500);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      enterDeepSleep();
    }
  } else {
    isButtonPressed = false;
  }
}
