#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include <WebConfig.h>
#include <Wiegand.h>

// #define RELAY_PIN       14
 #define PIN_D0          12 // green wire
 #define PIN_D1          13 // white wire
// #define GREEN_LED_PIN   5  // blue wire
// #define BEEP_PIN        4  // yellow wire
                           // brown wire is common ground with pad and ESP 
                           // grey I DONT KNOW YET
/*
#define RELAY_PIN       14
#define PIN_D0          5 // green wire
#define PIN_D1          4 // white wire
//#define GREEN_LED_PIN   5  // 
//#define BEEP_PIN        4  //                            

*/
#define RELAY_TIMER     3  //seconds the door latch activates



WIEGAND wg;

int RELAY_PIN;
//int PIN_D0;           // green wire
//int PIN_D1;           // white wire
int GREEN_LED_PIN;    // blue wire
int BEEP_PIN;         // yellow wire
char bufa[20];
String bufs;
String bufactive;
unsigned char codebuf[26];
// int codebuf[26];
int serbuf[32];
int seradd;
char cardstring[26];
char myaddbuf[2];
int keycount;
int incomingByte = 0;
bool webactive = true;
int addr = 0;
int sizevalue;
int numusers; // a global that stores number of users on file

String params = "["
  "{"
  "'name':'ssid',"
  "'label':'Name of WLAN',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'pwd',"
  "'label':'WLAN Password',"
  "'type':"+String(INPUTPASSWORD)+","
  "'default':''"
  "},"
  "{"
  "'name':'relay',"
  "'label':'Relay Pin',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':14,"
  "'default':'14'"
  "},"
  "{"
  "'name':'d0',"
  "'label':'Green wire D0 Pin',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':14,"
  "'default':'12'"
  "},"
  "{"
  "'name':'d1',"
  "'label':'White Wire D1 Pin',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':14,"
  "'default':'13'"
  "},"
  "{"
  "'name':'led',"
  "'label':'Blue Wire (LED) Pin',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':14,"
  "'default':'5'"
  "},"
  "{"
  "'name':'buzzer',"
  "'label':'Buzzer Wire Pin',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':14,"
  "'default':'4'"
  "},"
  "{"
  "'name':'admn',"
  "'label':'Admin Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'1234'"
  "},"
  "{"
  "'name':'usr1',"
  "'label':'User 1 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr2',"
  "'label':'User 2 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr3',"
  "'label':'User 3 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'ust4',"
  "'label':'User 4 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr5',"
  "'label':'User 5 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr6',"
  "'label':'User 6 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr7',"
  "'label':'User 7 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr8',"
  "'label':'User 8 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr9',"
  "'label':'User 9 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'usr10',"
  "'label':'User 10 Code',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'activeusers',"
  "'label':'Select Active Users',"
  "'type':"+String(INPUTMULTICHECK)+","
  "'options':["
  "{'v':'0','l':'User 1'},"
  "{'v':'1','l':'User 2'},"
  "{'v':'2','l':'User 3'},"
  "{'v':'3','l':'User 4'},"
  "{'v':'4','l':'User 5'},"
  "{'v':'5','l':'User 6'},"
  "{'v':'6','l':'User 7'},"
  "{'v':'7','l':'User 8'},"
  "{'v':'8','l':'User 9'},"
  "{'v':'9','l':'User 10'}],"
  "'default':''"
  "}"
  "]";

ESP8266WebServer server;
WebConfig conf;

boolean initWiFi() {
    boolean connected = false;
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.print(conf.values[0]);
    Serial.println(" network");
    if (conf.values[0] != "") {
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
      uint8_t cnt = 0;
      while ((WiFi.status() != WL_CONNECTED) && (cnt<20)){
        delay(500);
        Serial.print(".");
        cnt++;
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP-Address = ");
        Serial.println(WiFi.localIP());
        connected = true;
      }
    }
    if (!connected) {
          WiFi.mode(WIFI_AP);
          WiFi.softAP(conf.getApName(),"",1);  
    }
    return connected;
}

