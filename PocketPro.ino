#include <Adafruit_GFX.h>     
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include <SPI.h>
#include <Wire.h>

#define LCD_CS A3  // Chip Select
#define LCD_CD A2  // Command/Data
#define LCD_WR A1  // LCD Write
#define LCD_RD A0  // LCD Read
#define LCD_RESET A4 // Reset

// Colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x7BEF

// Touchscreen pins
#define YP A1  // Must be an analog pin
#define XM A2  // Must be an analog pin
#define YM 7   // Can be a digital pin
#define XP 6   // Can be a digital pin

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

bool isConnected = false;
bool connectionLost = false;
unsigned long lastDataTime = 0;
unsigned long checkInterval = 15000;  // 15 seconds for connection timeout
unsigned long reconnectTimeout = 5000; // 5 seconds for retrying connection

String currentQuote = "";
String currentTime = "";
String currentDate = "";  // New variable for storing the date

void setup() {
  Serial.begin(9600);
  tft.reset();
  tft.begin(0x9341); // ILI9341 Driver
  tft.setRotation(3);

  displayWelcomeAnimation();
  delay(1000);

  lastDataTime = millis();
  displayLoadingScreen();
}

void displayWelcomeAnimation() {
  tft.fillScreen(BLACK);
  tft.setTextColor(CYAN);
  tft.setTextSize(3);

  for (int i = 0; i <= 255; i += 5) {
    tft.setCursor(40, 100);
    tft.setTextColor(tft.color565(0, i, i)); // Fade-in effect in cyan
    tft.print("PocketPro");
    delay(50);
  }
  delay(1000);
  tft.fillScreen(BLACK);
}

void displayLoadingScreen() {
  tft.fillScreen(BLACK);
  tft.setCursor(50, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Connecting...");

  int dotPosX = 60;
  int dotY = 200;
  int numDots = 3;

  for (int i = 0; i < 15; i++) {
    tft.fillRect(dotPosX - 20, dotY - 10, 60, 20, BLACK);
    for (int j = 0; j < numDots; j++) {
      int x = dotPosX + (j * 10);
      tft.fillCircle(x, dotY, 3, (i % numDots == j) ? WHITE : GREY);
    }
    delay(300);
    if (Serial.available()) return; // Exit if connection established
  }
}

void displayConnectionLostScreen() {
  tft.fillScreen(RED);
  tft.setCursor(20, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Connection Lost");

  tft.setCursor(30, 130);
  tft.print("Retrying...");

  for (int i = 0; i < 3; i++) {
    tft.setTextColor(i % 2 == 0 ? WHITE : GREY);
    tft.setCursor(30, 160);
    tft.print("Please wait");
    delay(500);
  }
}

void displayConnectedScreen() {
  tft.fillScreen(GREEN);
  tft.setCursor(30, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("PC Connected");
  delay(1000);
}

void displaySystemInfo(String cpu, String ram, String storage) {
  tft.fillScreen(BLACK);
  tft.setCursor(10, 30);
  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.print("CPU: " + cpu + "%");

  tft.setCursor(10, 60);
  tft.setTextColor(YELLOW);
  tft.print("RAM: " + ram + "%");

  // Display Storage usage on next line
  tft.setCursor(10, 90);
  tft.setTextColor(MAGENTA);
  tft.print("Storage: " + storage + "%");

  displayClockAndQuote(); // Always display time and quote after system info
}

void displayClockAndQuote() {
  // Clear previous clock and quote area
  tft.fillRect(10, 130, 300, 100, BLACK); 

  // Display Date
  tft.setCursor(10, 130);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.print("Date: " + currentDate);

  // Display Time
  tft.setCursor(10, 160);
  tft.print("Time: " + currentTime);

  // Display Quote
  tft.setCursor(10, 190);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);  // Smaller size for quote
  tft.print("Quote:");
  
  // Clear previous quote area and print it
  tft.fillRect(10, 210, 300, 50, BLACK); // Clear quote area
  tft.setCursor(10, 210);
  tft.print(currentQuote); // Print quote
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.println("Received: " + data); // Debug print to Serial Monitor

    if (data.startsWith("CONNECTED")) {
      if (!isConnected) {
        displayConnectedScreen();
        isConnected = true;
        connectionLost = false;
      }
      lastDataTime = millis();

      // Parse the data for system info
      int cpuIndex = data.indexOf("CPU:") + 4;
      int ramIndex = data.indexOf("RAM:") + 4;
      int storageIndex = data.indexOf("STORAGE:") + 8;

      String cpu = data.substring(cpuIndex, data.indexOf('%', cpuIndex));
      String ram = data.substring(ramIndex, data.indexOf('%', ramIndex));
      String storage = data.substring(storageIndex, data.indexOf('%', storageIndex));

      displaySystemInfo(cpu, ram, storage);
    } else if (data.startsWith("TIME:")) {
      currentTime = data.substring(5);
      displayClockAndQuote(); // Update clock display
    } else if (data.startsWith("DATE:")) {  // New case for date data
      currentDate = data.substring(5);
      displayClockAndQuote(); // Update date display
    } else if (data.startsWith("QUOTE:")) {
      currentQuote = data.substring(5);
      displayClockAndQuote(); // Update quote display
    }
  }

  // Check for connection timeout
  if (isConnected && (millis() - lastDataTime > reconnectTimeout)) {
    displayConnectionLostScreen();
    isConnected = false;
    connectionLost = true;
  }

  // Check for longer timeout for no data
  if (!isConnected && millis() - lastDataTime > checkInterval) {
    displayConnectionLostScreen();
    connectionLost = true;
  }
}
