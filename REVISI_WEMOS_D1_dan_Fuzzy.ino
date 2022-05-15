#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Fuzzy.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
//SoftwareSerial nodemcu(15, 13); //D8 = Rx & D7 = Tx //Untuk ESP8266
SoftwareSerial nodemcu(D3, D2); //Untuk WemosD1

String kondisi;
String kondisi2;

const char *ssid =  "Pondok Mushofa";     // replace with your wifi ssid and wpa2 key
const char *pass =  "lunasawalbulan";
const char* server = "http://kebundurian.online/postData/durian";

WiFiClient client;
HTTPClient httpClient;

//pompa
int pompa1 = D7; //D5 
int pompa2 = D8; //D6

// Inisialisasi fuzzy objek
Fuzzy *fuzzy = new Fuzzy();

//Fuzzi set kelembaban
FuzzySet *basah         = new FuzzySet(40, 100, 200, 235);
FuzzySet *lembab        = new FuzzySet(236, 350, 430, 435);
FuzzySet *kering        = new FuzzySet(436, 550, 610, 630);

//Fuzzi set ph
FuzzySet *asam          = new FuzzySet(1, 3, 4, 5);
FuzzySet *netral        = new FuzzySet(6, 6, 7, 7);
FuzzySet *basa          = new FuzzySet(8, 10, 11, 14);

//Fuzzi output pompa1
FuzzySet *mati          = new FuzzySet(0, 3, 3, 4);
FuzzySet *jalan         = new FuzzySet(5, 5, 6, 7);
FuzzySet *jalan1        = new FuzzySet(8, 8, 9, 10);

//Fuzzi output pompa2
FuzzySet *mati2         = new FuzzySet(0, 2, 3, 4);
FuzzySet *jalan2        = new FuzzySet(5, 5, 6, 7);
FuzzySet *jalann2       = new FuzzySet(8, 8, 9, 10);


void setup() {
  // Inisialisasi Serial Port
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;

  // Inisalisasi WIFI
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Inisialisasi pin poma
  pinMode(pompa1, OUTPUT);
  pinMode(pompa2, OUTPUT);

  //Fuzzy Input Kelembaban 
  FuzzyInput *kelembaban = new FuzzyInput(1);

  kelembaban->addFuzzySet(basah);
  kelembaban->addFuzzySet(lembab);
  kelembaban->addFuzzySet(kering);
  
  fuzzy->addFuzzyInput(kelembaban);
  
  //Fuzzy Input ph
  FuzzyInput* ph = new FuzzyInput(2);

  ph->addFuzzySet(asam);
  ph->addFuzzySet(netral); 
  ph->addFuzzySet(basa);

  fuzzy->addFuzzyInput(ph); 

  //Fuzzy output pompa1 (Waktu Pengairan)
  FuzzyOutput* pompa1 = new FuzzyOutput(1);

  pompa1->addFuzzySet(mati); 
  pompa1->addFuzzySet(jalan); 
  pompa1->addFuzzySet(jalan1);
  
  fuzzy->addFuzzyOutput(pompa1);

  //Fuzzy output pompa1 (Waktu Pengairan)
  FuzzyOutput* pompa2 = new FuzzyOutput(2);

  pompa2->addFuzzySet(mati2); 
  pompa2->addFuzzySet(jalan2); 
  pompa2->addFuzzySet(jalann2);
  
  fuzzy->addFuzzyOutput(pompa2);

  //--------------------------Fuzzi RULE---------------------------------------//
  // Membuat FuzzyRule "IF (kondisi) = input  THEN (kondisi) = output"
  // Inisialisasi objek FuzzyRuleAntecedent 

  //--------------------------Fuzzi Rule 1 ---------------------------------------//

  FuzzyRuleAntecedent *ifkelembabanbasah = new FuzzyRuleAntecedent();
  ifkelembabanbasah->joinSingle(basah);
  
  FuzzyRuleConsequent *thenpompa1mati = new FuzzyRuleConsequent();
  thenpompa1mati->addOutput(mati);
  
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, ifkelembabanbasah, thenpompa1mati);
  fuzzy->addFuzzyRule(fuzzyRule01);
  //--------------------------Fuzzi Rule 2 ---------------------------------------//

  FuzzyRuleAntecedent *ifkelembabanlembab = new FuzzyRuleAntecedent();
  ifkelembabanlembab->joinSingle(lembab);
  
  FuzzyRuleConsequent *thenpompa1jalan = new FuzzyRuleConsequent();
  thenpompa1jalan->addOutput(jalan);
  
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, ifkelembabanlembab, thenpompa1jalan);
  fuzzy->addFuzzyRule(fuzzyRule02);
  //--------------------------Fuzzi Rule 3 ---------------------------------------//
  
  FuzzyRuleAntecedent *ifkelembabankering = new FuzzyRuleAntecedent();
  ifkelembabankering->joinSingle(kering);
 
  FuzzyRuleConsequent *thenpompa1jalan1 = new FuzzyRuleConsequent();
  thenpompa1jalan1->addOutput(jalan1);
  
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifkelembabankering, thenpompa1jalan1);
  fuzzy->addFuzzyRule(fuzzyRule03);
  //--------------------------Fuzzi Rule 4 ---------------------------------------//

  FuzzyRuleAntecedent *ifphasam = new FuzzyRuleAntecedent();
  ifphasam->joinSingle(asam);
  
  FuzzyRuleConsequent *thenpompa2jalan2 = new FuzzyRuleConsequent();
  thenpompa2jalan2->addOutput(jalan2);
  
  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, ifphasam, thenpompa2jalan2);
  fuzzy->addFuzzyRule(fuzzyRule04);
  //--------------------------Fuzzi Rule 5 ---------------------------------------//

  FuzzyRuleAntecedent *ifphnetral = new FuzzyRuleAntecedent();
  ifphnetral->joinSingle(netral);
  
  FuzzyRuleConsequent *thenpompa2mati2 = new FuzzyRuleConsequent();
  thenpompa2mati2->addOutput(mati2);
  
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, ifphnetral, thenpompa2mati2);
  fuzzy->addFuzzyRule(fuzzyRule05);
  //--------------------------Fuzzi Rule 6 ---------------------------------------//

  FuzzyRuleAntecedent *ifphbasa = new FuzzyRuleAntecedent();
  ifphbasa->joinSingle(basa);
  
  FuzzyRuleConsequent *thenpompa2jalann2 = new FuzzyRuleConsequent();
  thenpompa2jalann2->addOutput(jalann2);
  
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, ifphbasa, thenpompa2jalann2);
  fuzzy->addFuzzyRule(fuzzyRule06);
   
}

