#include    <Arduino.h>
#include    <Wire.h>
//#include    <FS.h>
//#include    <SD.h>
//#include    <SPI.h>
//#include    <Adafruit_NeoPixel.h>

//Adafruit_NeoPixel LED_RGB(1, 48, NEO_GRBW + NEO_KHZ800);

//ICM pins
#define ICM_0_ADD   0x68    //addres icm_0
#define ICM_1_ADD           //addres icm_1   
#define ICM_SCL     4      //correct problems with 
#define ICM_SDA     5      //
#define ICM_INT_0   12
#define ICM_INT_1

//SD card pins
#define SD_CS       39
#define SD_MISO     37
#define SD_MOSI     35
#define SD_CLK      36


bool newICM42688Data = false;
 
// File dataFile;

// SPIClass sd_spi(HSPI);
// SPISettings setting(1000000,  MSBFIRST,SPI_MODE0);

uint8_t reg{}, accel_setup{}, temp{};

void myinthandler1()
{
  newICM42688Data = true;
}



//Addr_Device = Device Address; Reg = Register; N = Numbers of byte, but size is less then 32 bytes
uint8_t readRegisterI2C(uint8_t Addr_Device, uint8_t Reg, uint8_t N){

    uint8_t res{};
    Wire.beginTransmission(Addr_Device);
    Wire.write(Reg);

    //check error transmissiobn
    res = Wire.endTransmission(true);

    delay(10);
 
    Wire.requestFrom(Addr_Device, N);
    if(Wire.available()==N){
        res = Wire.read();

    }
    
    //Wire.endTransmission(true);
    return(res);
        
}


//Addr_Device = Device Address; Reg = Register; value = value of register
void writeRegisterI2C(uint8_t Addr_Device,uint8_t Reg,uint8_t value){
    Wire.beginTransmission(Addr_Device);
    Wire.write(Reg);
    Wire.write(value);
    Wire.endTransmission(true);
}


struct packet_0{
    int16_t ax_0,ay_0,az_0;  
}accel_0;

struct packet_1{
    int16_t ax_1,ay_1,az_1;  
}accel_1;


struct dataAccel{
    unsigned long time  [256];    //1 element massive has weight 4 bytes
    packet_0 a_massiv_0 [256];   //1 element massive has weight 6 bytes
    packet_1 a_massiv_1 [256];   //1 element massive has weight 6 bytes
};

dataAccel packet0, packet1;


uint8_t ICM_DRstatus(){
    uint8_t temp = readRegisterI2C(ICM_0_ADD, 0x2D,1);
    return temp;

}


//after succesfully write data on sd card we will be wire.end();

void setup(){
    Serial.begin(115200);

    // LED_RGB.begin();
    // LED_RGB.setBrightness(100);
    // LED_RGB.setPixelColor(0, uint32_t(LED_RGB.Color(244, 255, 29)));
    // LED_RGB.show();

    pinMode(ICM_INT_0, INPUT);//interrupt pins
    attachInterrupt(ICM_INT_0, myinthandler1, RISING);


    //setup I2C
    Wire.begin(ICM_SDA, ICM_SCL);//Wire.begin(SDA, SCL)
    Wire.setClock(1000*1000); //1MHz

    //check whoami icm
    Serial.print("Check ICM:    ");
    Serial.println(readRegisterI2C(ICM_0_ADD, 0x75, 1));


    //set ODR 1KHz
    reg = readRegisterI2C(ICM_0_ADD, 0x50, 1);
    reg = 0x06 | (reg & 0xF0);
    writeRegisterI2C(ICM_0_ADD, 0x50, reg);

    
    //Check ODR setting
    accel_setup = readRegisterI2C(ICM_0_ADD, 0x50, 1);
    accel_setup = (accel_setup & 0x0F); 
    Serial.println(accel_setup, BIN);


    //set FS +-16 g
    reg = readRegisterI2C(ICM_0_ADD, 0x50, 1);
    reg = (0x00 <<5) | (reg & 0x1F);
    writeRegisterI2C(ICM_0_ADD, 0x50, reg);


    //check FS setting
    accel_setup = readRegisterI2C(ICM_0_ADD, 0x50, 1);
    accel_setup = (accel_setup & 0xE0)>>5;
    Serial.println(accel_setup, BIN);


    //turn low noise mode on accel
    writeRegisterI2C(ICM_0_ADD,0x4E, 0x0F);

    //push-pull, pulsed, active HIGH interrupts  
    // writeRegisterI2C(ICM_0_ADD, 0x14,0x18 | 0x03);
    // temp = readRegisterI2C(ICM_0_ADD,0x64, 1);
    // writeRegisterI2C(ICM_0_ADD, 0x64,temp&~(0x10));
    // writeRegisterI2C(ICM_0_ADD, 0x65, 0x08);








    //init sd card
    // sd_spi.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
    // if(!SD.begin(SD_CS, sd_spi)){
    //     Serial.println("SD card not work!");
    //     while(1);
    // }
    // dataFile = SD.open("/data.bin", FILE_WRITE);
    
    // if (!dataFile) {
    //     Serial.println("file faild");
    //     while(1);
    // }
    
    
    
    //enable icm
    writeRegisterI2C(ICM_0_ADD, 0x4E, 0x0F);

    // LED_RGB.setPixelColor(0, uint32_t(LED_RGB.Color(0, 89, 255)));
    // LED_RGB.show();


    // //ICM_DRstatus();
    if(newICM42688Data = true){
        Serial.println("INTERRUPT WORK !!!!!");
    }
    
    // RUSSIA();






     
    


}


void loop(){

}