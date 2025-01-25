#define BLYNK_TEMPLATE_ID "TMPL3IB6tWBiU"
#define BLYNK_TEMPLATE_NAME "Esp8266 Water Level"
#define BLYNK_AUTH_TOKEN "4fqrL-V4lYbfVKfVsgYarGmM0JtSCkmy"

// Your WiFi credentials
char ssid[] = "Diyabarta Naorem";   // WiFi Name
char pass[] = "diya%naorem";        // WiFi Password

#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>

// Define pins for ESP8266
#define D4 2   // GPIO2
#define D5 14  // GPIO14
#define D6 12  // GPIO12

#define i2c_Address 0x3c // Initialize with the I2C addr 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // QT-PY / XIAO

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int trigPin = D5;
const int echoPin = D6;
const int buzzer = D4;

const float tankHeight = 23; // Height of the water tank in cm

float duration, distance, waterLevel;

bool buzzerEnabled = true; // Flag to enable/disable the buzzer

// Timer for sending data to Blynk
BlynkTimer timer;

void setup() {
  // Debug console
  Serial.begin(9600);

  // Blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);

  // OLED setup
  delay(250); // Wait for the OLED to power up
  display.begin(i2c_Address, true);
  display.display(); // Show Adafruit splash screen
  display.clearDisplay();

  // Timer to send data to Blynk
  timer.setInterval(1000L, sendToBlynk); // Send data every second
}

void loop() {
  Blynk.run();  // Run Blynk
  timer.run();  // Run the timer

  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

  // Calculate water level
  waterLevel = ((tankHeight - distance) / tankHeight) * 100;
  waterLevel = constrain(waterLevel, 0, 100); // Constrain value to 0-100%

  // Display water level on OLED
  String levelStr = "Water Level: " + String(waterLevel, 1) + "%";
  displayText(levelStr.c_str(), 0, 2, 2);

  // Buzzer logic
  if (buzzerEnabled && distance <= 15) {
    tone(buzzer, 1000);  // Sound buzzer at 1kHz
  } else {
    noTone(buzzer);      // Turn off the buzzer
  }

  delay(500); // Update every half second
}

/**
 * Function to send data to Blynk
 */
void sendToBlynk() {
  Blynk.virtualWrite(V1, waterLevel); // Send water level percentage to Virtual Pin V1
  Blynk.virtualWrite(V2, distance);  // Send distance to Virtual Pin V2 (optional)
}

/**
 * Function to display text on the OLED screen
 * @param text The string to display
 * @param x The x-coordinate for the starting position
 * @param y The y-coordinate for the starting position
 * @param textSize The size of the text (1 = small, 2 = medium, etc.)
 */
void displayText(const char *text, int x, int y, int textSize) {
  display.clearDisplay();            // Clear the display buffer
  display.setTextSize(textSize);     // Set the text size
  display.setTextColor(SH110X_WHITE); // Set text color
  display.setCursor(x, y);           // Set cursor position
  display.println(text);             // Print the text
  display.display();                 // Display the updated buffer
}

/**
 * Function to handle Blynk virtual pin V5 for enabling/disabling the buzzer
 */
BLYNK_WRITE(V5) {
  int value = param.asInt(); // Get the value from the Blynk app (0 or 1)
  buzzerEnabled = (value == 1); // Enable buzzer if 1, disable if 0
  Serial.print("Buzzer Enabled: ");
  Serial.println(buzzerEnabled ? "Yes" : "No");
}
