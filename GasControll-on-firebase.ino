#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
const char* ssid = "Folk";
const char* password = "12345678";
#define FIREBASE_HOST "gasstation-dd9d4-default-rtdb.firebaseio.com/"  //Change to your Firebase RTDB project ID e.g. Your_Project_ID.firebaseio.com
#define FIREBASE_AUTH "hI0VgHRGnWCMIATX8rn6EvAjMZvj4ZgRmOCcQZqI" //Change to your Firebase RTDB secret password
FirebaseData gasValue;
FirebaseData gasSet;
FirebaseData Mode;
String path = "/Gas/Center";
String ID1 = "gasSet";      //This is this node ID to receive control
String ID2 = "gasValue"; //This is other node ID to control
String ID3 = "Mode";
volatile unsigned long prev;
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }else{
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if(!Firebase.beginStream(gasSet, path + "/" + ID1)){
    Serial.println("GAS SET NOT STREAM REASON: "+ gasSet.errorReason());
  }
  if(!Firebase.beginStream(gasValue, path + "/" + ID2)){
    Serial.println("GAS VALUE NOT STREAM REASON: "+ gasValue.errorReason());
  }
  if(!Firebase.beginStream(Mode, path + "/" + ID1)){
    Serial.println("MODE NOT STREAM REASON: "+ Mode.errorReason());
  }
  Serial.println("FINISH SETUP");
}

void loop() {
  if(millis()-prev > 1000){
    setting1();
    setting2();
    prev = millis();
  }
}

void setting1(){
  if(Firebase.getInt(gasSet, path + "/" + ID1) && Firebase.getInt(gasValue, path + "/" + ID2) && Firebase.getInt(Mode, path + "/" + ID3)){
    Serial.printf("GAS SET | GAS VALUE | MODE\n");
    Serial.printf("  %d    |    %d    |   %d \n",gasSet.intData(),gasValue.intData(),Mode.intData());
  }
}

void setting2(){
  /*if(Firebase.getInt(Mode, path + "/" + ID3)){
    Serial.printf("MODE : %d \n",Mode.intData());
  }*/
}