#include  <Arduino.h>
#include  <SPI.h>
#include  <SD.h>
#include  <FS.h>
#include  <esp_timer.h>


//ICM block pins
#define ICM_CS_0      10
#define ICM_CS_1      7     //for test without icm_cs_1
#define ICM_0_INT_1   7
#define ICM_1_INT_1   8
#define ICM_MISO      13 
#define ICM_MOSI      11
#define ICM_CLK       12


//SD block pins
#define SD_CS         39
#define SD_MISO       37
#define SD_MOSI       35
#define SD_CLK        36

//pins jumper
#define JUMP          47

//technical block
#define RES_FULL    0x00   //sensevity 16g force
#define ODR_V       0x05   //ODR set 2000hz
#define READ        0x80   //1 
#define WRITE       0x00   //0  


SPIClass icm_spi(FSPI);
SPIClass sd_spi(HSPI);
SPISettings setting(1000000,  MSBFIRST,SPI_MODE0);

//global variables for sd card

//check status bottom 
volatile bool status;


#define buffer_cicle 256


int32_t bufferA[4096];
uint8_t bufferB[4096];
uint8_t massivIndex {};

File dataFile;

//haven't interrupt on icm42688 it's hotfix
unsigned long last_time{};

void safetyWriteData(){
	status = 1;
}


byte readRegister(byte regAddr, uint8_t CS_PIN){

    icm_spi.beginTransaction(setting);

    digitalWrite(CS_PIN, LOW);  
    icm_spi.transfer(regAddr | READ);
    byte result = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);

    icm_spi.endTransaction();

    return result;

}


void writeRegister(byte regAddr, uint8_t value, uint8_t CS_PIN){
	icm_spi.beginTransaction(setting);

	digitalWrite(CS_PIN,LOW);
	icm_spi.transfer(regAddr);
	icm_spi.transfer(value);
	digitalWrite(CS_PIN, HIGH);

	icm_spi.endTransaction();

}


void readAccelRaw(int16_t &ax, int16_t &ay, int16_t &az, uint8_t CS_PIN){
	if(micros()- last_time>500){
		last_time = micros();
		int16_t buffer[6];
		icm_spi.beginTransaction(setting);

		digitalWrite(CS_PIN, LOW);
		icm_spi.transfer(0x1F | READ);
		for (int i = 0; i < 6; i++) {
			buffer[i] = icm_spi.transfer(0x00);
	}
	digitalWrite(CS_PIN, HIGH);
	icm_spi.endTransaction();
	ax = (int16_t)((buffer[0] << 8) | buffer[1]);
	ay = (int16_t)((buffer[2] << 8) | buffer[3]);
	az = (int16_t)((buffer[4] << 8) | buffer[5]);
	}
}


struct packet_0{
    int16_t ax_0,ay_0,az_0;  
}accel_0;

struct packet_1{
    int16_t ax_1,ay_1,az_1;  
}accel_1;

struct dataAccel{
    unsigned long time[256];    //1 element massive has weight 4 bytes
    packet_0 a_massiv_0[256];   //1 element massive has weight 6 bytes
    packet_1 a_massiv_1[256];   //1 element massive has weight 6 bytes
}packet;





void fillPacket(){
  Serial.println("Start cycle");
  
  for(uint16_t i{0}; i<256; ++i){
        int16_t ax_0{}, ay_0{}, az_0{};
        int16_t ax_1{}, ay_1{}, az_1{};

        readAccelRaw(ax_0,ay_0,az_0,ICM_CS_0);
        readAccelRaw(ax_1,ay_1,az_1,ICM_CS_1);
    

        accel_0.ax_0 = ax_0;
        accel_0.ay_0 = ay_0;
        accel_0.az_0 = az_0;

        accel_1.ax_1 = ax_1;
        accel_1.ay_1 = ay_1;
        accel_1.az_1 = az_1;

        packet.a_massiv_0[i] = accel_0;
        packet.a_massiv_1[i] = accel_1;
        packet.time[i]       = micros();
        Serial.println(i);
  }
}

