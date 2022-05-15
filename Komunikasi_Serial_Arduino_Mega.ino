#include <SimpleModbusMaster.h>
#include <avr/wdt.h>
#define baud 4800
#define timeout 3000
#define polling 100 // the scan rate
#define retry_count 1000
#define TxEnablePin 2
#define TOTAL_NO_OF_REGISTERS 200
#define Offset 0
#define pin_relay 14
#define max_pressure 30 //-------------------- > Sesuaikan dengan nilai maksimal pressure alat ukur 30
#define relay_on digitalWrite(pin_relay,LOW)
#define relay_off digitalWrite(pin_relay,HIGH)
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

//Initialise Arduino to NodeMCU (2=Rx & 3=Tx)
//SoftwareSerial nodemcu(2, 3); // Menggunakan pin 2 dan pin 3 pada arduino
SoftwareSerial nodemcu(2, 3);

//Kelembaban Tanah
const int dry = 640;   //Nilai analog sensor di udara 987
const int wet = 40;
int percentageHumididy;
int sensorPin1 = A0;    
int sensorValue1 = 0;  

//Ph Tanah
int sensorPin2 = A1; 
int analogph;
float sensorValue2 = 0;  

//Flowmeter, velocity, totalizer harian dan bulanan dan satuan kak
enum
{
  PACKET1,//
  TOTAL_NO_OF_PACKETS // leave this last entry
};
Packet packets[TOTAL_NO_OF_PACKETS];
//unsigned int regs[TOTAL_NO_OF_REGISTERS];
int regs[TOTAL_NO_OF_REGISTERS];
long last_millis1, last_millis2;
float sensor_ph;
bool output=1;
int data_masuk;

void setup()
{
  Serial.begin(9600);
  nodemcu.begin(9600);
//  delay(1000);
  Serial.println("Program started");
  pinMode(pin_relay,OUTPUT);
  relay_off;
//  Serial.begin(9600);
  Serial2.begin(baud);
  modbus_construct(&packets[PACKET1], 1, READ_HOLDING_REGISTERS, 0, 1, 0);//PH
  modbus_configure(&Serial2, baud, SERIAL_8N1, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
 }

void loop() {

  StaticJsonBuffer<5000> jsonBuffer; //Mengambil data
  JsonObject& data = jsonBuffer.createObject(); //Memisahkan data
  
  analog_func();

  //Data dari analog_func diberi nama
  data["analog1"] = sensorValue1;
  data["analog2"] = sensorValue2;
  data["analog3"] = sensor_ph; 
  data["analog4"] = percentageHumididy;

  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();

  delay(3000);
}

//Pembacaan hasil sensor yang digunakan
void analog_func() {
  //Kelembaban Tanah
  
  sensorValue1 = analogRead(sensorPin1);
  int percentageHumididy = map(sensorValue1, wet, dry, 0, 100); 
  Serial.print("Sensor 1 :");
  Serial.println(sensorValue1);
  //Ph Tanah
  analogph = analogRead(sensorPin2 );
  sensorValue2 = (analogph*(-0.002))-(-6.799);
  Serial.print("Sensor 2 :");
  Serial.println(sensorValue2);
  modbus_update();
  sensor_ph = regs[0];
  sensor_ph = sensor_ph/10;
   if (isnan(sensor_ph )) {
     sensor_ph = 0;
    }
  Serial.print("Sensor 3 :");
  Serial.println(sensor_ph);
  Serial.print("Sensor 4 :");
  Serial.println(percentageHumididy);
 
}
