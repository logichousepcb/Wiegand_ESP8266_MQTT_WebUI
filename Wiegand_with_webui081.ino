#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


#include <WebConfig.h>  //https://github.com/GerLech/WebConfig
#include <Wiegand.h>
#include <PubSubClient.h>   //https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>     //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA
/************************ PIN DEFINITIONS ********************************/
#define RELAY_PIN       14 // (D5)
#define PIN_D0          12 // green wire (D6)
#define PIN_D1          13 // white wire (D7)
#define GREEN_LED_PIN   5  // blue wire (D1)
#define BEEP_PIN        4  // yellow wire (D2)
                           // brown wire is common ground with pad and ESP 
                           // grey I DONT KNOW YET
/************************ PIN DEFINITIONS ********************************/



#define RELAY_TIMER     4  //seconds the door latch activates


WIEGAND wg;


char bufa[20];
String bufs;
String bufn;
String bufactive;
unsigned char codebuf[26];
int serbuf[32];
int seradd;
char myaddbuf[2];
int keycount;
int incomingByte = 0;
bool webactive = true;
int addr = 0;
int sizevalue;
int numusers; // a global that stores number of users on file
// bool codedone = false;

String finalcode;
int codercv = 0;
bool addrcv = false;

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
  
WiFiClient client;
ESP8266WebServer server;
// ESP8266Client espClient;
//HARestAPI ha(espClient);
WebConfig conf;

const char* host = "192.168.1.130";
const char* ssid = "ESP";
const char* password = "doodlehead";
const char* ha_ip = "192.168.1.130";
uint16_t ha_port = 8123; // Could be 443 is using SSL
const char* ha_pwd = "doodlehead";  //long-lived password. On HA, Profile > Long-Lived Access Tokens > Create Token
String fingerprint = "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19";

boolean initWiFi() {
    boolean connected = false;
    WiFi.mode(WIFI_STA);
    Serial.print("SSID-");
    Serial.println(conf.values[0]);
    if (conf.values[0] != "") {
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
      uint8_t cnt = 0;
      while ((WiFi.status() != WL_CONNECTED) && (cnt<20)){
        // fast flashing connecting to network  
        digitalWrite(GREEN_LED_PIN, LOW);
        delay (5);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay (5);
        delay(490);
      //  Serial.print(".");
        cnt++;
      }
    //  Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("UIIP-");
        Serial.println(WiFi.localIP());
        connected = true;
    //    ha.setHAServer(ha_ip, ha_port);
    //    ha.setHAPassword(ha_pwd);
    //    ha.setFingerPrint(fingerprint); // Only used if HA is running SSL
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
// setup config file and wifi stuff
//  Serial.println(params);
  conf.setDescription(params);
//  Serial.println("this is the result of readConfig");
  conf.readConfig();
 
// assign pins from config file  
//  RELAY_PIN = conf.getInt("relay");
//  BEEP_PIN = conf.getInt("buzzer");
//  GREEN_LED_PIN = conf.getInt("led");
//  PIN_D0 = conf.getInt("d0");
//  PIN_D1 = conf.getInt("d1");
  wg.begin(PIN_D0,PIN_D1); // setup wiegard data pin


  pinMode (RELAY_PIN, OUTPUT); // setup relay pin for output  
  pinMode (GREEN_LED_PIN, OUTPUT); // setup relay pin for output 
  pinMode (BEEP_PIN, OUTPUT); // setup relay pin for output 
  digitalWrite(GREEN_LED_PIN, HIGH);

  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("WEBUI-ON");
    pulseled (5);
  }
  server.on("/",handleRoot);
  server.begin(80);
 
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
    Serial.println("WEBUI-ON");
    pulseled (5);
  }
  server.on("/",handleRoot);
  server.begin(80);
//  server.handleClient();
//  MDNS.update();  
}

void pulseled (int flashcount) {
    for (int i = 0; i <= flashcount-1; i++) {
    digitalWrite(GREEN_LED_PIN, LOW);
    delay (500);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay (500);
    } 
}