void setup(){
	//initialization Serial
	Serial.begin(115200);


	pinMode(JUMP, INPUT_PULLDOWN);
	digitalWrite(20, HIGH);
	Serial.print("Check 47: ");
	Serial.println(digitalRead(47));

	attachInterrupt(JUMP, safetyWriteData, RISING);


	//initilization interrupt
	// pinMode(ICM_0_INT_1, INPUT);
	// pinMode(ICM_1_INT_1, INPUT);
	// //attachInterrupt(ICM_0_INT_1, setImuFlag, RISING);
	// attachInterrupt(ICM_0_INT_1, setImuFlag, RISING);


	//initialization SPI
	Serial.println("Start init");
	icm_spi.begin(ICM_CLK, ICM_MISO, ICM_MOSI);
	//SPI.setFrequency(24 000 000);

	pinMode(ICM_CS_0, OUTPUT);
	digitalWrite(ICM_CS_0, HIGH);

	pinMode(ICM_CS_1, OUTPUT);
	digitalWrite(ICM_CS_1, HIGH);

	delay(100);
	
	//check work ICM-42688
	byte check_work_0 = readRegister(0x75, ICM_CS_0);
	byte check_work_1 = readRegister(0x75, ICM_CS_1);

	if (check_work_0 == 71){
		Serial.println("ICM_0 work correctly!");
	}
	else{
		Serial.println("ICM_0 not found!");
	}

	if (check_work_1 == 71){
		Serial.println("ICM_1 work correctly!");
	}
	else{
		Serial.println("ICM_1 not found!");
	}


	//set ODR
	byte reg = readRegister(0x50, ICM_CS_0);
	reg = ODR_V | (reg & 0xF0);
	writeRegister(0x50, reg, ICM_CS_0);

	reg = readRegister(0x50, ICM_CS_1);
	reg = ODR_V | (reg & 0xF0);
	writeRegister(0x50, reg, ICM_CS_1);

	//set ACCEL_FS
	reg = readRegister(0x50, ICM_CS_0);
	reg = (RES_FULL <<5) | (reg & 0x1F);
	writeRegister(0x50, reg, ICM_CS_0);

	reg = readRegister(0x50, ICM_CS_1);
	reg = (RES_FULL <<5) | (reg & 0x1F);
	writeRegister(0x50, reg, ICM_CS_1);

	delay(200);


	//check setup icm settings
	//get setup setting AccelFS
	byte accel_Config = readRegister(0x50, ICM_CS_0);
	byte accel_Range  = (accel_Config & 0xE0)>>5;
	Serial.print("ICM_0 RANGE:  ");
	Serial.println(accel_Range, BIN);

	accel_Config = readRegister(0x50, ICM_CS_1);
	accel_Range  = (accel_Config & 0xE0)>>5;
	Serial.print("ICM_1 RANGE:  ");
	Serial.println(accel_Range, BIN);


	//get setup setting ODR
	accel_Config = readRegister(0x50, ICM_CS_0);
	byte accel_ODR = (accel_Config & 0x0F);
	Serial.print("ICM_0 ODR:  ");
	Serial.println(accel_ODR, BIN);

	accel_Config = readRegister(0x50, ICM_CS_1);
	accel_ODR = (accel_Config & 0x0F);
	Serial.print("ICM_1 ODR:  ");
	Serial.println(accel_ODR, BIN);
	

	//start init sd card
	sd_spi.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
	if(!SD.begin(SD_CS, sd_spi)){
		Serial.println("SD card not work!");
		while(1);
	}
	File file = SD.open("/data.bin", FILE_WRITE);
	if (!file) {
		Serial.println("file faild");
		while(1);
	
	}

	// enable icm42688
	writeRegister(0x4E, 0x0F, ICM_CS_0);
	writeRegister(0x4E, 0x0F, ICM_CS_1);
	delay(500);


	fillPacket();
	Serial.print("Size packet:	");
	Serial.println(sizeof(packet));

	Serial.println("Start write on sd card");
	
	uint64_t a{},b{};

	
	a = micros();
	file.write((uint8_t*)&packet, 4096);
	b = micros();
	Serial.println(b-a);
	file.flush();
	file.close();
	
	






  
}


void loop(){
  // int16_t ax_0, ay_0, az_0;
  // int16_t ax_1, ay_1, az_1;

  
  // readAccelRaw(ax_0, ay_0, az_0, ICM_CS_0);
  // readAccelRaw(ax_1, ay_1, az_1, ICM_CS_1);

  // Serial.print(micros());
  // Serial.print(";");
  // Serial.print(ax_0);
  // Serial.print(";");
  // Serial.print(ay_0);
  // Serial.print(";");
  // Serial.print(az_0);

  // Serial.print(";");

  // Serial.print(ax_1);
  // Serial.print(";");
  // Serial.print(ay_1);
  // Serial.print(";");
  // Serial.print(az_1);
  // Serial.println(";");
  //Serial.println(status);
  // if(millis()- last_time > 1000){
    // last_time = millis();
    // Serial.println("every 1 second... maybe...");
// 
  // }
  // 
// 
  // delay(500);
}

