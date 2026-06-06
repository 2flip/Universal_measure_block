#include "ICM45686.h"
#include<Arduino.h>


#define ACCEL_FSR_G     32				                //if change setting IMU, change this value!
#define GYRO_FSR_DPS    4000                            //if change setting IMU, change this value!
#define MAX_LSB         32768


#define ICM_ADD         0X68
#define ICM_INT         2


#define ACCEL_CONFIG0   0x1B            
#define PWR_MGMT0       0x10
#define GYRO_CONFIG0    0x1C


ICM456xx IMU(Wire,0);
void    enableDataReadyInterrupt();
void    writeRegisterI2C(uint8_t Addr_Device,uint8_t Reg,uint8_t value);
void IRAM_ATTR setICMFlag();
volatile bool dataReady = false;

struct main
{
    float time, ax, ay, az, gx,gy,gz,temp;

}packet;


void setup() {
    Serial.begin(576000);

    pinMode(ICM_INT, INPUT_PULLDOWN);
    attachInterrupt(ICM_INT, setICMFlag, RISING);
    
    IMU.begin();                                        //init IMU

    writeRegisterI2C(ICM_ADD, ACCEL_CONFIG0, 0x06);     //set 1.6kHz, 32g
    writeRegisterI2C(ICM_ADD, GYRO_CONFIG0,  0x06);     //set 1.6kHz, 4000dps
    writeRegisterI2C(ICM_ADD, PWR_MGMT0, 0x0F);         //set low noise, enable accel, gyro
    
    delay(100);
    enableDataReadyInterrupt();
}

void loop() {
    if(dataReady == true){
        inv_imu_sensor_data_t imu_data;
        float accel_g[3] = { 0 };
        float gyro_dps[3] = { 0 };
        float temp_degc;

        // Read registers
        IMU.getDataFromRegisters(imu_data);

        // Format data for Serial Plotter
        accel_g[0]  = (float)(imu_data.accel_data[0] * ACCEL_FSR_G) / MAX_LSB;
        accel_g[1]  = (float)(imu_data.accel_data[1] * ACCEL_FSR_G) / MAX_LSB;
        accel_g[2]  = (float)(imu_data.accel_data[2] * ACCEL_FSR_G) / MAX_LSB;
        gyro_dps[0]  = (float)(imu_data.gyro_data[0] * GYRO_FSR_DPS) / MAX_LSB;
        gyro_dps[1]  = (float)(imu_data.gyro_data[1] * GYRO_FSR_DPS) / MAX_LSB;
        gyro_dps[2]  = (float)(imu_data.gyro_data[2] * GYRO_FSR_DPS) / MAX_LSB;
        temp_degc = 25 + ((float)imu_data.temp_data/128);
        
        packet.time = micros();
        packet.ax = accel_g[0];
        packet.ay = accel_g[1];
        packet.az = accel_g[2];
        packet.gx = gyro_dps[0];
        packet.gy = gyro_dps[1];
        packet.gz = gyro_dps[2];
        packet.temp  = temp_degc;

        Serial.write(0xAA);
        Serial.write(0xBB);
        Serial.write((uint8_t*)&packet, sizeof(packet));        

        // Serial.print(micros());        
        // Serial.print(';');

        // Serial.print(accel_g[0]);        
        // Serial.print(';');

        // Serial.print(accel_g[1]);        
        // Serial.print(';');

        // Serial.print(accel_g[2]);        
        // Serial.print(';');
        
        // Serial.print(gyro_dps[0]);        
        // Serial.print(';');

        // Serial.print(gyro_dps[1]);        
        // Serial.print(';');

        // Serial.print(gyro_dps[2]);        
        // Serial.print(';');

        // Serial.println(temp_degc);        

        // Serial.print(">AccelX:");
        // Serial.println(accel_g[0]);
        // Serial.print(">AccelY:");
        // Serial.println(accel_g[1]);
        // Serial.print(">AccelZ:");
        // Serial.println(accel_g[2]);


        // Serial.print(">GyroX:");
        // Serial.println(gyro_dps[0]);
        // Serial.print(">GyroY:");
        // Serial.println(gyro_dps[1]);
        // Serial.print(">GyroZ:");
        // Serial.println(gyro_dps[2]);

        // Serial.print(">temp:");
        // Serial.println(temp_degc);




        dataReady = false;

    }
}


void enableDataReadyInterrupt(){
    writeRegisterI2C(ICM_ADD, INT1_CONFIG2, 0x01);//SET ACTIVE HIGH, PULSE MODE, PUSH PULL-UP
    writeRegisterI2C(ICM_ADD, INT1_CONFIG0, 0x84); //ENABLE DATAREADY
    writeRegisterI2C(ICM_ADD, INT1_STATUS0, 0x84);//hz che delaet, no nado
}

void writeRegisterI2C(uint8_t Addr_Device,uint8_t Reg,uint8_t value){
    Wire.beginTransmission(Addr_Device);
    Wire.write(Reg);
    Wire.write(value);
    Wire.endTransmission(true);
}

void IRAM_ATTR setICMFlag(){
    dataReady = true;
}