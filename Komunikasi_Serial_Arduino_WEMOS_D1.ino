#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//SoftwareSerial nodemcu(15, 13); //D8 = Rx & D7 = Tx //Untuk ESP8266
SoftwareSerial nodemcu(D3, D2); //Untuk WemosD1

String kondisi;
String kondisi2;

const char *ssid =  "Pondok Mushofa";     // replace with your wifi ssid and wpa2 key
const char *pass =  "lunasawalbulan";
const char* server = "http://kebundurian.online/postData/durian";

WiFiClient client;
HTTPClient httpClient;


void setup() {
  // Inisialisasi Serial Port
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;

  // Inisalisasi WIFI
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  
  StaticJsonBuffer<3000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
//    Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }
  //Pembacaan data serial dari arduino mega
  //Sensor Kelembaban Tanah
  int kelembaban = data["analog1"];
    if(kelembaban >= 435){
     kondisi = ("Tanah Kering");
    }
    else if(kelembaban >= 235 && kelembaban <=435){
     kondisi = ("Tanah Lembab");
    }
    else if(kelembaban <= 235){
     kondisi = ("Tanah Basah");
    }
    
  //Sensor PH Tanah
  float ph = data["analog2"];
  float ph2 = data["analog3"];
    if(ph > 7 ){
     kondisi2 = ("PH Basa");
    }
    else if(ph >=6 && ph <=7){
     kondisi2 = ("PH Normal");
    }
    else if(ph < 6){
     kondisi2 = ("PH Asam");
  }

  if (isnan(kelembaban)||isnan(ph2)) {
    Serial.println("Gagal membaca data sensor");
    return;
  }

  //Pengiriman Data Sensor Dari Arduino WemosD1 Ke WebServer
  if (client.connect(server,80)) {   
    
    String data = "kelembapan=" + String(kelembaban) + "&ph=" + String(ph2) + "&humi=" + "&tempe=" + "&kondisi=" + String(kondisi) + "&kondisi2=" + String(kondisi2) + "";
    httpClient.begin(client, server);
    httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpClient.POST(data);
    String content = httpClient.getString();
    httpClient.end();
    Serial.println(content);
    delay(5000);           

    Serial.print("Kelembaban Tanah: ");
    Serial.print(" || ");
    Serial.println(kelembaban);
    Serial.print("PH Tanah: ");
    Serial.print(" || ");
    Serial.println(ph2);
   }                                             
//    client.stop();
  
}


  