void handleRoot() {
  conf.handleFormRequest(&server);
  if (server.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Configuration ************");
    for (uint8_t i = 0; i<cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
    if (conf.getBool("switch")) Serial.printf("%s %s %i %5.2f \n",
                                conf.getValue("ssid"),
                                conf.getString("continent").c_str(), 
                                conf.getInt("amount"), 
                                conf.getFloat("float"));
  }
}

void setup() {
  Serial.begin(9600);
// setup wiegand stuff
  seradd = 0;
  strcpy(bufa,"");
  keycount=0;
  // default Wiegand Pin 2 and Pin 3 see image on README.md
  // for non UNO board, use wg.begin(pinD0, pinD1) where pinD0 and pinD1 
  // are the pins connected to D0 and D1 of wiegand reader respectively.
 // wg.begin(PIN_D0,PIN_D1); // setup wiegard data pin

// setup config file and wifi stuff
  Serial.println(params);
  conf.setDescription(params);
  conf.readConfig();
// assign pins from config file  
  RELAY_PIN = conf.getInt("relay");
  BEEP_PIN = conf.getInt("buzzer");
  GREEN_LED_PIN = conf.getInt("led");
//  PIN_D0 = conf.getInt("d0");
//  PIN_D1 = conf.getInt("d1");
  // default Wiegand Pin 2 and Pin 3 see image on README.md
  // for non UNO board, use wg.begin(pinD0, pinD1) where pinD0 and pinD1 
  // are the pins connected to D0 and D1 of wiegand reader respectively.
  wg.begin(PIN_D0,PIN_D1); // setup wiegard data pin

  
  Serial.print ("Relay is at GPIO");
  Serial.println (RELAY_PIN);// setup relay stuff
  Serial.print ("LED is at GPIO");
  Serial.println (GREEN_LED_PIN);// setup relay stuff
  pinMode (RELAY_PIN, OUTPUT); // setup relay pin for output  
  pinMode (GREEN_LED_PIN, OUTPUT); // setup relay pin for output 
  pinMode (BEEP_PIN, OUTPUT); // setup relay pin for output 
  digitalWrite(GREEN_LED_PIN, LOW);
  beep ();
//  digitalWrite(BEEP_PIN, HIGH);
// only connect wifi is web actice 
 if (webactive) { 
  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder started");
  }
  server.on("/",handleRoot);
  server.begin(80);
 };
}

void beep () {
  digitalWrite(BEEP_PIN, HIGH);
  delay (1000);
  digitalWrite(BEEP_PIN, LOW);
}

void webUI () {
  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder started");
  }
  server.on("/",handleRoot);
  server.begin(80);
//  server.handleClient();
//  MDNS.update();  
}

void triggerrelay() {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
 //   Serial.println("Relay on");
    delay (RELAY_TIMER*1000);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
 //   Serial.println("Relay off"); 
 
}


void makecode(long addbuf) {
    
    switch (addbuf) {
      case 13:
    //    Serial.println("ENT pressed");
        
        Serial.print("Buffer size = ");
        Serial.print(codebuf[0]);
        Serial.print("for code ");
        Serial.println(bufa);
//        if (bufa = code) {Serial.println("MATCHED!") };
        // after enter clear things up
     //   strcpy(bufa,"");
        strcat(bufa,"\0");
        keycount=0;
        bufs = String(bufa);  // convert char array entered to a string
        // PUT THE COMPARE ROUTINE IN HERE
        comparecode();
   
       // readalleprom();
        break;
      case 1:
   //     Serial.println("1 pressed");
        strcat(bufa,"1");
        
        break;        
      case 2:
    //    Serial.println("2 pressed");
        strcat(bufa,"2");
        break;
      case 3:
    //    Serial.println("3 pressed");
        strcat(bufa,"3");
       
        break;
      case 4:
     //   Serial.println("4 pressed");
        strcat(bufa,"4");
        
        break;
       case 5:
     //   Serial.println("5 pressed");
        strcat(bufa,"5");
        
        break;
      case 6:
   //     Serial.println("6 pressed");
        strcat(bufa,"6");
      
        break;
      case 7:
     //   Serial.println("7 pressed");
        strcat(bufa,"7");
       
        break;
      case 8:
    //    Serial.println("8 pressed");
        strcat(bufa,"8");
        
        break;
      case 9:
     //   Serial.println("9 pressed");
        strcat(bufa,"9");
        break;
      case 27:
        Serial.println("ESC pressed");
        strcpy(bufa,"");
        keycount=0;
       
        break;
      case 0:
    //    Serial.println("0 pressed");
        strcat(bufa,"0");
        break;

      default:
    // statements
      break;
    }
    
}

