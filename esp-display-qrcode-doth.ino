#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "QR.h"
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx#include "QRCODE.h"
#define FIREBASE_HOST "esp8266c-93720-default-rtdb.asia-southeast1.firebasedatabase.app"  //Change to your Firebase RTDB project ID e.g. Your_Project_ID.firebaseio.com
#define FIREBASE_AUTH "k2bg4gdCe66g1A2JmGNcarHWrlboPCtOkF8K7TtF" //Change to your Firebase RTDB secret password
#define WIFI_SSID "Donee"  //ใส่ชื่อ wifi
#define WIFI_PASSWORD "12345678" //ใส่ password wifi
const long offsetTime = 25200;                                                   // หน่วยเป็นวินาที จะได้ 7*60*60 = 25200
WiFiUDP ntpUDP;                                                                  // กำหนด object ของ WiFiUDP ชื่อว่า ntpUDP
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);                        // กำหนด object ของ NTPClient ชื่อว่า timeClient มีรูปแบบ ("WiFiUDP Object","NTP Server Address","offset time")
int hourNow, minuteNow, secondNow, starttime;     
FirebaseData firebaseData1;
FirebaseData firebaseData2;
//********* Parameter PART INCLUDE ************//
char Array[] = "";
char *strings[20];                                                               // an array of pointers to the pieces of the above array after strtok()
char *ptr = NULL;
String Voltage = "";
String Current = "";
String Power = "";
String Energy = "";
String Frequency = "";
String PF = "";
volatile int Times_USE = 0;
volatile float Units_USE = 0;
volatile float Total_Units = 0;
volatile float Last_Units = 0;
volatile int HH = 0;
volatile int MM = 0;
volatile int Ss = 0;
bool counter = false;
bool check;
unsigned long last_time = 0;
unsigned long time_pre = 250;
String path = "/parameter";
String ID1 = "State";      //This is this node ID to receive control
String ID2 = "Counter"; //This is other node ID to control
TFT_eSPI tft = TFT_eSPI();
void setup(){
  tft.begin();
  tft.setRotation(3);                                                 // 0, 2 = portrait 1, 3 Landscape
  //tft.fillScreen(0xFFFF);
    /************* WIFI SETUP ***************/
    Serial.begin(9600);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    /************* WIFI SETUP ***************/

    /************* FIREBASE SETUP ***************/
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

    if (!Firebase.beginStream(firebaseData1, path + "/" + ID1))
    {
        Serial.println("Could not begin stream");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println();
    }
    if (!Firebase.beginStream(firebaseData2, path + "/" + ID2))
    {
        Serial.println("Could not begin stream");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println();
    }
    /************* FIREBASE SETUP ***************/
}

void loop()
{
  /*if(Serial.available())
  {
    String Temp = Serial.readString();
    int str_len = Temp.length() + 1;
    char char_array[str_len];
    Temp.toCharArray(char_array, str_len);
    byte index = 0;
    ptr = strtok(char_array, ":");  // delimiter
    while (ptr != NULL)
    {
       strings[index] = ptr;
       index++;
       ptr = strtok(NULL, ":");
    }
    Voltage = (strings[0]); 
    Current = (strings[1]);
    Power = (strings[2]);
    Energy = (strings[3]); 
    Frequency = (strings[4]);
    PF = (strings[5]);
  }*/
  HH = Times_USE/3600;                                                  // แปลงเวลาเป็นวินาที เพิ่อลดลงทุกๆ 1 วินาทีตามโจทย์
  MM = (Times_USE%3600)/60;
  Ss = (Times_USE%3600)%60;
  String HH_S = String(HH);
  String MM_S = String(MM);
  String SS_S = String(Ss);
  String Unit_S = String(Units_USE);
  tft.drawXBitmap(0, 0, logo, logoWidth, logoHeight, TFT_WHITE);
  tftdisplay(HH_S,MM_S,SS_S, Unit_S);
    if(millis() - last_time > time_pre){                                    //*This func run every 1 sec
        if(State()){                                                      //*Check State from Firebase
          if(Firebase.getInt(firebaseData2, path + "/" + ID2)){
              int firebaseCounter = firebaseData2.intData();
              if(firebaseCounter > 0){                                    //*Check Counter >= 0.
                tft.setTextSize(11);
                tft.drawString("          ", 15, 230);
                tft.drawString("Charging", 35, 230);
                Serial.print("ON");                                       //*Command to relay on
                setCounterfirebase(firebaseCounter);                      //*Counting time and set new recoder.
              }else{
                tft.setTextSize(11);
                tft.drawString("Uncharging", 15, 230);
                Serial.println("OFF");
                Firebase.setInt(firebaseData2, path + "/" + ID2, 0);      //*Set Counter to 0
              }         
          }else{
            tft.setTextSize(11);
            tft.drawString("Uncharging", 15, 230);
            //Firebase.setBool(firebaseData1, path + "/" + ID1, false);     //*And let's set State too false (Unchage);
            Serial.println("OFF");
          }
    }
    last_time = millis();
  }
  Serial.print("\n");
}

int setCounterfirebase(int x){
    if(counter == true){
      if(x>0){
        if(starttime != timeClient.getMinutes()){
          starttime = timeClient.getMinutes();
          Times_USE = Times_USE + 1;
          Units_USE = Units_USE + 0.0167;     
          Total_Units = Total_Units + 0.0167;
          x -= 1;
          Firebase.setInt(firebaseData2, path + "/" + ID2, x);
        }
      }else{
        Last_Units = Units_USE;
        x = 0;
        counter = false;        
      }
    }else{                                                      //*Start Counter
      starttime = timeClient.getMinutes();                      //*Request Time
      Units_USE = 0;                                            //*Reset Units_USE
      counter = true;                                           //*Change Counter State
    }
    return x;    
}

void timeReq(){
  timeClient.update();                                          // ร้องขอ timestamps ด้วยคำสั่ง update
  secondNow = timeClient.getSeconds();                        // get ค่าวินาที นาที ชั่วโมง ไปเก็บไว้ในตัวแปร
  minuteNow = timeClient.getMinutes();
  hourNow = timeClient.getHours();
}

void tftdisplay(String HH_S, String MM_S, String SS_S, String Unit_S ){
  //Serial.println("tft display function start");
  //tft.fillRect(220, 50, 250, 250, 0xFFFF);
  tft.drawXBitmap(0, 0, logo, logoWidth, logoHeight, TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Times: ", 240, 10);
  tft.drawString("Units: ", 240, 110);
  tft.setTextSize(4);
  tft.drawString(HH_S, 240, 55);
  tft.drawString(":", 290, 55);
  tft.drawString(MM_S, 315, 55);
  tft.drawString(":", 365, 55);
  tft.drawString(SS_S, 390, 55);
  tft.setTextSize(5);
  tft.drawString(Unit_S, 250, 155);
}


bool State(){
  bool x;
  if(Firebase.getBool(firebaseData1, path + "/" + ID1)){
        x = firebaseData1.boolData();
  }
  return x;
}