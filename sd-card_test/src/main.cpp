#include  <Arduino.h>
#include  <SPI.h>
#include  <SD.h>

//pins SD card
#define SD_MOSI  35
#define SD_MISO  37
#define SD_SCLK  36
#define SD_CS    39




File logFile;


SPIClass SPI_SD(HSPI);
SPIClass SPI_ADXL(FSPI);

SPISettings settings(5000000, MSBFIRST, SPI_MODE3);

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(115200);

  // Init sd card with pins
  SPI_SD.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);


  if (!SD.begin(SD_CS, SPI_SD)){
    Serial.println("SD init failed");
    while (1);
  }

  Serial.println("SD init is done!");

  //Check exist file, if this exist - make new with new number 
  int i{0};
  
  for(;(SD.exists(("/data" + String(i) + ".csv")) == 1); i++){
    Serial.print(i);
    Serial.println("  is exists");
  }

  Serial.println("data" + String(i) + ".csv");

  logFile = SD.open("/data" + String(i) + ".csv", FILE_WRITE);

  if (!logFile){
    Serial.println("File error");
    while (1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}