void comparecode () {
  int match;
  String activelist;
  char activeusr;

  match=0; // starts with no match in case there are no bytes
  Serial.print (" Is active test =");
  Serial.print (conf.getValue("activeusers"));
  Serial.print ("=");
  activelist = conf.getValue("activeusers");
  activeusr = activelist.charAt(1);  
  Serial.println (activeusr);
  Serial.print (" code entered =");
  Serial.print (bufs);
  Serial.println ("=");
    
//  String bufs = String(bufa);  // convert char array entered to a string
  if (bufs == "99991") {Serial.println ("ENTERING WEB UI MODE");webactive = true;webUI();};
  if (bufs == "99990") {Serial.println ("SHUTTING DOWN WEB UI MODE");webactive = false;};
  if (bufs == conf.getValue("admn")) {Serial.println ("WE HAVE A MATCH Admin");
      match = 1;
      strcpy(bufa,"");
      bufs = "00000";
  }
  if (bufs == conf.getValue("usr1")) {
      Serial.println ("WE HAVE A MATCH User 1");
      activeusr = activelist.charAt(0);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr2")) {Serial.println ("WE HAVE A MATCH User 2");
      activeusr = activelist.charAt(1);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr3")) {Serial.println ("WE HAVE A MATCH User 3");
      activeusr = activelist.charAt(2);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr4")) {Serial.println ("WE HAVE A MATCH User 4");
      activeusr = activelist.charAt(3);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr5")) {Serial.println ("WE HAVE A MATCH User 5");
      activeusr = activelist.charAt(4);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr6")) {Serial.println ("WE HAVE A MATCH User 6");
      activeusr = activelist.charAt(5);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr7")) {Serial.println ("WE HAVE A MATCH User 7");
      activeusr = activelist.charAt(6);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr8")) {Serial.println ("WE HAVE A MATCH User 8");
      activeusr = activelist.charAt(7);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
   if (bufs == conf.getValue("usr9")) {Serial.println ("WE HAVE A MATCH User 9");
      activeusr = activelist.charAt(8);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr10")) {Serial.println ("WE HAVE A MATCH User 10");
      activeusr = activelist.charAt(9);  
      if (activeusr == '1') {
         match = 1;};
      Serial.println (activeusr);
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (match==1) {
      triggerrelay();
     
      }
      else Serial.println ("NO MATCH") ;
  strcpy(bufa,"");
  bufs = "00000";
 }

void processcmd (int cmd) {
  if (cmd == 97) {
    Serial.println("Time to add: ");
        
    
    };  //a
  
  if (cmd == 99) {Serial.println("Time to clear: ");};  //c
  
  
  if (cmd == 114) {Serial.println("Time to relay1: ");};  //r
  
  
  if (cmd == 82) {Serial.println("Time to relay2: ");};  //R
  
  
  if (cmd == 103) {Serial.println("Time to green light: ");}; //g
  
  
  if (cmd == 98) {Serial.println("Time to beep: ");};  //b
}

void loop() {
  // put your main code here, to run repeatedly:
  keyread_loop();
  // above
  if (webactive) {
      server.handleClient();
      MDNS.update(); }
}

volatile long reader1 = 0;
volatile int reader1Count = 0;
int code_B;
int code_C;
int code_A;
int i;


void keyread_loop() {
  //SERIAL input section 

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    processcmd(incomingByte);
     // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte);
 //   Serial.println(incomingByte, HEX);
  }
//  serbuf[seradd]=incomingByte;
//  seradd=seradd+1;
//  if (seradd==3) {processcmd();};  // if a 3 byte code is recieved, process it
   
  // WIEGAND input section
  if(wg.available()) {
/*
  long int rawToken = (reader1 >> 1) & 0xffffff); // remove both parity bits
  int facilityCode = rawToken >> 16; // just get the top 8 bits
  int userID = rawToken & 0xffff; // bottom 16 bits are the user code
  Serial.print("Facility Code ");
  Serial.println(facilityCode); // use Serial.println(facilityCode, HEX); // to see the result in hex
  Serial.print("User ID ");
  Serial.println(userID);
  reader1 = 0;
  reader1Count = 0;
*/     

    
    unsigned long code = wg.getCode();
    int wtype = wg.getWiegandType();
    // Card data

    if(code > 27) {  // chane to > 27 
      long int rawToken = (code & 0xffffff);
      int facilityCode = (rawToken & 0x1F0000) >> 16;
      int cardCode = (rawToken & 0x3FFF) >> 16;
      int cardCodeb = (rawToken & 0xffff) >> 8;
 //     unsigned long facilityCode = (code & 0x1F0000) >> 16;
 //     unsigned long cardCode = code & 0x3FFF;
      code_B = code / 100000;
      code_A = code - (code_B * 100000);
    //  Serial.println(code, HEX);
/*     
      codebuf[3] = (int)((code >> 24) & 0xFF) ;
      codebuf[2] = (int)((code >> 16) & 0xFF) ;
      codebuf[1] = (int)((code >> 8) & 0XFF);
      codebuf[0] = (int)((code & 0XFF));


      codebuf[3] = (int)((code >> 24) & 0xFF) ;
     
      codebuf[0] = (int)((rawToken & 0x1F0000) >> 16); // facility code
      codebuf[2] = (int)((rawToken & 0x3FFF) >> 16);
      codebuf[1] = (int)((rawToken & 0xffff) >> 8);
*/
     //     codebuf[0] = (int)((rawToken & 0x1F0000) >> 16);    
      String myStr;
      bufs = String(rawToken);
     // myStr = String(rawToken);
      Serial.println(myStr);
      comparecode();
    }
    // Keypad data
    else {
      if (code != 13) {
        keycount=keycount+1; //Don't count the ENT key as key pressed
        codebuf[keycount] = code; //add code to code buffer only if its not ENT
      };  
  //    Serial.print("press #");Serial.println(keycount);
  //size of string is equal to keycount
      codebuf[0]=keycount;  //size of string is equal to keycount
      makecode(code);
      
 //     Serial.print (code);Serial.print (" pressed for a toal of ");Serial.println (codebuf[0]);
      if (keycount==12) {
//        Serial.println("Maximum keys pressed, resetting buffer");
        strcpy(bufa,"");
        keycount=0;
      }
    }
  }
}
