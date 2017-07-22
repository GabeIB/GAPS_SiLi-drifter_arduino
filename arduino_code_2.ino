#define triac_pin 5
#define delay_input 2
#include <digitalWriteFast.h>
int temp_index = 15;
int old_temp_index = 0;
int binary_counter = 0;

void setup() {
  pinMode(13,INPUT);
  pinMode(12,INPUT);
  pinMode(11,INPUT);
  pinMode(10,INPUT);
  pinMode(9,INPUT);
  pinMode(8,INPUT);
  
   pinMode(delay_input,INPUT);
   pinMode(triac_pin,OUTPUT);
   attachInterrupt(0,acon,RISING);

   Serial.begin(9600);
}

void loop() {
  temp_index = booleanRead();
  if(temp_index != old_temp_index){
    Serial.println(temp_index);
    old_temp_index = temp_index;
  }
  //Serial.print("test");
}

void acon(){
  //Serial.println("rising");
  if(temp_index>15){ //want to raise temperture
  delayMicroseconds((1023-temp_index*20.46)*6.84+1100); //Min 1100 Max 8100 usec
  digitalWrite(triac_pin,HIGH);
  digitalWrite(triac_pin,LOW);
}
else if(temp_index<=15){}//do nothing
}

/*
 * reads 6 bit binary message from pins
 */
int booleanRead(){
  int message=0;
  int add = 0;
  //this code would look better if it used a loop, but it would be slower to run.
  int pinVal = digitalReadFast(8);
  message += pinVal*bit(0);
  pinVal = digitalReadFast(9);
  message += pinVal*bit(1);
  pinVal = digitalReadFast(10);
  message += pinVal*bit(2);
  pinVal = digitalReadFast(11);
  message += pinVal*bit(3);
  pinVal = digitalReadFast(12);
  message += pinVal*bit(4);
  pinVal = digitalReadFast(13);
  message += pinVal*bit(5);
  /*
  for(int ii=0;ii<6;ii++){
    int pinVal = digitalRead(ii+8);
    int power = bit(ii);
    add = pinVal * power;
    message += add;
  }*/
  return message;
}
