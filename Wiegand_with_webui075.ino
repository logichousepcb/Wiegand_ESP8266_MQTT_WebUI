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



#define RELAY_TIMER     3  //seconds the door latch activates


WIEGAND wg;


char bufa[20];
String bufs;
String bufactive;
unsigned char codebuf[26];
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
  
// PubSubClient client(DNSClient);
ESP8266WebServer server;
WebConfig conf;

boolean initWiFi() {
    boolean connected = false;
    WiFi.mode(WIFI_STA);
    Serial.print("SSID-");
    Serial.println (conf.values[0]);

    if (conf.values[0] != "") {
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
 //     PubSubClient (client(conf.values[0].c_str(),conf.values[1].c_str());
 //       Serial.println (conf.values[0].c_str());
      uint8_t cnt = 0;
      while ((WiFi.status() != WL_CONNECTED) && (cnt<20)){
      // fast flashing connecting to network  
        digitalWrite(GREEN_LED_PIN, LOW);
        delay (5);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay (5);
      //  Serial.print(".");
        cnt++;
      }
     // Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("UIIP-");
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
// setup config file and wifi stuff
  Serial.println(params);
  conf.setDescription(params);
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
//  beep ();
//  digitalWrite(BEEP_PIN, HIGH);
// only connect wifi is web actice 
 if (webactive) { 
  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("WEBUI-ON");
    pulseled (5);
  }
  
//  client.setBufferSize(512); // set buffer size for config doc                        
//  client.setServer(USER_MQTT_SERVER, USER_MQTT_PORT);
//  if (!client.connected()) 
//  {
//    reconnect();
//  }
  
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
    delay (RELAY_TIMER*200);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay (RELAY_TIMER*200);
    } 
}

void triggerrelay() {
  //  digitalWrite(RELAY_PIN, HIGH);
    pulseled (4);
    /*digitalWrite(GREEN_LED_PIN, LOW);
    delay (RELAY_TIMER*200);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay (RELAY_TIMER*200);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay (RELAY_TIMER*200);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay (RELAY_TIMER*200);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay (RELAY_TIMER*400);
    digitalWrite(GREEN_LED_PIN, HIGH);
    */
    digitalWrite(RELAY_PIN, LOW);
 
}


void makecode(long addbuf) {
    
    switch (addbuf) {
      case 13:
        strcat(bufa,"\0");
        keycount=0;
        bufs = String(bufa);  // convert char array entered to a string
        comparecode();
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

void comparecode () {
  int match;
  String activelist;
  char activeusr;

  match=0; // starts with no match in case there are no bytes
//  Serial.print (" Is active test =");
//  Serial.print (conf.getValue("activeusers"));
//  Serial.print ("=");
  activelist = conf.getValue("activeusers");
  activeusr = activelist.charAt(1);  
//  Serial.println (activeusr);
  Serial.print ("CODE-");
  Serial.print (bufs);
  Serial.print ("-");
 // if (bufs == "99992") {Serial.println ("feeding value",conf.setValue("user5","4602"));conf.writeConfig();};  
  if (bufs == "99991") {Serial.println ("99-C");webactive = true;webUI();};
  if (bufs == "99990") {Serial.println ("99-C");pulseled (2); webactive = false;};
  if (bufs == conf.getValue("admn")) {Serial.print ("00-");
      match = 1;
      strcpy(bufa,"");
      bufs = "00000";
  }
  if (bufs == conf.getValue("usr1")) {
      Serial.print ("01-");
      activeusr = activelist.charAt(0);  
      if (activeusr == '1') {
         
         match = 1; }; 
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr2")) {Serial.print ("02-");
      activeusr = activelist.charAt(1);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr3")) {Serial.print ("03-");
      activeusr = activelist.charAt(2);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr4")) {Serial.print ("04-");
      activeusr = activelist.charAt(3);  
      if (activeusr == '1') {
         match = 1;};
    
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr5")) {Serial.print ("05-");
      activeusr = activelist.charAt(4);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr6")) {Serial.print ("06-");
      activeusr = activelist.charAt(5);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr7")) {Serial.print ("07-");
      activeusr = activelist.charAt(6);  
      if (activeusr == '1') {
         match = 1;};
   
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr8")) {Serial.print ("08-");
      activeusr = activelist.charAt(7);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
   if (bufs == conf.getValue("usr9")) {Serial.print ("09-");
      activeusr = activelist.charAt(8);  
      if (activeusr == '1') {
         match = 1;};
 
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (bufs == conf.getValue("usr10")) {Serial.print ("10-");
      activeusr = activelist.charAt(9);  
      if (activeusr == '1') {
         match = 1;};
  
      strcpy(bufa,"");
      bufs = "00000";
  } 
  if (match==1) {
      Serial.println ("G");
      triggerrelay();
     
      }
      else Serial.println ("99-B") ;  // here is where to add user 99 denied code
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
    Serial.println(incomingByte, HEX);
  }
  // WIEGAND input section
  if(wg.available()) {
    unsigned long code = wg.getCode();
    int wtype = wg.getWiegandType();
    // Card data

    if(code > 27) {  // chane to > 27 
      long int rawToken = (code & 0xffffff);
      int facilityCode = (rawToken & 0x1F0000) >> 16;
      int cardCode = (rawToken & 0x3FFF) >> 16;
      int cardCodeb = (rawToken & 0xffff) >> 8;
      code_B = code / 100000;
      code_A = code - (code_B * 100000);
      String myStr;
      bufs = String(rawToken);
  //    Serial.println(myStr);
      comparecode();
    }

    // Keypad data
    else {
      if (code != 13) {
        keycount=keycount+1; //Don't count the ENT key as key pressed
        codebuf[keycount] = code; //add code to code buffer only if its not ENT
      };  
      codebuf[0]=keycount;  //size of string is equal to keycount
      makecode(code);
      if (keycount==12) {

    //    Serial.println("Maximum keys pressed, resetting buffer");
        strcpy(bufa,"");
        keycount=0;
      }
    }
  }
}
