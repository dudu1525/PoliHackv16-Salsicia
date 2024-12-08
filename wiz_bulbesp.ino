#include <WiFi.h>
#include <WiFiUdp.h>

int buttonPin=21;
int buttonState = HIGH;   
int lastButtonState = HIGH; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 10; 

//variabile care sa modifice dupa user intensitatea si culoarea
int max_int=95;
int max_col=6300;

const int sensorPin = 34;
int sensorValue = 0;
int intens;
int tempr;

int currentHourIndex = 0; 
int hr[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
int nr_hrs=24;

int hour=1;
const char* ssid = "pppaaa";
const char* password = "12345678";
const char* bulbIP = "192.168.208.86"; 
const int bulbPort = 38899;
WiFiUDP udp; //protocolul prin care se transmit informatiile prin wifi catre bec

void setup() {
    pinMode(sensorPin, INPUT);
    pinMode(21, INPUT_PULLUP);
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");
}

bool sendCommand(const char* command) {
    if (!udp.beginPacket(bulbIP, bulbPort)) { //daca nu se trimite comanda, alerteaza
        Serial.println("Failed to start UDP packet.");
        return false;
    }
    udp.write((const uint8_t*)command, strlen(command));//executarea comenzii
    if (!udp.endPacket()) { //daca nu se primeste comanda
        Serial.println("Failed to send UDP packet.");
        return false;
    }
  //  Serial.println("Command sent: ");
   // Serial.println(command);
    return true;
}

void listenForResponse() {
    int packetSize = udp.parsePacket();//verifica daca exista pachet udp
    if (packetSize) {
        char buffer[512];
        int len = udp.read(buffer, 512);
        if (len > 0) {
            buffer[len] = '\0'; 
          //  Serial.println("Received response: ");
           // Serial.println(buffer);
        }
    }
}

void sendCommand(int dimming, int temperature, bool lit) {
    char command[256];
    snprintf(command, sizeof(command),
             "{\"method\": \"setPilot\", \"params\": {\"state\": %s, \"dimming\": %d, \"temp\": %d}}",
             lit ? "true" : "false", dimming, temperature); //se trimite comanda de tip json cu parametrii mentionati
    sendCommand(command);
    listenForResponse();
}

void ensureWiFiConnected() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost. Reconnecting...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Reconnecting to WiFi...");
        }
        Serial.println("Reconnected to WiFi!");
    }
}

void hrcalc()
{
  int reading = digitalRead(buttonPin); //ia informatia butonului
    
  
  if (reading != lastButtonState) {//se schimba debounce time
    lastDebounceTime = millis(); 
  }
  //check debounce time
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && buttonState == HIGH) { //if button is pressed
      currentHourIndex = (currentHourIndex + 1) % nr_hrs; 
      Serial.print("Hour changed to: ");
      Serial.println(hr[currentHourIndex]); 
    }
    buttonState = reading; // update the state of the button
  }

  lastButtonState = reading;
  hour=hr[currentHourIndex];
}


void transform()
{
sensorValue = analogRead(sensorPin);

//   if (sensorValue<500)
//   intens=35;
//   else if (sensorValue<800)
//   intens=40;
//  else if (sensorValue<1100)
//   intens=45;
//   else if (sensorValue<1400)
//   intens=50;
//   else if (sensorValue<1700)
//   intens=55;
//   else if (sensorValue<2000)
//   intens=60;
//   else if (sensorValue<2300)
//   intens=65;
//   else if (sensorValue<2600)
//   intens=70;
//   else if (sensorValue<2900)
//   intens=75;
//   else if (sensorValue<3200)
//   intens=80;
//   else if (sensorValue<3500)
//   intens=85;
//   else if (sensorValue<3800)
//   intens=90;
//   else 
//   intens=95;

double x=(double)sensorValue;
  x=(double)x*100/4095;

  intens=100-x;


  if (hour>=6 && hour<8)
    tempr=4000;
    else if (hour>=8 && hour<12)
    tempr=6000;
    else if (hour>=12 && hour<14)
    tempr=6500;
    else if (hour>=14 && hour<17)
    tempr=5000;
    else if (hour>=17 && hour<20)
    tempr=4000;
    else if (hour>=20 && hour<22)
    tempr=3000;
    else
    tempr=2300;


 if (hour>=6 && hour<8) //edge cases, for example rainy or cloudy days
        if (intens<=15)
          intens=20;
    else if (hour>=8 && hour<12)
          if (intens<=25)
              intens=30;
    else if (hour>=12 && hour<14)
        if (intens<=35)
            intens=40;
    else if (hour>=14 && hour<17)
          if (intens<=25)
              intens=30;

    if (intens>max_int) //check if autmation doesent respect user's input
      intens=max_int;
    if (tempr>max_col)
     tempr=max_col;



}

void loop() {
    ensureWiFiConnected();

      hrcalc();
        transform();
        

  
    //sensorValue = 4095 - sensorValue;
      Serial.print("sensed number from sensor:");
      Serial.println(sensorValue);
    
        sendCommand(intens,tempr,true);
    // if (sensorValue <= 500) {
    //     sendCommand(30, newval, true);
    // } else if (sensorValue <= 1000) {
    //     sendCommand(20, newval, true);
    // } else if (sensorValue <= 2000) {
    //     sendCommand(20, newval, true);
    // } else if (sensorValue <= 3000) {
    //     sendCommand(20, newval, true);
    // } else if (sensorValue <= 4095) {
    //     sendCommand(20, newval, true);
    // }
    delay(200);
}