void loop() {
  
  StaticJsonBuffer<5000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    Serial.println("Invalid Json Object");
    jsonBuffer.clear();
   return;
  }
//  Serial.println("JSON Object Recieved");
//  Serial.print("Kelembaban:  ");
  int kelembaban = data["analog1"];
  float ph2 = data["analog3"];

  fuzzy->setInput(1, kelembaban);
  fuzzy->setInput(2, ph2);
  fuzzy->fuzzify();

//  int pompa_out1 = fuzzy->defuzzify(1);
//  digitalWrite(pompa1, pompa_out1);
//  int pompa_out2 = fuzzy->defuzzify(2);
//  digitalWrite(pompa2, pompa_out2);

  int pompa_out1 = fuzzy->defuzzify(1);
    if(pompa_out1 > 7){
      digitalWrite(pompa1, LOW);
       }
    else if(pompa_out1 >=5 && pompa_out1 <=7){
      digitalWrite(pompa1, LOW);
      }
    else if(pompa_out1 < 5){
      digitalWrite(pompa1, HIGH);
      }
  int pompa_out2 = fuzzy->defuzzify(2);
    if(pompa_out2 < 5){
      digitalWrite(pompa2, HIGH);
       }
    else if(pompa_out2 >=5 && pompa_out2 <=7){
      digitalWrite(pompa2, LOW);
      }
    else if(pompa_out2 >= 8){
      digitalWrite(pompa2, LOW);
      }
  
//  if(kelembaban >= 435){
//     kondisi = ("Tanah Kering");
//    }
//    else if(kelembaban >= 235 && kelembaban <=435){
//     kondisi = ("Tanah Lembab");
//    }
//    else if(kelembaban <= 235){
//     kondisi = ("Tanah Basah");
//    }
//  Serial.println(kelembaban);
//  Serial.print("PH:  ");
//  float ph = data["analog2"];
//  Serial.println(ph);
//  Serial.print("PH Baru: ");
//  float ph2 = data["analog3"];
//  Serial.println(ph2);
//  if(ph2 > 7 ){
//     kondisi2 = ("PH Basa");
//    }
//    else if(ph2 >=6 && ph2 <=7){
//     kondisi2 = ("PH Normal");
//    }
//    else if(ph2 < 6){
//     kondisi2 = ("PH Asam");
//  }

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
    Serial.print("Pompa1     :  ");
    Serial.println(pompa_out1);
    Serial.print("Pompa2     :  ");
    Serial.println(pompa_out2); 
}
             
  delay(5000);  
}


  
