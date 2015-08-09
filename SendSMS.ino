#include "TimerOne.h"
#include <GSM.h>
#include <Wire.h> // I2C library include
#include <Deuligne.h> // LCD library include
#define PINNUMBER ""

static Deuligne lcd; // lcd object declaration
char message[16];
boolean gsmStatus = false;
int actualPrint = 1;
int nextPrint = 1;

// begin wdt stuff
void (*resetFunc)(void) = 0;
 
//last time the WDT was ACKd by the application
unsigned long lastUpdate=0;
 
//time, in ms, after which a reset should be triggered
unsigned long timeout=12*1000;
 
void longWDT(void)
{
  if((millis()-lastUpdate)>timeout)
  {
    //enable interrupts so serial can work
    sei();
 
    //detach Timer1 interrupt so that if processing goes long, WDT isn't re-triggered
    Timer1.detachInterrupt();
 
    //flush, as Serial is buffered; and on hitting reset that buffer is cleared
    printLCD(" !! WDT !!");
    delay(500);
    Serial.flush();
    //call to bootloader / code at address 0
    resetFunc();
  }
}
// end wdt stuff


// initialize the library instance
GSM gsmAccess(false); // include a 'true' parameter for debug enabled
GSM_SMS sms;

// char array of the telephone number to send SMS
// change the number 1-212-555-1212 to a number
// you have access to
char remoteNumber[20]= "0688649102";  

// char array of the message
char txtMsg[200]="Test envoi SMS long";

void setup()
{

// start lcd stuff
  Wire.begin(); // join i2c
  lcd.init(); // LCD init
  lcd.clear(); // Clear Display
  lcd.backLight(true); // Backlight ON
// end lcd stuff


// start wdt stuff
  timeout = 20*1000; 
  lastUpdate=millis();
  Timer1.initialize(1000000); //1 second pulses
  Timer1.attachInterrupt(longWDT); //code to execute
// end wdt stuff

  // initialize serial communications
  Serial.begin(9600);
  printChoix1();
  actualPrint = 1;
  nextPrint = 1;
  start();
}

void printLCD(char* text){
  lcd.clear(); // Clear Display
  lcd.setCursor(0,0); // Place cursor row 6, 1st line (counting from 0)
  lcd.print(text);
  Serial.println(text);
}
void printLCD(int pos, int line, char* text, boolean clearScreen=false){
  if(clearScreen == true) {lcd.clear(); }// Clear Display
  lcd.setCursor(pos,line); // Place cursor row 6, 1st line (counting from 0)
  lcd.print(text);
  Serial.println(text);
}
void printLCD(char* text, char* text2){
  lcd.clear(); // Clear Display
  lcd.setCursor(0, 0); // Place cursor row 6, 1st line (counting from 0)
  lcd.print(text);
  lcd.setCursor(0, 1); // Place cursor row 6, 1st line (counting from 0)
  lcd.print(text2);
}
void printChoix1(){
  printLCD("menu : ", "1 - SendSMS");  
  }
void setChoix1(){
  printLCD("ok"); ;
  delay(300);
  sendSMS(); 
  }
void printChoix2(){
  printLCD("menu : ", "2 - Power off");  
  }
void setChoix2(){
  printLCD("ok"); ;
  delay(300);
  stopgsm(); 
  }
void printChoix3(){
  printLCD("menu : ", "3 - Power on");  
  }
void setChoix3(){
  printLCD("ok"); ;
  delay(300);
  start(); 
}
void printChoix4(){
  printLCD("menu : ", "4 - Status");  
  }
void setChoix4(){
  printLCD("ok"); ;
  delay(300);
  start(); 
}

void resetChoix(){
  printChoix1();
  actualPrint = 1;
  nextPrint = 1;
}

void processChoix(){
  if(actualPrint==1 ) {setChoix1();}
  if(actualPrint==2 ) {setChoix2();}
  if(actualPrint==3 ) {setChoix3();}
  if(actualPrint==4 ) {setChoix4();}
}

void toPrint(){
  if(nextPrint==1) {printChoix1();}
  if(nextPrint==2) {printChoix2();}
  if(nextPrint==3) {printChoix3();}
  if(nextPrint==4) {printChoix4();}
  actualPrint = nextPrint;
}
void loop()
{
   // delay(1000000);

    lastUpdate=millis(); //wdt reset
    int8_t key = -1;

    delay(100);    // wait for debounce time
    key = lcd.get_key(); // read the value from the sensor & convert into key press
    if (key!=-1){
      if (key==2){nextPrint = nextPrint - 1;}
      if (key==1){nextPrint = nextPrint + 1;}
      if (key==4){processChoix();}
  
      if(nextPrint>4) {nextPrint=1;}
      if(nextPrint<1) {nextPrint=4;}
      toPrint();
    }

  if (gsmStatus == true){
    if (sms.available()){ readSMS(); }    
  }
}

void start(){
  printLCD("GSM init");
  // connection state
  boolean notConnected = true;

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY){
      notConnected = false;
      gsmStatus = true;
    }
    else
    {
      printLCD("Not connected");
      delay(1000);
    }
  }
  printLCD("GSM init OK");
  delay(500);  
  resetChoix();
}

void stopgsm(){
  if (gsmStatus == false){
    printLCD("already off");  
    delay(500);   
  }
  else{
    delay(500);
    printLCD("shuting down");  
    gsmAccess.shutdown();
    gsmStatus = false;
    printLCD("GSM off");  
    delay(500);  
    resetChoix();
  }
}

void sendSMS(){

  printLCD("Message to ");
  printLCD(0, 1, remoteNumber);
  delay(500);
  // sms text
  printLCD(txtMsg);

  // send the message
  sms.beginSMS(remoteNumber);
  sms.print(txtMsg);
  sms.endSMS(); 
  printLCD("envoi ok");
  delay(500);  
  resetChoix();
}
void readSMS(){
  printLCD("New message !");
  delay(400);
  sms.remoteNumber(remoteNumber, 20);
  printLCD(remoteNumber);
  for (byte i = 0; i < 16; i++){
    message[i] = sms.read();
  }
  printLCD(0, 1, message);

  //while(c=sms.read())
  //Serial.print(c);
  sms.flush();
  delay(3000);
  printLCD("message deleted !");
  delay(500);
  resetChoix();
}