void triggerrelay() {
   digitalWrite(RELAY_PIN, HIGH);
   pulseled (RELAY_TIMER);
   digitalWrite(RELAY_PIN, LOW);

/*    Serial.println("[Sending a request]");
    client.print(String("GET /") + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n"
                );

    Serial.println("[Response:]");
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
   */
  //  ha.setURL("/api/services/light/toggle");
  //  ha.sendHAComponent("switch.office_light");
}


void makecode(long addbuf) {

    
    switch (addbuf) {
      case 13:
        strcat(bufa,"\0");
        keycount=0;
        bufs = String(bufa);  // convert char array entered to a string
        codercv = 1;
        
        break;
      case 1:
        strcat(bufa,"1");

        break;        
      case 2:
        strcat(bufa,"2");
      
        break;
      case 3:
        strcat(bufa,"3");
      
        break;
      case 4:
        strcat(bufa,"4");
       
        break;
      case 5:
        strcat(bufa,"5");
       
        break;
      case 6:
        strcat(bufa,"6");
        
        break;
      case 7:
        strcat(bufa,"7");
        
        break;
      case 8:
        strcat(bufa,"8");
       
        break;
      case 9:
        strcat(bufa,"9");
       
        break;
      case 27:
        strcpy(bufa,"");
       
        keycount=0;
        break;
      case 0:
        strcat(bufa,"0");
        
        break;
      default:
    // statements
      break;
    }
    
}
void addUser () {
  String outline;
  String bufnlist;
  int usrnum;
//  digitalWrite(GREEN_LED_PIN, LOW); // LED solid green waiting for user number
  //enter 1 digit user code 1-9
  pulseled (6);
  if(wg.available()) {
     unsigned long usrnumin = wg.getCode();
     Serial.print ("ADD-");
     Serial.print (usrnumin);
     Serial.print ("-");
     usrnum = usrnumin; 
  }
//  digitalWrite(GREEN_LED_PIN, HIGH); // LED solid green waiting for code
  delay (1000); // Delay before turning LED green again waiting for code 
  
  // wait for certain time for input
  
  for (int thisPin = 1; thisPin < 400; thisPin++) {
      //Serial.print (".");
      digitalWrite(GREEN_LED_PIN, LOW); // LED solid green waiting for code
      delay (10);
      getnewcode ();
      };
  digitalWrite(GREEN_LED_PIN, HIGH);  // LED off after code enter timer ends
  Serial.println (bufn);
//  Serial.println ("DONE WITH ADD");
  switch (usrnum) {
      case 1:
        conf.setValue("usr1",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;            
      case 2:
        conf.setValue("usr2",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;               
      case 3:
        conf.setValue("usr3",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;                
      case 4:
        conf.setValue("usr4",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;        
      case 5:
        conf.setValue("usr5",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;           
      case 6:
        conf.setValue("usr6",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;        
      case 7:
        conf.setValue("usr7",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;        
      case 8:
        conf.setValue("usr8",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;        
      case 9:
        conf.setValue("usr9",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(usrnum-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;              
      case 0:
        conf.setValue("usr10",bufn);
        bufnlist = conf.getValue("activeusers");
        bufnlist.setCharAt(10-1,'1');  
        conf.setValue("activeusers",bufnlist);
        conf.writeConfig();
        break;             
      default:
    // statements
      break;
    }

  
                       
}

void comparecode (String compUser) {
  int match;
  String activelist;
  char activeusr;

  match=0; // starts with no match in case there are no bytes
  activelist = conf.getValue("activeusers");
  activeusr = activelist.charAt(1);  
  Serial.print ("CODE-");
  Serial.print (compUser);
  Serial.print ("-");
  if (compUser == "99992") {Serial.println ("99-C");
                        conf.setValue("usr5","3555997");
                       
                        Serial.print ("ADD-");
                        Serial.print ("user5-");
                        Serial.print (conf.getValue("usr5"));
                        conf.writeConfig();
                       };  
  if (compUser == "99997") {Serial.println ("99-C");addrcv=true;};
  if (compUser == "99991") {Serial.println ("99-C");webactive = true;webUI();};
  if (compUser == "99990") {Serial.println ("99-C");pulseled (2); webactive = false;};
  if (compUser == conf.getValue("admn")) {Serial.print ("00-");
      match = 1;
      strcpy(bufa,"");
      bufs = "00000";
  }
  if (compUser == conf.getValue("usr1")) {
      Serial.print ("01-");
      activeusr = activelist.charAt(0);  
      if (activeusr == '1') {
         
         match = 1; }; 
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr2")) {Serial.print ("02-");
      activeusr = activelist.charAt(1);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr3")) {Serial.print ("03-");
      activeusr = activelist.charAt(2);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr4")) {Serial.print ("04-");
      activeusr = activelist.charAt(3);  
      if (activeusr == '1') {
         match = 1;};
    
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr5")) {Serial.print ("05-");
      activeusr = activelist.charAt(4);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr6")) {Serial.print ("06-");
      activeusr = activelist.charAt(5);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr7")) {Serial.print ("07-");
      activeusr = activelist.charAt(6);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr8")) {Serial.print ("08-");
      activeusr = activelist.charAt(7);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
   if (compUser == conf.getValue("usr9")) {Serial.print ("09-");
      activeusr = activelist.charAt(8);  
      if (activeusr == '1') {
         match = 1;};
 
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (compUser == conf.getValue("usr10")) {Serial.print ("10-");
      activeusr = activelist.charAt(9);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } // else Serial.print ("99-") ; 
  if (match==1) {
      Serial.println ("G");
      triggerrelay();
      codercv = true;
      }
      else {//Serial.println ("B") ;
             codercv = true;};  
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
  // put code here, to run repeatedly:
  keyread_loop();
  if (codercv == 1) { comparecode(bufs);codercv=0;};
  if (addrcv) {addUser();codercv=0;addrcv=false;};
  server.handleClient();
  MDNS.update();
  
}

volatile long reader1 = 0;
volatile int reader1Count = 0;
//int code_B;
//int code_C;
//int code_A;
int i;


void keyread_loop() {
  //SERIAL input section 
/*
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    processcmd(incomingByte);
     // say what you got:
    Serial.print("RECV-");
    Serial.println(incomingByte, HEX);
  }
*/
  // WIEGAND input section
  if(wg.available()) {
    unsigned long code = wg.getCode();
    int wtype = wg.getWiegandType();
    // Card data

    if(code > 27) {  // chane to > 27 
      long int rawToken = (code & 0xffffff);
      bufs = String(rawToken);
      codercv = 1;
      
    }

    // Keypad data
    else {
      if (code != 13) {
        keycount=keycount+1; //Don't count the ENT key as key pressed
        codebuf[keycount] = code; //add code to code buffer only if its not ENT
      };  
      codebuf[0]=keycount;  //size of string is equal to keycount
      makecode(code);
  //    if (codedone = true) { comparecode(bufs);};
      if (keycount==12) {

    //    Serial.println("Maximum keys pressed, resetting buffer");
        strcpy(bufa,"");
        keycount=0;
      }
    }
  }
}  
void getnewcode () {
  // WIEGAND input section
  if(wg.available()) {
    unsigned long code = wg.getCode();
    int wtype = wg.getWiegandType();
    // Card data

    if(code > 27) {  // chane to > 27 
      long int rawToken = (code & 0xffffff);
      bufn = String(rawToken);
    //  codercv = 1;
      
    }

    // Keypad data
    else {
      if (code != 13) {
        keycount=keycount+1; //Don't count the ENT key as key pressed
        codebuf[keycount] = code; //add code to code buffer only if its not ENT
      };  
      codebuf[0]=keycount;  //size of string is equal to keycount
      makecode(code);
      bufn = code;
  //    if (codedone = true) { comparecode(bufs);};
      if (keycount==12) {

    //    Serial.println("Maximum keys pressed, resetting buffer");
        strcpy(bufa,"");
        keycount=0;
      }
    }
  }
}
