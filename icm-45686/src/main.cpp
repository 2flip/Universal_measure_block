#include <Arduino.h>
#include <SPI.h>

//pins icm-45686
#define ICM_MISO  13
#define ICM_MOSI  11
#define ICM_SCLK  12
#define ICM_CS    5
#define ICM_INT_1 15  //remove for test


SPIClass SPI_ICM(FSPI);
SPISettings settings(5000000, MSBFIRST, SPI_MODE3);



void writeReg(uint8_t reg, uint8_t val)
{
  digitalWrite(ICM_CS, LOW);

  SPI.transfer(reg & 0x7F);
  SPI.transfer(val);

  digitalWrite(ICM_CS, HIGH);
}


uint8_t readReg(uint8_t reg)
{
  digitalWrite(ICM_CS, LOW);

  SPI.transfer(reg | 0x80);
  uint8_t val = SPI.transfer(0x00);

  digitalWrite(ICM_CS, HIGH);
  return val;
  
}


uint16_t read16(uint8_t reg)
{
  digitalWrite(ICM_CS, LOW);

  SPI.transfer(reg | 0x80);
  uint8_t high = SPI.transfer(0x00);
  uint8_t low = SPI.transfer(0x00);

  digitalWrite(ICM_CS, HIGH);

  return(high<<8) | low;
}


volatile bool dataReady = false;
void onDataReady()
{
  dataReady = true;
}





void setup()
{
  Serial.begin(115200);
  delay(2000);

  pinMode(ICM_CS, OUTPUT);
  digitalWrite(ICM_CS, HIGH);

  SPI.begin(ICM_SCLK, ICM_MISO, ICM_MOSI, ICM_CS);
  Serial.println("start init");

  attachInterrupt(digitalPinToInterrupt(ICM_INT_1), onDataReady, RISING);
  writeReg(0x1F, 0x01);
  delay(100);
  writeReg(0x20, 0x8F);
  delay(50);

  // 🔥 3. (опционально) включить interrupt data ready
  writeReg(0x30, 0x01);
  
  // writeReg(0x1F, 0x47);
  // writeReg(0x20, 0x05);
  // writeReg(0x20, 0x0F);
  //writeReg(0x30, 0x01);



  uint8_t whoami = readReg(0x00);
  Serial.println(whoami, HEX);
  Serial.println(whoami, HEX);
  Serial.println(whoami, HEX);


  SPI.transfer(0x80);
  SPI.transfer(0x00); // dummy
  uint8_t val = SPI.transfer(0x00);

  
}
void loop(){
  if(dataReady){
    dataReady=false;


    int16_t ax = read16(0x10);
    int16_t ay = read16(0x12);
    int16_t az = read16(0x14);

    Serial.print(millis());
    Serial.print(" ");
    Serial.print(ax);
    Serial.print(" ");
    Serial.print(ay);
    Serial.print(" ");
    Serial.println(az);
    delay(500);    
  }
  

 
  
  
  
}