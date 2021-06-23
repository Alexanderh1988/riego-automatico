#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <Ticker.h>

Ticker Raj_Tick; // nombre de ticker

/* Set these to your desired credentials. */

const char *ssid = "HUAWEI-B310-01B3";
const char *password = "AA34NH1TEG4";

// set pin numbers:
#define D0 16  //  LED Wake
#define D1 5
#define D2 4
#define D3 0   // FLASH
#define D4 2   // TXD1 
#define D5 14
#define D6 12
#define D7 13  // RXD2  Serial.swap(); after Serial.begin();. 
#define D8 15  // TXD2  Serial.swap(); after Serial.begin();. 
#define D9 3   // RXD0
#define D10 1  // TXD0
#define buttonPin  D1     // the number of the pushbutton pin
#define ledPin  D0        // the number of the LED pin

//Web/Server address to read/write from

String payload;

String url = "http://hstech.cl/Post_Server_STODOM/";
int timer = 0;
int tiempotrans = 30;
int limite = 600;
int limitemoist, state1, state2,  limite,  limitenot;

int apausa, amoist, atemp, astate2, astate1;

int temp = 0;
int humid = 0;
int moist = 0;
volatile int WatchDogCount = 0;
int pausa = 8000;

#define DHTPIN D7

// Use DHT11 sensor
#define DHTTYPE DHT11
// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
ESP8266WiFiMulti WiFiMulti;

void ISRWatchDog() {
  WatchDogCount++;
  Serial.print("ISRWatchDog function executed...>>");
  Serial.printf("Counter's value increased to ");
  Serial.println(WatchDogCount);
}

void Automatic() {

  Serial.println("MODO automatico");

  digitalWrite(D1, HIGH);
  delay(tiempotrans);
  moist = analogRead(A0); //LECTURA INICIAL
  delay(tiempotrans);
  digitalWrite(D1, LOW);

  if (moist > limite )
  {
    Serial.print("Motor On, supero limite superior: " + limite);
    int conta = 0;
    for (int motorValue = 0 ; motorValue <= 255; motorValue += 5)
    {
      analogWrite(D2, motorValue);
      delay(50);
    }
    digitalWrite(D2, HIGH);

    while (conta <= 15)
    {
      conta++;
      digitalWrite(D2, HIGH);

      digitalWrite(D1, HIGH);
      delay(tiempotrans);
      moist = analogRead(A0); //take a sample
      delay(tiempotrans);
      digitalWrite(D1, LOW);

      Serial.print("Contador de i :");
      Serial.println(conta);
      Serial.print("el sensor muestra :" + moist);
      delay(300);

      if (moist < limite )
      {
        digitalWrite(D2, LOW);
        Serial.println("dentro del contador  bajo limite inferior " + limite);
        break;
      }
    }
    Serial.println("espera 20 segundos");
    delay(20000);
  }
}

void DarInstrucciones() {

  Serial.println("instrucciones..");

  if (state2 == 1) {
    Automatic();
    state2 = 1;
  }
  else { //Manual

    if (state1 == 1) {
      Serial.println("inst state1=1");
      digitalWrite(D2, HIGH);
      delay(3000);
      digitalWrite(D2, LOW);
      state1 = 0;
    }  if (state1 == 2) {
      Serial.println("inst state1=2");
      digitalWrite(D2, HIGH);
      delay(1500);
      digitalWrite(D2, LOW);
      state1 = 0;
    }  if (state1 == 3) {
      Serial.println("inst state1=3");
      digitalWrite(D2, HIGH);
      delay(500);
      digitalWrite(D2, LOW);
      state1 = 0;
    }

  }
}

void setup() {

  Serial.begin(115200);
  pinMode(A0, INPUT); //medicion humedad de suelo
  //d1 SE ECHO A PERDER
  pinMode(D1, OUTPUT);     //transistor humedad
  // lectura de humedad de suelo
  pinMode(D2, OUTPUT);  //encendido bomba
  // declarado mas arriba D7 lector de um rel
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  //  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //   Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  //Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // Init DHT
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Raj_Tick.attach(3, ISRWatchDog); // 1st argument is in seconds after which the ISRwatchdog() executes


}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {

  apausa = pausa;
  amoist = moist;
  atemp = temp;
  astate2 = state2;
  astate1 = state1;

  Serial.println("pausa de " + pausa);
  delay(pausa);
  digitalWrite(D2, LOW);
  digitalWrite(D1, LOW);

  HTTPClient http;    //Declare object of class HTTPClient

  //  >>--------leer pagina -----------<<
  // wait for WiFi connection

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  //----------------------------------------------------------------------------------------------------------------------------

  DarInstrucciones();

  //----------------------------------------------------------------------------------------------------------------------------
  //Post:

  String postData;
  http.begin(url);              //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header

  if (timer >= 1000) {

    digitalWrite(D1, HIGH);
    delay(tiempotrans);
    moist = analogRead(A0);
    delay(tiempotrans);
    digitalWrite(D1, LOW);
  }
  timer += 450;

  if ( apausa != pausa || amoist != moist || atemp != temp || astate2 != state2 || astate1 != state1)  //se envia post solo si cambiaron parametros
  {
    postData = "info={\"state1\":" + (String)state1 + ",\"state2\":" + (String)state2 + ",\"moist\":" + (String)moist  + ",\"temp\":" + (String)temp +
               ",\"limite\":" + (String)limite +

               ",\"limitenot\":" + (String)limitenot + ",\"pausa\":" + (String)pausa + ",\"WatchDogCount\":" + WatchDogCount +  "}";

    Serial.println("post es: " + postData);
    int httpCode = http.POST(postData);    //Send the request
    http.end();  //Close connection
    Serial.println("Se mando post");

  }
  Serial.println("timer is " + timer);
  delay(1000);

  // >>------ ahora deserealizar Json:  ---<<

  int stgleng = payload.length() + 1;
  Serial.println("payload es " + payload);

  StaticJsonDocument<300> doc;       //cambiar al final
  char json[stgleng];

  payload.toCharArray(json, stgleng);
  deserializeJson(doc, payload);

  String state1stg = doc["state1"];
  String state2stg = doc["state2"];
  String pausa = doc["pausa"];
  String limite = doc["limite"];
  String limitenot = doc["limitenot"];
  String WatchDogCount = doc["WatchDogCount"];

  state1 = state1stg.toInt();
  state2 = state2stg.toInt();
  pausa = pausa.toInt();
  WatchDogCount = WatchDogCount.toInt();
  limite = limite.toInt();
  limitenot = limitenot.toInt();

  // Print values.
  Serial.println(state1);
  Serial.println(state2);
  Serial.println(pausa);
  Serial.println(limitemoist);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }


}
