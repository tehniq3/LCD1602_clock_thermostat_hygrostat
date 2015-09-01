// based on
// Dawn & Dusk controller. http://andydoz.blogspot.ro/2014_08_01_archive.html
// 16th August 2014.
// (C) A.G.Doswell 2014
// adapted sketch by niq_ro from http://nicuflorica.blogspot.ro & http://arduinotehniq.blogspot.com/
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
//
// Time is set using a rotary encoder with integral push button. The Encoder is connected to interrupt pins D2 & D3 (and GND), 
// and the push button to pin analogue 0 (and GND)
// The RTC is connections are: Analogue pin 4 to SDA. Connect analogue pin 5 to SCL.
// A 2 x 16 LCD display is connected as follows (NOTE. This is NOT conventional, as interrupt pins are required for the encoder)
// 
// Use: Pressing and holding the button will enter the clock set mode (on release of the button). Clock is set using the rotary encoder. 

#include <Wire.h>
#include "RTClib.h" // from https://github.com/adafruit/RTClib
#include <LiquidCrystal.h>
#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html

RTC_DS1307 RTC; // Tells the RTC library that we're using a DS1307 RTC
//Encoder knob(2, 3); //encoder connected to pins 2 and 3 (and ground)
Encoder knob(5, 6); //encoder connected to pins 5 and 6 (and ground)

// initialize the library with the numbers of the interface pins
 LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
/*                                    -------------------
                                      |  LCD  | Arduino |
                                      -------------------
 LCD RS pin to digital pin D7         |  RS   |   D7    |
 LCD Enable pin to digital pin D8     |  E    |   D8    |
 LCD D4 pin to digital pin D9         |  D4   |   D9    |
 LCD D5 pin to digital pin D10        |  D5   |   D10   |
 LCD D6 pin to digital pin D11        |  D6   |   D11   |
 LCD D7 pin to digital pin D12        |  D7   |   D12   |
 LCD R/W pin to ground                |  R/W  |   GND   |
                                      -------------------
*/

//the variables provide the holding values for the set clock routine
int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 2000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

float knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag
byte pauza = 15;  // time beetween work at encoder

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

#include <DHT.h>
#define DHTPIN A1     // what pin we're connected DHT11/22
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22

DHT dht(DHTPIN, DHTTYPE);
float t1;



byte heat;  // is 1 for heater and 0 for cooler
byte dry;   // is 1 for dryer and 0 for wetter 

// define pins for relay or leds
#define temppeste 4
#define umidpeste 3

// define variable

float te, tunu, tes, dete, dete1;   // variable for temperature
int has, hass, dehas;        // variable for humiditiy


void setup () {
    //Serial.begin(57600); //start debug serial interface
    Wire.begin(); //start I2C interface
    RTC.begin(); //start RTC interface
  dht.begin();  // DHT init 
    lcd.begin(16,2); //Start LCD (defined as 16 x 2 characters)
  lcd.createChar(0, grad);  // create custom symbol
    lcd.clear(); 
    pinMode(A0,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(A0,HIGH); //Pull A0 high
//    pinMode(A3,OUTPUT); //Relay connected to A3
//    digitalWrite (A3, HIGH); //sets relay off (default condition)
    
    //Checks to see if the RTC is runnning, and if not, sets the time to the time this sketch was compiled.
    if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
 
// define outputs
 pinMode(temppeste, OUTPUT);  
 pinMode(umidpeste, OUTPUT);

// set the default state for outputs
  digitalWrite(temppeste, LOW);
  digitalWrite(umidpeste, LOW);

// preset value for temperature and humidity
tes = 28.9;   
dete = 0.2;
hass = 58;
dehas = 2;
 

   
}
           

void loop () {

  int h = dht.readHumidity();
  float t = dht.readTemperature();
 //  lcd.setCursor(11, 1);
     lcd.setCursor(9, 1);
  // lcd.print("t=");
 if (t < 0) 
 {t1=-t;}
 else t1=t;
  
    if ( t1 < 10)
   {
     lcd.print(" "); 
   }
   if (t>0) lcd.print("+"); 
   if (t==0) lcd.print(" "); 
   if (t<0) lcd.print("-");
 //  lcd.print(t1,0);
     lcd.print(t1,1);
   //lcd.print(",0");
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C");
    

   lcd.setCursor(11, 0);
  // lcd.print("H=");
   lcd.print(h);
   lcd.print("%RH"); 
   
    DateTime now = RTC.now(); //get time from RTC
    //Display current time
    lcd.setCursor (0,0);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.print(" ");
    lcd.setCursor (0,1);
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.second());
//    lcd.print("     ");
    
    //current time in minutes since midnight (used to check against sunrise/sunset easily)
 //   TimeMins = (now.hour() * 60) + now.minute();
    
     pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
   
    if (pushlength <pushlengthset) {
     
      ShortPush ();   
    }
    
       
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength >pushlengthset) {
         lcd.clear();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };
}

