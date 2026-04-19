#include    <Arduino.h>
#include    <FS.h>
#include    <SD.h>
#include    <SPI.h>
#include    <ESP32_SoftWire.h>

//ICM pins
#define ICM_0_ADD   0x68    //addres icm_0
#define ICM_1_ADD   1       //addres icm_1   
#define ICM_SCL     4       //correct problems with 
#define ICM_SDA     5       //
#define ICM_INT_0   6
#define ICM_INT_1   8

//SD card pins
#define SD_CS       39
#define SD_MISO     37
#define SD_MOSI     35
#define SD_CLK      36


//registers ICM42688
#define UB0_REG_INT_CONFIG1     0x64
#define UB0_REG_INT_CONFIG      0x14
#define UB0_REG_INT_SOURCE0     0x65
#define UB0_REG_TEMP_DATA1      0x1D

size_t _numBytes = 0;

//volatile bool dataReady0 = false;  //ALARM AFTE TEST REMOVE THIS
volatile bool FLAG_SOFT = false;
bool dataReady0 = true;
bool dataReady1 = true;
//INTERRUPT FOR ESP32
void IRAM_ATTR setICMFlag()
{
  dataReady0 = true;
}


void IRAM_ATTR softSave(){
    FLAG_SOFT = true;
}

SoftWire i2c;
File dataFile;

SPIClass sd_spi(HSPI);
SPISettings setting(1000000,  MSBFIRST,SPI_MODE0);

uint8_t reg{}, accel_setup{}, temp{};


uint8_t _buffer[15] = {};
int16_t _rawT      = 0;
int16_t _rawAcc[3] = {};
int16_t _rawGyr[3] = {};
float _t      = 0.0f;



//Addr_Device = Device Address; Reg = Register; N = Numbers of byte, but size is less then 32 bytes
uint8_t readRegisterI2C(uint8_t Addr_Device, uint8_t Reg, uint8_t N){

    uint8_t res{};
    i2c.beginTransmission(Addr_Device);
    i2c.write(Reg);

    //check error transmissiobn
    res = i2c.endTransmission(true);

    delay(10);
 
    i2c.requestFrom(Addr_Device, N);
    if(i2c.available()==N){
        res = i2c.read();

    }
    
    return(res);
        
}


