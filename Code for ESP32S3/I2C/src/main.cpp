#include <Arduino.h>
#include<Wire.h>
#include<FS.h>
#include<SD.h>
#include<SPI.h>


#define ICM_0       0x68    //addres icm_0
#define ICM_1               //addres icm_1   
#define SCL         16
#define SDA         17
#define INT_0       18
#define INT_1


#define SD_CS       39
#define SD_MISO     37
#define SD_MOSI     35
#define SD_CLK      36


File dataFile;

SPIClass sd_spi(HSPI);
SPISettings setting(1000000,  MSBFIRST,SPI_MODE0);

uint8_t reg{}, accel_setup{};


//Addr_Device = Device Address; Reg = Register; N = Numbers of byte, but size is less then 32 bytes
uint8_t readRegisterI2C(uint8_t Addr_Device, uint8_t Reg, uint8_t N){

    uint8_t res{};
    Wire.beginTransmission(Addr_Device);
    Wire.write(Reg);

    //check error transmissiobn
    res = Wire.endTransmission(false);

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

//after succesfully write data on sd card we will be wire.end();

void setup(){
    Serial.begin(115200);

    //setup I2C
    Wire.begin(SDA, SCL);//Wire.begin(SDA, SCL)
    Wire.setClock(1000*1000); //1MHz

    //check whoami icm
    Serial.print("Check ICM:    ");
    Serial.println(readRegisterI2C(ICM_0, 0x75, 1));


    //set ODR 1KHz
    reg = readRegisterI2C(ICM_0, 0x50, 1);
    reg = 0x06 | (reg & 0xF0);
    writeRegisterI2C(ICM_0, 0x50, reg);

    //Check ODR setting
    accel_setup = readRegisterI2C(ICM_0, 0x50, 1);
    accel_setup = (accel_setup & 0x0F); 
    Serial.println(accel_setup, BIN);

    //set FS +-16 g
    reg = readRegisterI2C(ICM_0, 0x50, 1);
    reg = (0x00 <<5) | (reg & 0x1F);
    writeRegisterI2C(ICM_0, 0x50, reg);

    //check FS setting
    accel_setup = readRegisterI2C(ICM_0, 0x50, 1);
    accel_setup = (accel_setup & 0xE0)>>5;
    Serial.println(accel_setup, BIN);



    //init sd card
    sd_spi.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
	if(!SD.begin(SD_CS, sd_spi)){
		Serial.println("SD card not work!");
		while(1);
	}
	dataFile = SD.open("/data.bin", FILE_WRITE);
	if (!dataFile) {
		Serial.println("file faild");
		while(1);
	
	}
    
    
    
    //enable icm
    writeRegisterI2C(ICM_0, 0x4E, 0x0F);







     
    


}


void loop(){

}