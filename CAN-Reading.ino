#include <SPI.h>
#include <SoftwareSerial.h>
#include <mcp2515.h>
#include <Nextion.h>
#include <SD.h>
MCP2515 mcp2515(9);
SoftwareSerial nextion(2,3);
Nextion dp(nextion,9600);

struct can_frame tmp;
struct can_frame rpm;
struct can_frame spd;
struct can_frame o2v;
struct can_frame maf;

volatile struct can_frame datatmp;
volatile struct can_frame datarpm;
volatile struct can_frame dataspd;
volatile struct can_frame datao2v;
volatile struct can_frame datamaf;
volatile int count;
volatile int value;
volatile float value2;
volatile float o;
volatile float m;

void setup() {
    tmp.can_id = rpm.can_id = spd.can_id = o2v.can_id = maf.can_id = 0x7df;
    tmp.can_dlc = rpm.can_dlc = spd.can_dlc = o2v.can_dlc = maf.can_dlc = 8;
    tmp.data[0] = rpm.data[0] = spd.data[0] = o2v.data[0] = maf.data[0] = 0x02;
    tmp.data[1] = rpm.data[1] = spd.data[1] = o2v.data[1] = maf.data[1] = 0x01;
    tmp.data[3] = rpm.data[3] = spd.data[3] = o2v.data[3] = maf.data[3] = 0x00;
    tmp.data[4] = rpm.data[4] = spd.data[4] = o2v.data[4] = maf.data[4] = 0x00;
    tmp.data[5] = rpm.data[5] = spd.data[5] = o2v.data[5] = maf.data[5] = 0x00;
    tmp.data[6] = rpm.data[6] = spd.data[6] = o2v.data[6] = maf.data[6] = 0x00;
    tmp.data[7] = rpm.data[7] = spd.data[7] = o2v.data[7] = maf.data[7] = 0x00;
    tmp.data[2] = 0x05; //tmp
    rpm.data[2] = 0x0c; //rpm
    spd.data[2] = 0x0d; //spd
    o2v.data[2] = 0x14; //o2v
    maf.data[2] = 0x10; //maf
    
    Serial.begin(115200);
    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS);
    mcp2515.setNormalMode();
    Serial.println("CONNECTED");
    dp.init();
}

void loop() {
    int gspd,pspd,grpm;
    if(count == 0){
      mcp2515.sendMessage(&spd);
      delay(2);
    }if(count == 1){
      value = calspd();
      printspd(value,gspd,pspd);
    }
    if(count == 2){
      mcp2515.sendMessage(&tmp);
      delay(2);
    }if(count == 3){
      value = caltmp();
      printtmp(value);
    }
    if(count == 4){
      mcp2515.sendMessage(&o2v);
      delay(2);
    }if(count == 5){
      mcp2515.readMessage(&datao2v);
      float resulto,c;
      c = datao2v.data[3];
      resulto = c/200;
      printox(resulto);
      delay(2);
    }if(count == 6){
      mcp2515.sendMessage(&maf);
      delay(2);
    }if(count == 7){
      mcp2515.readMessage(&datamaf);
      float result,d,e,cal;
      d = datamaf.data[3];
      e = datamaf.data[4];
      cal = (256*d)+4;
      result = cal/100;
      printmaf(result);
      delay(2);
    }
    if(count == 8){
      mcp2515.sendMessage(&rpm);
      delay(2);
    }if(count == 9){
      value2 = calrpm();
      printrpm(value2,grpm);
    }if(count == 10){
      count = -1;
    }
    count++;
}

int caltmp(){
  mcp2515.readMessage(&datatmp);
  int ttmp;
  if(mcp2515.readMessage(&datatmp) == MCP2515::ERROR_OK){
      if(datatmp.data[2] == 0x05){
        /*Serial.print("TEMP RECIVE = ");
        Serial.println(data.data[3]);*/
        ttmp = datatmp.data[3]-40;
        return ttmp;
      }
    }else{
        Serial.println(" TEMP CAL ERROR");
      }
}

int calspd(){
  mcp2515.readMessage(&dataspd);
  int tspd;
  if(mcp2515.readMessage(&dataspd) == MCP2515::ERROR_OK){
      if((dataspd.data[2])== 0x0d){
        /*Serial.print("SPEED RECIVE = ");
        Serial.println(data.data[3]);*/
        tspd = dataspd.data[3];
        return tspd;
      }
    }else{
        Serial.println(" SPEED CAL ERROR");
      }
}
float calrpm(){
    mcp2515.readMessage(&datarpm);
    float a,b,trpm;
    if(mcp2515.readMessage(&datarpm) == MCP2515::ERROR_OK){
      if((datarpm.data[2])== 0x0c){
        /*Serial.print("RPM RECIVE = ");
        Serial.print(data.data[3]);
        Serial.print(" ");
        Serial.println(data.data[3]);*/
        a = (datarpm.data[3]);
        b = (datarpm.data[4]);
        trpm = (((256*a)+b)/4);
        return trpm;
      }
    }else{
        Serial.println(" RPM CAL ERROR");
      } 
}
void printrpm(float rpm, int r){
    Serial.print(" ");
    Serial.print("RPM = ");
    Serial.print(rpm);
    dp.setComponentText("t72", String(rpm));
    if((rpm>=2750)&&(rpm<=17500)){
      r = map(rpm, 2750, 17500, 0, 213);dp.setComponentValue("z31", r);
    }if(rpm<2750){
      r = map(rpm, 0, 2750, 327, 360);dp.setComponentValue("z31", r);
    }
}
void printtmp(int tmp){
    Serial.print(" ");
    Serial.print("TMP = ");
    Serial.print(tmp);
    dp.setComponentText("t71", String(tmp));
}
void printspd(int spd,int g,int p){
    Serial.print(" ");
    Serial.print("SPD = ");
    Serial.print(spd);
    dp.setComponentText("t73", String(spd));
    p = map(spd, 0, 260, 0, 100);
    dp.setComponentValue("j42", p);
    if(spd>=42.5){
      g = map(spd, 42.5, 260, 0, 225);dp.setComponentValue("z41", g);
    }if(spd<42.5){
      g = map(spd, 0, 42.5, 316, 360);dp.setComponentValue("z41", g);
    }
}
void printox(float ox){
    Serial.print(" ");
    Serial.print("O2V = ");
    Serial.print(ox);
    dp.setComponentText("t74", String(ox));
}
void printmaf(float maf){
    Serial.print(" ");
    Serial.print("MAF = ");
    Serial.print(maf);
    Serial.println();
    dp.setComponentText("t75", String(maf));
}