//sets the clock
void setclock (){
   setyear ();
   lcd.clear ();
   setmonth ();
   lcd.clear ();
   setday ();
   lcd.clear ();
   sethours ();
   lcd.clear ();
   setmins ();
   lcd.clear();
   
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));
//   CalcSun ();
   delay (1000);
   
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(A0);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}
// The following subroutines set the individual clock parameters
int setyear () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Year");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2015) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2015;
    }
    lcd.print (setyeartemp);
    lcd.print("  "); 
    setyear();
}
  
int setmonth () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Month");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
    lcd.print (setmonthtemp);
    lcd.print("  "); 
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
//lcd.clear();  
   lcd.setCursor (0,0);
    lcd.print ("Set Day");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
    lcd.print (setdaytemp);
    lcd.print("  "); 
    setday();
}

int sethours () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 1;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
    lcd.print (sethourstemp);
    lcd.print("  "); 
    sethours();
}

int setmins () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
    lcd.print (setminstemp);
    lcd.print("  "); 
    setmins();
}



void ShortPush () {
  //This displays the calculated sunrise and sunset times when the knob is pushed for a short time.
lcd.clear();
for (long Counter = 0; Counter < 2000 ; Counter ++) { //returns to the main loop if it's been run 604 times 
                                                     //(don't ask me why I've set 604,it seemed like a good number)
//  lcd.setCursor (0,0);
//  lcd.print ("Sunrise ");

   lcd.setCursor(0, 0);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C ");
   lcd.setCursor(1, 1);
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C ");

   lcd.setCursor(7, 0);
   lcd.print(hass);
   lcd.print("%RH ");
   lcd.setCursor(8, 1);
   lcd.print(dehas);
   lcd.print("%RH ");

   lcd.setCursor(14, 0);
if (heat == 1) 
{
   lcd.print("HT");
}
if (heat == 0) 
{
   lcd.print("CL");
}

   lcd.setCursor(14, 1);
if (dry == 1) 
{
   lcd.print("DR");
}
if (dry == 0) 
{
   lcd.print("WE");
}


  if (te > tes) 
 {
if (heat == 1) digitalWrite(temppeste, LOW); 
if (heat == 0) digitalWrite(temppeste, HIGH); 
 } 
if (tes - dete > te)
  {
if (heat == 0) digitalWrite(temppeste, LOW); 
if (heat == 1) digitalWrite(temppeste, HIGH); 
 } 

 if (has > hass) 
 {
if (dry == 1) digitalWrite(umidpeste, HIGH); 
if (dry == 0) digitalWrite(umidpeste, LOW); 
 } 
 if (has < hass - dehas) 
 {
if (dry == 0) digitalWrite(umidpeste, HIGH); 
if (dry == 1) digitalWrite(umidpeste, LOW); 
 } 

    
  //If the knob is pushed again, enter the mode set menu
  pushlength = pushlengthset;
  pushlength = getpushlength ();
  if (pushlength != pushlengthset) {
    lcd.clear ();
//    heat = setmode ();
   tehas();
  }
  
}
lcd.clear();
}


void tehas (){
sette ();
   lcd.clear ();
setdete ();
   lcd.clear ();
sethas ();
   lcd.clear ();
setdehas ();
   lcd.clear ();
setheat ();
   lcd.clear();
setdry();
   lcd.clear();

}


// The following subroutines set thermostat and hygrostat parameters
// this subroutine set temperature point for thermostat
int sette () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set temperature");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return tes;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    tes = tes + knobval/10;
//    delay(10);
    if (tes < 10) { 
      tes = 10;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

    sette();
}


// this subroutine set hysteresis temperature for thermostat
int setdete () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set hyst.temp.:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dete;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dete = dete + knobval/10;
//    delay(10);
    if (dete < 0.1) { //Year can't be older than currently, it's not a time machine.
      dete = 0.1;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

    setdete();
}


// this subroutine set humidity point for hygrostat
int sethas () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set humidity:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return hass;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    hass = hass + knobval;
//    delay(10);
   if (has < 10) { 
      has = 10;
    }
   if (has > 90) { 
      has = 90;
    }
    
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass,1);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

    sethas();
}


// this subroutine set hysteresis humidity for hygrostat
int setdehas () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set hyst.humidit");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dehas;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dehas = dehas + knobval;
//    delay(10);
    if (dehas < 1) { //Year can't be older than currently, it's not a time machine.
      dehas = 1;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass,1);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

    setdehas();
}

// this subroutine set type of thermostat
int setheat () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("thermostat type:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return heat;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    heat = heat + knobval;
//    delay(10);
    if (heat < 0) { 
      heat = 0;
    }
    if (heat > 1) { 
      heat = 1;
    }
       
    lcd.setCursor(0, 1);
if (heat == 1) 
{
   lcd.print("HEATER ");
}
if (heat == 0) 
{
   lcd.print("COOLER ");
}
    setheat();
}

// this subroutine set type of hygrostat
int setdry () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("hygrostat type:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dry;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dry = dry + knobval;
//    delay(10);
    if (dry < 0) { 
      dry = 0;
    }
    if (dry > 1) { 
      dry = 1;
    }
       
    lcd.setCursor(0, 1);
if (dry == 1)     
{
   lcd.print("DRYER ");
}
if (dry == 0) 
{
   lcd.print("WETTER ");
}
    setdry();
}


