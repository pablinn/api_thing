
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#define DHTTYPE DHT22
#define DHTPIN 2



ADC_MODE(ADC_VCC);

//**** variables a modificar
const char* ssid = "accespoint";// nombre ssid
const char* password = "pass"; // password
WiFiClient client;
// ThingSpeak Settings
const int channelID = id_int;//id de tu canal es un numero entero
String writeAPIKey = "api_write"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com";
const int postingInterval = 20 * 1000; // post data every 20 seconds
// *****variables a modificar

//Datos agregados sensores I/O
Adafruit_BMP280 bmp; // I2C
DHT dht(DHTPIN, DHTTYPE, 30); // 11 works fine for ESP8266
 
float HR,T1,T2,P,alt,PR,ST,IZ;  
unsigned long previousMillis = 0; // will store last temp was read
const long interval = 2500; // interval at which to read sensor
    
void gettemperature() {
    unsigned long currentMillis = millis();
    
    if(currentMillis - previousMillis >= interval)
    {
     previousMillis = currentMillis;
     HR = dht.readHumidity(); // Read humidity (percent)
     T1 = dht.readTemperature(false); // Read temperature as Fahrenheit
     if (isnan(HR) || isnan(T1))
     {
      Serial.println("Failed to read from DHT sensor!");
      return;
     }
    ST = dht.computeHeatIndex(T1, HR, false);
    PR =dht.DewPoint();
    }
}
    
float pendiente(float y1,float x1,float y2, float x2){
float m=0;

 m=(y2-y1)/(x2-x1);
return m;
}

//la constante la obtengo despejando 
float constante(float m,float y1,float x1){
  float c=0;

  c=y1-(m*x1);
  return c;
} 
    
float pres_baj(float y1,float x1,float y2, float x2,float pres_atm){
float  prom_met;
float m_aux=pendiente(y1,x1,y2,x2);
float k_aux =constante(m_aux,y1,x1);
    
    prom_met = (m_aux * pres_atm)+k_aux;
    return prom_met;
  
}
void bmp280(){
  
    P =(bmp.readPressure()/100); //HPA o mBar
    T2 = bmp.readTemperature();
    alt = bmp.readAltitude(1013.25); // this should be adjusted to your local forcase
    IZ = pres_baj(1,1033,10,960,P);    
 }
     
void setup() {
  Serial.begin(115200);
  if (!bmp.begin()) {  
    Serial.println(F("Chequea el cableado del bmp280 dit 76H"));
    while (1);    
    }     
    
    dht.begin(); // initialize temperature sensor
    delay(10);
    
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  if (client.connect(server, 80)) {
    gettemperature();
    bmp280();
    // Measure Signal Strength (RSSI) of Wi-Fi connection
//    long rssi = WiFi.RSSI();

    // Construct API request body
    String body = "field1="; 
           body += String(T1);    
           
           body += "&field2=";
           body += String(HR);    
           
           body += "&field3=";
           body += String(P);

           body += "&field4=";
           body += String(PR);
           
           body += "&field5=";
           body += String(ST);
           
           body += "&field6=";
           body += String(IZ);
           
   // Serial.print("RSSI: ");
   // Serial.println(rssi); 
    //client.print("GET https://api.thingspeak.com/update?api_key=H3C29BIN357XX7Q4&field1="+String(T1));
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");

  }
  client.stop();

  // wait and then post again
     delay(postingInterval);
         
     
}

  


