#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

// WiFi network
const char* ssid = "Peter";                                         
const char* password = "quesadilla12";    

//Your Domain name with URL path or IP address with path
const char* getInfoActuador = "http://ginger-backend.herokuapp.com/api/actuador/1";
const char* publicarRegistro = "http://ginger-backend.herokuapp.com/api/actuadorregistro/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can't be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
#define bomba D6

unsigned long timerDelay = 5000;

String actuadorInfo;
float sensorReadingsArr[3];
char buffer[500];


void setup() {
  Serial.begin(115200);
  pinMode(bomba, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    
    if(WiFi.status()== WL_CONNECTED){
      
      actuadorInfo = httpGETRequest(getInfoActuador);              
      JSONVar actuadorInfoJSON = JSON.parse(actuadorInfo);
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(actuadorInfoJSON) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      // myObject.keys() can be used to get an array of all the keys in the object
      JSONVar keys = actuadorInfoJSON.keys();
    
      //JSONVar estado = actuadorInfoJSON[keys[6]];
      //JSONVar valor = actuadorInfoJSON[keys[7]];
      
      String estado =  JSON.stringify(actuadorInfoJSON[keys[6]]);
      int valor = atoi(actuadorInfoJSON["valor"]);
      
      if(estado.length() == 11)
      {
        Serial.println("estado pendiente");
        if(valor == 100){
          Serial.println("valor 100");
          digitalWrite(bomba,HIGH);
          delay(1000);
          digitalWrite(bomba,LOW);
          delay(10000);
          digitalWrite(bomba,HIGH);
          delay(1000);
          digitalWrite(bomba,LOW);
        }
        httpPutRequest(getInfoActuador, "Aplicado");
        httpPostRequest(publicarRegistro);
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void httpPostRequest(const char* serverName){
  actuadorInfo = httpGETRequest(getInfoActuador);              
  JSONVar actuadorInfoJSON = JSON.parse(actuadorInfo);
  sprintf(buffer, "{\"aplicado\":\"true\",\"idActuador\":\"1\",\"timeStamp\":\"w\",\"valor\":\"%d\"}", atoi(actuadorInfoJSON["valor"]));
  Serial.println(buffer);
 
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  
  http.addHeader("Content-Type", "application/json");
  
  int code = http.POST(buffer);
  
  http.end();
  Serial.println(code);
}

void httpPutRequest(const char* serverName, String estado){
  actuadorInfo = httpGETRequest(getInfoActuador);              
  JSONVar actuadorInfoJSON = JSON.parse(actuadorInfo);
  //sprintf(buffer, "{\"nombre\":\"Bomba de plantas\",\"descripcion\":\"bomba para regar plantas\",\"tipoActuador\":\"Bomba de 12V\",\"ubicacion\":\"Jardin\",\"unidades\":\"PWM\",\"estado\":\"Aplicado\",\"valor\":\"%d\"}",atoi(actuadorInfoJSON["valor"]));
  sprintf(buffer, "{\"nombre\":\"Bomba de plantas\",\"descripcion\":\"bomba para regar plantas\",\"tipoActuador\":\"Bomba de 12V\",\"ubicacion\":%s,\"unidades\":%s,\"estado\":\"%s\",\"valor\":\"%d\"}",
                    JSON.stringify(actuadorInfoJSON["ubicacion"]),
                    JSON.stringify(actuadorInfoJSON["unidades"]),
                    estado,
                    atoi(actuadorInfoJSON["valor"]));

  Serial.println(buffer);
      
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  
  int code = http.PUT(buffer);
  
  http.end();
  Serial.println(code);
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0)
    payload = http.getString();
  
  // Free resources
  http.end();

  return payload;
}
