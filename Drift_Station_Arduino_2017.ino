//#define output_pin 5
#include <SPI.h>
#include <digitalWriteFast.h>
unsigned long Timer;
const int CSpin =10;
const int ADQCS =9;
int temp_want;
int HV_want;
int HV_get;
int HV_prev;
int temp_index=15;
int old_temp_index=0;
 int n=0;
 int Voltage;
String HVBuffer;
String TEMPBuffer;
int TEMPData=0;
int HVData=0;
int heater_delay = 0;

String message;
  int requestedTemp;
  int requestedVolt;

  boolean validity = false;

  boolean hasT = false;
  boolean has_1 = false;
  boolean hasH = false;
  boolean has_2 = false;
  boolean hase = false; //has exclamation point

boolean eend= false;
boolean ehold= false;
boolean lctrip= false;
boolean temp0 = false;
boolean temp50 = false;
boolean temp70 = false;
boolean temp90 = false;
boolean temp95 = false;
boolean emergencycheck = false;
boolean run_heater = false;
void setup(){
  pinMode(7,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(2,OUTPUT);
  
  //pinMode(2,INPUT);
  //digitalWrite(2,HIGH);
  //pinMode(output_pin,OUTPUT);
  //attachInterrupt(0,acon,RISING);
  
  SPI.setBitOrder(MSBFIRST);
SPI.setClockDivider(SPI_CLOCK_DIV8);
SPI.setDataMode(SPI_MODE0);
pinMode(CSpin,OUTPUT); 
pinMode(ADQCS,OUTPUT);
  
  
SPI.begin();
//Serial.begin(57600);
Serial.begin(9600); //higher baud rate is crashing the system
//Serial.setTimeout(10); //ns was 10
delay(100);
}


/*start of loop*/
void loop() 
{
Serial.print("\n");
  
//HEATER TEMPERATURE
 float Voltage = ADQRead1()*.00244140625;
  float temp = (Voltage*33.78 +63.87);
      Serial.print("A");//temp
      Serial.print(temp);
      //Serial.print("\n");
     //Serial.print("Voltage=");
    //Serial.print(Voltage,DEC);
    //Serial.print("\n");
     delay(100);
//LEAKAGE CURRENT
  float LeakageCur= (ADQRead2()*.00244260/5 +0.223)/86;//
  Serial.print("B");//LC
  Serial.print(LeakageCur,DEC);
  //Serial.print("\n");
//TEMP and HV SETPOINTS
// if (Serial.available() >0){
 // delay(300);
   //HV_get= HVDataRetrieve();
   
 //}
//delay(300);
   //if (Serial.available() >0){
   //temp_want = TEMPDataRetrieve();//adress 1 for TEMP 
     
    
 //}
  
   //Serial.print("C");//HVSET
   //Serial.print(HV_want);
 //Serial.print("\n");
 Serial.print("C");//HVNOW
   Serial.print(n);
  //Serial.print("\n"); 
   //Serial.print("TEMPSET");
   //Serial.print(temp_want);
   Serial.print("D");
//Serial.print("\n");
//SEND OUT HV SIGNAL 
// if (abs(HV_get - HV_prev)<20){ //HV_prev used to avoid erroneous readings from affecting HV.
   HV_want =HV_get;//adress 0 for HV
  //Send HV signal
 // HV_prev=HV_get;
 //}
 if (n <HV_want){
   n=n+1;
 }
 if (n > HV_want){
   n=n-2;
 }

 //set voltage
 dacwriteA((n*8.19));
 
 //Determining how much to voltage to send to heat detector/plate
if( (temp_want - temp) > 0 && (temp_want - temp)< 2){
   
temp_index= (temp_want - temp)*17.5+15;}
else if ((temp_want - temp)>2){
   temp_index = 50;}
else {
  temp_index=15;

}
  



//Serial.print(i);

//whenever the heating rate is changed, it updates
if(old_temp_index != temp_index){
  binaryWrite(temp_index);
  //Serial.println(temp_index);
  old_temp_index = temp_index;
}

//serialEvent();
delay(900); //nate was 900

}

void acon(){
  //Serial.print("break start ");
  if(temp_index>15){ //want to raise temperture
 /* delayMicroseconds((1023-temp_index*20.46)*6.84+1100); //Min 1100 Max 8100 usec
  digitalWrite(output_pin,HIGH);
 // delayMicroseconds(100);
  digitalWrite(output_pin,LOW); */
  run_heater = true;
  heater_delay = ((1023-temp_index*20.46)*6.84+1100); //gabe added -500
}
else if(temp_index<=15){
  
}

}

/*
 * takes param message and turns it into binary signal sent through digital pins to other arduino
 * used to communicate the temperture_index from the blue arduino to the black arduino
 * 
 */
void binaryWrite(int message){
  //Serial.print("sending ");
  //Serial.print(message);
  //Serial.print(" binary ");
  int output[6];
  for(int ii=0;ii<6;ii++){
    output[ii] = message%2;
    //Serial.print(output);
    //digitalWrite(ii,output);
    message = ((message-(message%2))/2);
  }
  /*digitalwritefast is used instead of digitalwrite because it increases the speed by an order of magnitude*/
  digitalWriteFast(2,output[0]);
  digitalWriteFast(3,output[1]);
  digitalWriteFast(4,output[2]);
  digitalWriteFast(5,output[3]);
  digitalWriteFast(6,output[4]);
  digitalWriteFast(7,output[5]);
}

/*
 * Uses SPI protocall to communicate with the analog to digital converter
 * ADQCS pin is both output and input
 * SPI.transfer sends the byte in (*) and then returns the response that the analog to digital converter sends back
 * 
 * Read1 gets the voltage from the thermacouple
 */
int ADQRead1()//Analog Pins 0&1 +/-,   Differential Voltage 0 single 1, Bipolar 0,
                          //Start bit is first 1, Single-1/Differential-0, 
                          //Reverse Polarities(1) Normal Polarities(0),
                          // Adress 1-00 2-01 3-10, Unipolar-1 Bipolar-0
                          //MSBF 1 LSBF 0, Shutdown 1-inactive 0-active
            {
        byte inByte1 = 0;
        byte inByte2 = 0;
        digitalWrite(ADQCS,LOW);
        SPI.transfer(B00011000);//single ended, bipolar, Analog pin 1
        inByte1 =SPI.transfer(B01100000);
        inByte2=SPI.transfer(0);
        digitalWrite(ADQCS,HIGH);
        int AnalogVal= (inByte1<<8)| inByte2;
        if (AnalogVal > 0B100000000000){
        AnalogVal= ((AnalogVal ^ 0B100000000000)-2048);
        }
        else if(AnalogVal< 0B100000000000){
        AnalogVal=AnalogVal; 
        }
         
         return AnalogVal;
        }

/*
 * uses SPI to get the leakage current from the analog to digital converter
 */
int ADQRead2()
      {
        byte inByte1 = 0;
        byte inByte2 = 0;
        digitalWrite(ADQCS,LOW); //single ended, unipolar, analog pin 3 (skips pin 2 b/c it is the - for pin1)
        SPI.transfer(B00011001);
        inByte1 =SPI.transfer(B01100000);
        inByte2=SPI.transfer(0);
        digitalWrite(ADQCS,HIGH);
         int AnalogVal= (inByte1<<8)| inByte2;
        if (AnalogVal > 0B100000000000){
        AnalogVal= ((AnalogVal ^ 0B100000000000)-2048);
        }
        else if(AnalogVal< 0B100000000000){
        AnalogVal=AnalogVal; 
        }
         
         return AnalogVal;
        }


/*
 * not sure what this is for
 * 
 * it's not used anywhere else in the code
 */
int ADQRead3(){
        byte inByte1 = 0;
        byte inByte2 = 0;
        digitalWrite(ADQCS,LOW);
        SPI.transfer(B00010010);
        inByte1 =SPI.transfer(B01100000);
        inByte2=SPI.transfer(0);
        digitalWrite(ADQCS,HIGH);
        int AnalogVal= (inByte1<<8)| inByte2;
        if (AnalogVal > 0B100000000000){
        AnalogVal= ((AnalogVal ^ 0B100000000000)-2048);
        }
        else if(AnalogVal< 0B100000000000){
        AnalogVal=AnalogVal; 
        }
        return AnalogVal;
        }


/*
 * used to set the voltage in the drifter
 */
void dacwriteA(int value){
      int DATA = value >> 8 ;
      int DATA1= DATA +B00110000;//0-active,  buffered input(1 for internal Vref 0 for external),1 regulargain, 1 shutdown off
      int DATA2 = value;
      digitalWrite(CSpin,LOW); 
      SPI.transfer(DATA1);
      SPI.transfer(DATA2);
      digitalWrite(CSpin,HIGH);
      //Serial.println( DATA1 +48,BIN);
      //Serial.println(DATA2,BIN);
}

/*
 * not used anywhere in the code
 */
void dacwriteB(int value){
      int DATA = value >> 8 ;
      int DATA1= DATA +B10110000;//0A 1B,  buffered input(1 for internal Vref 0 for external),1 regulargain, 1 shutdown off
      int DATA2 = value;
      digitalWrite(CSpin,LOW); 
      SPI.transfer(DATA1);
      SPI.transfer(DATA2);
      digitalWrite(CSpin,HIGH);
      //Serial.println( DATA1 +48,BIN);
      //Serial.println(DATA2,BIN);
}



/*
int TEMPDataRetrieve(){ //input DataRetrieve("HV") or DataRetrieve("TEMP") in loop
 
      
        
        if (Serial.available()>0){
          String DataSeperate = String(char(Serial.peek()));
            if (DataSeperate == "T"){
              
              String AdressErase= String(char(Serial.read()));
              while(Serial.available() >0){              
                   TEMPBuffer += String(char(Serial.read())); 
              TEMPData= TEMPBuffer.toInt();
            }                          
         Serial.flush();
          TEMPBuffer=('0');
         
         
        }
       
     }
        
     return  TEMPData;      
    }
       
       



int HVDataRetrieve(){ //input DataRetrieve("HV") or DataRetrieve("TEMP") in loop
 

 
     
        if (Serial.available()>0){
          String DataSeperate = String(char(Serial.peek()));
            if (DataSeperate == "H"){
             
              String AdressErase= String(char(Serial.read()));
              while(Serial.available() >0){              
                   HVBuffer += String(char(Serial.read())); 
              HVData= HVBuffer.toInt();
          
          }                          
          Serial.flush();
          HVBuffer=('0');
        
           
        }
      
    
      
     }
         
      return  HVData;
      }
   */    

       /* 
void serialEvent(){

 String inBuffer_T;
 String inBuffer_H;
 int lf=95;   
     
if ( Serial.available() > 0) {  // If data is available,
            
      int address = Serial.read(); //Reads the address bit, i.e A,B,C
         
         //Case statements break up data into variables by using the address bit of address variable.  
        switch(address){
          
          case 'T':
                       inBuffer_T =  Serial.readStringUntil(lf);
                        for (int i=0; i < inBuffer_T.length(); i++){
                         if (inBuffer_T[i]== lf ){
                           inBuffer_T[i] = 0;
                           }
                        }   
                       temp_want = inBuffer_T.toInt();
                     
                     break;                 
                
          case 'H':
                       inBuffer_H =  Serial.readStringUntil(lf);
                        for (int i=0; i < inBuffer_H.length(); i++){
                         if (inBuffer_H[i]== lf ){
                           inBuffer_H[i] = 0;
                           }
                        }   
                       HV_get = inBuffer_H.toInt();              
                     
                     break;     
                  }
                
        }
}

*/

/*
 * old (buggy) serialEvent code
 * 
void serialEvent(){

 String inBuffer_T;
 String inBuffer_H;
 String data = ""; //g
 int lf='_';
     
while ( Serial.available()) {  // If data is available, //Gabe changed from if to while so it processes all data before moving on
  //lines marked with "//g" are lines Gabe added for debugging and aren't necessary
            
      char address = Serial.read(); //Reads the address bit, i.e A,B,C
      if(address != 10){
        data += address; //g
        Serial.print(" address is ");
      Serial.print(address);
      }
      else {
        data += "10";
      }
      
         //Case statements break up data into variables by using the address bit of address variable.  
        switch(address){
          
          case 'T':
                       inBuffer_T =  Serial.(lf);
                       data += inBuffer_T; //greadStringUntil
                       data += "_"; //g
                        for (int i=0; i < inBuffer_T.length(); i++){
                         if (inBuffer_T[i]== lf ){
                           inBuffer_T[i] = 0;
                           Serial.print(" if statement T was called");
                           }
                        }   
                        Serial.print(" inBuffer_T is equal to ");
                        Serial.print(inBuffer_T);
                        Serial.print(", tempwant = ");
                        Serial.print(inBuffer_T.toInt());
                       temp_want = inBuffer_T.toInt();
                     
                     break;                 
                
          case 'H':
                       inBuffer_H =  Serial.readStringUntil(lf);
                       data += inBuffer_H;
                       data += "_";
                        for (int i=0; i < inBuffer_H.length(); i++){
                         if (inBuffer_H[i]== lf ){
                           inBuffer_H[i] = 0;
                           Serial.print(" if statement H was called");
                           }
                        }   
                        Serial.print(" inBuffer_H is equal to ");
                        Serial.print(inBuffer_H);
                        Serial.print(", HV_get = ");
                        Serial.print(inBuffer_H.toInt());
                       HV_get = inBuffer_H.toInt();              
                     
                     break;     
                  }
                
        }
        Serial.print(" the entire string arduino recieved is: ");
        Serial.print(data);
}
*/


/*
 * takes a message and a "marker" char. reads through message until it sees marker char.
 * returns everything after marker char until '_'
 * 
 * param message - the string that the parser is looking through
 * param marker - the character that the parser is looking for. the integer between marker and '_' is returned
 * return value - the integer in the message between the marker and an underscore ('_') - returns -1 if no value is found
 */
int parse_Message(String message, char marker){
  
  int value = -1; //the integer value that we're looking for and will return
  int subStart = -1; //the start of the substring we want to pull from the message
  int subEnd = -1; //the end of the substring we want to parse from the message
  
  for(int ii=0; ii < message.length(); ii++){
    if(message.charAt(ii) == marker){
      subStart = ii+1;
    }
    if(message.charAt(ii) == '_'){
      subEnd = ii+1;
    }
  }
  if(subStart != -1 && subEnd != -1 && subStart < subEnd){
    if(subEnd-subStart > 1){ //this is to make sure arduino doesn't read something like "H_" as set HV to 0. Bad labview communication.
      value = message.substring(subStart,subEnd).toInt();
    }
  }
  return value;
}

/*
 * makes sure that passed message isn't corrupted. (labview often sends broken messages to arduino) Gabe fix
 * checks there's a 'T' then an '_' then an 'H' then another '_' then an '!'
 * 
 * note: this is no longer necessary because we fixed the problem that was causing labview to send corrupted messages
 * but there's no harm in keeping it in. -Gabe
 * 
 * param message - the message the function checks for validity
 * return validity - boolean - true if message valid, false if not
 */
boolean messageValid(String message){
  validity = false;

  hasT = false;
  has_1 = false;
  hasH = false;
  has_2 = false;
  hase = false; //has exclamation point


  for(int ii=0; ii < message.length(); ii++){
      if(message.charAt(ii) == 'T'){
        hasT = true;
      }
    if(hasT){
      if(message.charAt(ii) == '_'){
        has_1 = true;
      }
    }
    if(hasT && has_1){
      if(message.charAt(ii) == 'H'){
        hasH = true;
      }
    }
    if(hasT && has_1 && hasH){
      if(message.charAt(ii) == '_'){
        has_2 = true;
      }
    }
    if(hasT && has_1 && hasH && has_2){
      if(message.charAt(ii) == '!'){
        hase = true;
      }
    }
    
  }

  /* Nate hard coded checks ~ no longer necessary
  if(message.charAt(message.length()-1) == '@'){
        ehold = true;
        eend = false;
        lctrip = false;
        emergencycheck = true;
      } else if(message.charAt(message.length()-1) == '*'){
        eend = true;
        ehold = false;
        lctrip = false;
        emergencycheck = true;
      }else if(message.charAt(message.length()-1) == '$'){
        lctrip = true;
        ehold = false;
        eend = false;
        emergencycheck = true;
      } else if(message.charAt(message.length()-1) == '%'){
        temp0 = false;
        temp50 = true;
        temp70 = false;
        temp90 = false;
        temp95 = false;
        emergencycheck = true;
      } else if(message.charAt(message.length()-1) == '&'){
        temp0 = false;
        temp50 = false;
        temp70 = true;
        temp90 = false;
        temp95 = false;
        emergencycheck = true;
      } 
      else if(message.charAt(message.length()-1) == '?'){
        temp0 = false;
        temp50 = false;
        temp70 = false;
        temp90 = true;
        temp95 = false;
        emergencycheck = true;
      } else if(message.charAt(message.length()-1) == '#'){
        temp0 = false;
        temp50 = false;
        temp70 = false;
        temp90 = false;
        temp95 = true;
        emergencycheck = true;
      } else if(message.charAt(message.length()-1) == ')'){
        temp0 = true;
        temp50 = false;
        temp70 = false;
        temp90 = false;
        temp95 = false;
        emergencycheck = true;
      } */
  validity = (hasT && has_1 && hasH && has_2 && hase); //redundant but for the sake of readability
  return validity;
}



/*
 * improvement on old serialEvent() function
 * a string containing the desired temperture and voltage is passed from labview to the arduino.
 * for example, if the temperture is set to 40 and the voltage is set to 50, labview will pass a string that reads
 * "T40_H50_!"
 * 
 * This function reads the string and parses it into a temperture and voltage integer
 * 
 * note: serialEvent() is a special type of function and doesn't need to be explictly called in the loop
 * it's automatically performed every time the arduino finishes a loop
 */
 
void serialEvent()
{
  

  while(Serial.available()) {
  
    message = Serial.readStringUntil(10); //n was 10, actual Serial termchar is 65
    //Serial.print(" recieved message: ");
    //Serial.print(message);
    if(messageValid(message) == true || emergencycheck == true){
      requestedTemp = parse_Message(message, 'T');
      if(requestedTemp != -1){
        temp_want = requestedTemp;
      }
      requestedVolt = parse_Message(message, 'H');

      if(requestedVolt != -1){
        HV_get = requestedVolt;
      } 

      /* Nate hard coded - no longer necessary
      if (temp0==true){
      temp_want = 0; 
      HV_get = 250;
      }else if (temp50==true){
      temp_want = 50; 
      HV_get = 250;
      } else if (temp70==true){
      temp_want = 70; 
      HV_get = 250;
      } else if (temp90==true){
      temp_want = 90; 
      HV_get = 250;
      }else if (temp95==true){
      temp_want = 95; 
      HV_get = 250;
      }*/
     
      
      /* Nate hard code
      if(eend == true){
        HV_get = 0;
        temp_want = 0;
      } else if(ehold == true){
       HV_get = 250;
       if(temp50 == true){
        temp_want = 50;
       }if(temp0 == true){
        temp_want = 0;
       }if(temp70 == true){
        temp_want = 70;
       }if(temp90 == true){
        temp_want = 90;
       }if(temp95 == true){
        temp_want = 95;
       }*/
       
       /*else if (lctrip==true){ //nate hard code?
      HV_get=250;
      temp_want=0;
      } */

      /* for debugging
      Serial.print(" parsed temperture as: ");
     Serial.print(requestedTemp);
     Serial.print(" temp_want is now: ");
     Serial.print(temp_want);
      Serial.print(" parsed voltage as: ");
      Serial.print(requestedVolt);
      Serial.print(" HV_get is now: ");
      Serial.print(HV_get);*/
      Serial.print("recieved");
    }
    else {
      Serial.print("invalid");
    }
  }
}









