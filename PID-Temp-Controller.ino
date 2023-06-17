/*    Max6675 Module  ==>   Arduino
      CS              ==>     D10
      SO              ==>     D9
      SCK             ==>     D13
      Vcc             ==>     Vcc (5v)
      Gnd             ==>     Gnd      */

//LCD config
#include "max6675.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); 

/*    i2c LCD Module  ==>   Arduino
      SCL             ==>     A5
      SDA             ==>     A4
      Vcc             ==>     Vcc (5v)
      Gnd             ==>     Gnd      */

//Inputs and outputs อุณหภูมิ
int firing_pin = 3;
int increase_pin = 11;
int decrease_pin = 12;
int zero_cross = 8;
int thermoDO = 9;
int thermoCS = 10;
int thermoCLK = 13;

// mode//
int s_mode1 = 4;
String Namemode;
int s_timerup = 5;
int s_timerlo = 6;
int s_rst = 7;
int timeset = 0;
unsigned long timer = millis();
int s = 0, m = 0;
int Timeset = 0;
int count = 0;
int selectmode = 0;
int last_CH1_state = 0;
bool zero_cross_detected = false;
int firing_delay = 7400;
int maximum_firing_delay = 7400;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int temp_read_Delay = 500;
int real_temperature = 0;
int setpoint = 100;
bool pressed_1 = false;
bool pressed_2 = false;
//PID variables
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
int PID_value = 0;
//PID constants
int kp = 400;   int ki = 2;   int kd = 1;
int PID_p = 0;    int PID_i = 0;    int PID_d = 0;

void setup() {
  //Define the pins
  pinMode (firing_pin, OUTPUT);
  pinMode (zero_cross, INPUT);
  pinMode (increase_pin, INPUT);
  pinMode (decrease_pin, INPUT);
  pinMode (s_mode1, INPUT);
  pinMode (s_timerup, INPUT);
  pinMode (s_timerlo, INPUT);
  pinMode (s_rst, INPUT);
  PCICR |= (1 << PCIE0);    
  PCMSK0 |= (1 << PCINT0);  
  PCMSK0 |= (1 << PCINT3);  
  PCMSK0 |= (1 << PCINT4);  
  lcd.init();       
  lcd.backlight();  
  Serial.begin(9600);
}
void loop()
{
  if (digitalRead (s_mode1) == 1)
  {
    delay(200);
    selectmode = selectmode = !selectmode;
  }
  if (digitalRead(s_timerup) == 1)
  {
    delay(200);
    Timeset  += 5;
  }
  if (digitalRead(s_timerlo) == 1)
  {
    delay(200);
    Timeset -= 5;
    if (Timeset <= 0)
    {
      Timeset = 0;
    }
  }
  currentMillis = millis();           
"temp_read_Delay"
 if(currentMillis - previousMillis >= temp_read_Delay);{
    previousMillis += temp_read_Delay
    real_temperature = thermocouple.readCelsius();  
 
    PID_error = setpoint - real_temperature;        
    if (PID_error > 30)                             
    {
      PID_i = 0;
    }
    PID_p = kp * PID_error;                         
    PID_i = PID_i + (ki * PID_error);               
    timePrev = Time;             
    Time = millis();                    
    elapsedTime = (Time - timePrev) / 1000;
    PID_d = kd * ((PID_error - previous_error) / elapsedTime); 
    PID_value = PID_p + PID_i + PID_d;                      

    if (PID_value < 0)
    {
      PID_value = 0;
    }
    if (PID_value > 7400)
    {
      PID_value = 7400;
    }
    //Printe the values on the LCD
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Real");
    lcd.setCursor(14, 0);
    lcd.print("Set");
    lcd.setCursor(10, 0);
    lcd.print(" ");
    lcd.setCursor(10, 1);
    lcd.print(" ");
    lcd.setCursor(10, 2);
    lcd.print(" ");
    lcd.setCursor(10, 3);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print("Temp ");
    lcd.setCursor(5, 1);
    lcd.print(real_temperature);
    lcd.setCursor(11, 1);
    lcd.print("Temp   ");
    lcd.setCursor(16, 1);
    lcd.print(setpoint);
    lcd.setCursor(0, 2);
    lcd.print("Time ");
    lcd.setCursor(5, 2);
    lcd.print(m);
    lcd.print(":");
    lcd.print(s);
    lcd.setCursor(11, 2);
    lcd.print("Time ");
    lcd.setCursor(16, 2);
    lcd.print(Timeset);
    lcd.setCursor(3, 3);
    lcd.print("MODE");
    lcd.setCursor(14, 3);
    lcd.print(Namemode);

    Serial.print("Set: ");
    Serial.print(setpoint);
    Serial.print("   Real temp: ");
    Serial.print(real_temperature);
    Serial.print("   ");
    Serial.print(Namemode);
    Serial.print("  TimeRun  ");
    Serial.print(m);
    Serial.print(":");
    Serial.print(s);
    Serial.print("   Timeset  ");
    Serial.print(Timeset);
    Serial.print("   .....  ");
    Serial.println(s);
    previous_error = PID_error; 
  }
  if (selectmode == 1)
  {
    if (digitalRead (s_rst) == 1)
    {
      delay (200);
      timeset = 1;
    }
    if (Timeset == m)
    {
      selectmode = 0;
    }
    Namemode = "ON";

    if (zero_cross_detected)
    {
      delayMicroseconds(maximum_firing_delay - PID_value); 
      digitalWrite(firing_pin, HIGH);
      delayMicroseconds(100);
      digitalWrite(firing_pin, LOW);
      zero_cross_detected = false;
    }
    timerun();
  }

  else
  {
    Namemode = "OFF";
    digitalWrite(firing_pin, LOW);
    s = 0;
    m = 0;
    timeset = 0;
    Timeset = 5;
  }
}
void timerun()
{
  if (timeset == 1)
  {
    count++;
    if (millis() - timer >= 1000)
    { 
    count++;
    timer += 1000;
    if(count>=1000)
    {
      s++;
      count=0;
    }
      
    }

    if (s > 59)
    {
      s = 0;
      m++;
      if (m >= 60)
      {
        m = 0;
      }
    }
  }
}
ISR(PCINT0_vect) {
  
  if (PINB & B00000001) {   pin D8 is HIGH
    if (last_CH1_state = 0) {     
      zero_cross_detected = true; 
    }
  }
  else if (last_CH1_state == 1) {  
    zero_cross_detected = true;    
    last_CH1_state = 0;           
  }
  if (PINB & B00001000) {       
    if (!pressed_1)
    {
      setpoint = setpoint + 5;   
      delay(20);
      pressed_1 = true;
    }
  }
  else if (pressed_1)
  {
    pressed_1 = false;
  }
  if (PINB & B00010000) {     
    if (!pressed_2)
    {
      setpoint = setpoint - 5;  
      delay(20);
      pressed_2 = true;
    }
  }
  else if (pressed_2)
  {
    pressed_2 = false;
  }
}