//Addr_Device = Device Address; Reg = Register; value = value of register
void writeRegisterI2C(uint8_t Addr_Device,uint8_t Reg,uint8_t value){
    i2c.beginTransmission(Addr_Device);
    i2c.write(Reg);
    i2c.write(value);
    i2c.endTransmission(true);
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





int readRegisterMultiply(uint8_t Addr_Device, uint8_t Reg, uint8_t count, uint8_t* dest){
    i2c.beginTransmission(Addr_Device);
    i2c.write(Reg);
    i2c.endTransmission(true);
    _numBytes = i2c.requestFrom(Addr_Device, count);
    if(_numBytes == count){
        for(uint8_t i{}; i<count; i++){
            dest[i] = i2c.read();
        }
        return(1);
    }else{
        return(-1);
    }
}


int getRawAGT(uint8_t Addr_Device){
    if(readRegisterMultiply(Addr_Device, UB0_REG_TEMP_DATA1, 14, _buffer)<0){
        return(-1);
    }

    int16_t rawMeas[7];
    for(size_t i = 0; i<7; i++){
        rawMeas[i] = ((int16_t)_buffer[i * 2] << 8) | _buffer[i * 2 + 1];
    }

	_rawT      = rawMeas[0];
	_rawAcc[0] = rawMeas[1];
	_rawAcc[1] = rawMeas[2];
	_rawAcc[2] = rawMeas[3];
	_rawGyr[0] = rawMeas[4];
	_rawGyr[1] = rawMeas[5];
	_rawGyr[2] = rawMeas[6];
}



bool FLAG_BUFFER_0{false} , FLAG_BUFFER_1{false};

void enableDataReadyInterrupt(){
    writeRegisterI2C(ICM_0_ADD, UB0_REG_INT_CONFIG, 0x18|0x03);
    uint8_t reg;
    reg = readRegisterI2C(ICM_0_ADD, UB0_REG_INT_CONFIG1, 1);
    reg &= ~0x10;
    writeRegisterI2C(ICM_0_ADD, UB0_REG_INT_CONFIG1, reg);
    writeRegisterI2C(ICM_0_ADD, UB0_REG_INT_SOURCE0, 0x18);
}

void fillPacket(uint8_t Addr_Device_0, uint8_t Addr_Device_1){
    if(FLAG_BUFFER_0<256){
        if(dataReady0 && dataReady1){
            Serial.println(FLAG_BUFFER_0);
            dataReady0  =   false;
            //dataReady1  =   false;//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ

            getRawAGT(Addr_Device_0);
            accel_0.ax_0 = _rawAcc[0];
            accel_0.ay_0 = _rawAcc[1];
            accel_0.az_0 = _rawAcc[2];
            packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
            packet0.time[FLAG_BUFFER_0]= micros();

            getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
            accel_1.ax_1 = _rawAcc[0];
            accel_1.ay_1 = _rawAcc[1];
            accel_1.az_1 = _rawAcc[2];
            packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;
            FLAG_BUFFER_0++;

        }  
   }
   if(FLAG_BUFFER_1<256){
        Serial.println(FLAG_BUFFER_1);
        dataReady0  =   false;
        //dataReady1  =   false;//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ

        getRawAGT(Addr_Device_0);
        accel_0.ax_0 = _rawAcc[0];
        accel_0.ay_0 = _rawAcc[1];
        accel_0.az_0 = _rawAcc[2];
        packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
        packet0.time[FLAG_BUFFER_0]= micros();

        getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
        accel_1.ax_1 = _rawAcc[0];
        accel_1.ay_1 = _rawAcc[1];
        accel_1.az_1 = _rawAcc[2];
        packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;
        FLAG_BUFFER_1++;

   }
}

void fillPacket_TEST_FUNCTION_ALARM(uint8_t Addr_Device_0, uint8_t Addr_Device_1){
    if(FLAG_BUFFER_0<256 &&(dataReady0 && dataReady1)){
        Serial.println(FLAG_BUFFER_0);
        dataReady0  =   false;
        //dataReady1  =   false;//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ

        getRawAGT(Addr_Device_0);
        accel_0.ax_0 = 0;
        accel_0.ay_0 = 1;
        accel_0.az_0 = 2;
        packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
        packet0.time[FLAG_BUFFER_0]= micros();

        getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
        accel_1.ax_1 = 3;
        accel_1.ay_1 = 4;
        accel_1.az_1 = 5;
        packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;
        FLAG_BUFFER_0++;
    }
    
   if(FLAG_BUFFER_1<256 &&(dataReady0 && dataReady1)){
        Serial.println(FLAG_BUFFER_1);
        dataReady0  =   false;
        //dataReady1  =   false;//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ

        getRawAGT(Addr_Device_0);
        accel_0.ax_0 = 0;
        accel_0.ay_0 = 1;
        accel_0.az_0 = 2;
        packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
        packet0.time[FLAG_BUFFER_0]= micros();

        getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
        accel_1.ax_1 = 3;
        accel_1.ay_1 = 4;
        accel_1.az_1 = 5;
        packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;
        FLAG_BUFFER_1++;

   }
}

void fillPacket_2(uint8_t Addr_Device_0, uint8_t Addr_Device_1){
    if(FLAG_BUFFER_0 == false){
        for(uint i{}; i<256; i++){
            if(dataReady0){
                Serial.println(i);
                getRawAGT(Addr_Device_0);
                accel_0.ax_0 = 0;
                accel_0.ay_0 = 1;
                accel_0.az_0 = 2;
                packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
                
                getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
                accel_1.ax_1 = 3;
                accel_1.ay_1 = 4;
                accel_1.az_1 = 5;
                packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;

                packet0.time[FLAG_BUFFER_0]= micros();
            }

            
        }
        FLAG_BUFFER_0 = true;
        



    }else if(FLAG_BUFFER_1 == false && FLAG_BUFFER_0 == true){
        Serial.println("bufer0 is busy");
        for(uint i{}; i<256; i++){
            if(dataReady0){
                Serial.println(i);
                getRawAGT(Addr_Device_0);
                accel_0.ax_0 = 0;
                accel_0.ay_0 = 1;
                accel_0.az_0 = 2;
                packet0.a_massiv_0[FLAG_BUFFER_0] = accel_0;
                packet0.time[FLAG_BUFFER_0]= micros();

                getRawAGT(Addr_Device_0);//УБРАТЬ ДЕВАЙС 0 ИЗ ЭТОЙ ШТУКИ
                accel_1.ax_1 = 3;
                accel_1.ay_1 = 4;
                accel_1.az_1 = 5;
                packet0.a_massiv_1[FLAG_BUFFER_0] = accel_1;
            }
            

        }
        FLAG_BUFFER_1 = true; 
           
        
    }

}

void setup(){
    Serial.begin(115200);

    pinMode(ICM_INT_0, INPUT_PULLDOWN);//interrupt pins
    attachInterrupt(ICM_INT_0, setICMFlag, RISING);
    pinMode(20, OUTPUT);
    pinMode(47, INPUT_PULLDOWN);
    attachInterrupt(47, softSave, RISING);

    //setup I2C
    i2c.begin(ICM_SDA, ICM_SCL);//Wire.begin(SDA, SCL)
    i2c.setClock(1000*1000); //

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

    enableDataReadyInterrupt();

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
    writeRegisterI2C(ICM_0_ADD, 0x4E, 0x0F);  

    fillPacket_TEST_FUNCTION_ALARM(ICM_0_ADD, ICM_1_ADD);
    


}

void loop(){

    // if(FLAG_SOFT){
    //     dataFile.flush();
    //     dataFile.close();       
    //     while(1){
    //         Serial.println("The file was saved successfully");
	// 		delay(10000);
    //     }

    // }else{
    //     fillPacket_TEST_FUNCTION_ALARM(ICM_0_ADD, ICM_1_ADD);
    //     if(FLAG_BUFFER_0 = 255){
    //         dataFile.write((uint8_t*)&packet0, 4096);
    //         FLAG_BUFFER_0 = 0;
    //     }
    //     if(FLAG_BUFFER_1 = 255){
    //         dataFile.write((uint8_t*)&packet1, 4096);
    //         FLAG_BUFFER_1 = 0;

    //     }
    // }   
    fillPacket_2(ICM_0_ADD, ICM_1_ADD);

    if(FLAG_BUFFER_0 ==true){
        dataFile.write((uint8_t*)&packet0, 4096);
        FLAG_BUFFER_0 = false;
    }else if(FLAG_BUFFER_1 == true){
        dataFile.write((uint8_t*)&packet1, 4096);
        FLAG_BUFFER_0 = false;

    }else if(FLAG_SOFT == true){
        dataFile.flush();
        dataFile.close();
        while(1){
            Serial.println("data is save");
            delay(500);
        }


    }
    
}
