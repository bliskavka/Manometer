//ver 2, threads included
//4th pin - button, 6s pin - bt
#include <Thread.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1015.h>

#define REFRESH_TIME 1000
#define FILTER_ACCURACY 32 // 1 - to disable
#define PRECISION_COEF 0.54
#define DEBUG_ENABLED false
#define DISPLAY_REFRESH true
#define ROUNDING_ENABLED true
#define SHORT_CLICK_DURATION 100
bool backLightEnabled = false;

LiquidCrystal_I2C lcd(0x3f,8,2);  // 0x27 for large, 0x3f for small disp
Adafruit_ADS1115 ads(0x48);

Thread adcThread = Thread();
int btnPin = 4;
float difference = 0.0;
float dpValue = 0.0;

void setup()
{
  pinMode(btnPin, INPUT);  //button pin
  Serial.begin(9600);
  ads.begin(); 
  lcd.init();

  adcThread.onRun(getAdcFiltered);   //creating new thread
  adcThread.setInterval(REFRESH_TIME);  //loop interval

  if (backLightEnabled)
     lcd.backlight();
  else
     lcd.noBacklight();  
}

void loop()
{
  switch(getButtonState(btnPin)){
    case 1:                           //short click
        if (backLightEnabled){
            lcd.noBacklight(); 
            backLightEnabled = false;}
        else{
            lcd.backlight();
            backLightEnabled = true;}
        break;
    case 2:                          //long click
        difference = dpValue;
        break; 
    }

  if (DEBUG_ENABLED){
  Serial.print("***********");
  Serial.print("\n");
  }
  if (Serial.available()) 
     btInput(Serial.read());

  if (adcThread.shouldRun())adcThread.run(); // starting the thread
  delay(50);
}


void displayOutput(float volt0, float volt1, float vcc){ 
  
  if (DISPLAY_REFRESH) lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Dinamic:");

  float dmPr = (volt0 / vcc - 0.5) / 0.057 * 101.97 - difference;  
  if (ROUNDING_ENABLED)
  dmPr = int(dmPr * 100) / 100.0 ;
  dpValue = dmPr;
  
  lcd.setCursor(1,1);
  lcd.print(dmPr);
  lcd.setCursor(7,1);
  lcd.print(" mmH2O");

  lcd.setCursor(1,2);
  lcd.print("Static:");

  float stPr = ((volt1 / vcc + 0.095) / 0.009) + PRECISION_COEF;
  if (ROUNDING_ENABLED)
  stPr = int(stPr * 100) / 100.0;
  
  lcd.setCursor(1,3);
  lcd.print(stPr);
  lcd.setCursor(7,3);
  lcd.print(" kPa");
}

 void getAdcFiltered(){
  unsigned long adc0 = 0, adc1 = 0;  
  float volt0, volt1, vcc;
  
    for (unsigned int i = 0; i < FILTER_ACCURACY; i++) 
        { 
          adc0 += ads.readADC_SingleEnded(0); 
          adc1 += ads.readADC_SingleEnded(1); 
        }         
    volt0 = (adc0 / FILTER_ACCURACY * 0.1865) / 1000;
    volt1 = (adc1 / FILTER_ACCURACY * 0.1865) / 1000;    
    vcc = (ads.readADC_SingleEnded(3)* 0.1875) / 1000;
    
    if (DEBUG_ENABLED == 1){
      Serial.print(volt1);
      Serial.print("__");
      Serial.print(adc1 / FILTER_ACCURACY);      
      Serial.print("\n");
      }
     displayOutput (volt0, volt1, vcc); 
  }
  
int getButtonState(int pin){
  int duration = pulseIn(pin, HIGH);  
  Serial.print(duration);
  Serial.print("\n");
  if ((duration < 100)&&(duration != 0)) return 1;
  if (duration > 2000) return 2;
  return 0;
  }

void btInput(int val){ 
   if (val == '1')
    {
      lcd.backlight();
    }
    if ( val == '0')
    {
      lcd.noBacklight();
    }  
}
