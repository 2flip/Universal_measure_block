#include <Arduino.h>
#include<Wire.h>


//I2C device found at address 0x68
//I2C device found at address 0x7E
#define ICM_0       0x68    //addres icm_0
#define ICM_1               //addres icm_1   
#define SCL         16
#define SDA         17 

uint8_t reg{}, accel_setup{};


//Addr_Device = Device Address; Reg = Register; N = Numbers of byte, but size is less then 32 bytes
uint8_t readRegister(uint8_t Addr_Device, uint8_t Reg, uint8_t N){

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
void writeRegister(uint8_t Addr_Device,uint8_t Reg,uint8_t value){
    Wire.beginTransmission(Addr_Device);
    Wire.write(Reg);
    Wire.write(value);
    Wire.endTransmission(true);
}



//after succesfully write data on sd card we will be wire.end();

void setup(){
    Serial.begin(115200);
    //Wire.begin(SDA, SCL)
    Wire.begin(SDA, SCL);
    Wire.setClock(1000*1000); //1MHz

    Serial.print("Check ICM:    ");
    Serial.println(readRegister(ICM_0, 0x75, 1));


    //set ODR 1KHz
    reg = readRegister(ICM_0, 0x50, 1);
    reg = 0x06 | (reg & 0xF0);
    writeRegister(ICM_0, 0x50, reg);

    //Check ODR setting
    accel_setup = readRegister(ICM_0, 0x50, 1);
    accel_setup = (accel_setup & 0x0F); 
    Serial.println(accel_setup, BIN);

    //set FS +-16 g
    reg = readRegister(ICM_0, 0x50, 1);
    reg = (0x00 <<5) | (reg & 0x1F);
    writeRegister(ICM_0, 0x50, reg);

    //check FS setting
    accel_setup = readRegister(ICM_0, 0x50, 1);
    accel_setup = (accel_setup & 0xE0)>>5;
    Serial.println(accel_setup, BIN);







     
    


}


void loop(){

}