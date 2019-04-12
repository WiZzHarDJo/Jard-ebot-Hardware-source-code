/*
  Soil sensor managment of ECE's garden

  This code is reading data from three sensor, one for air humidity one for temperature and one for soil moisture.
  It then compute the heat index and post all the retrieved data to a web service that mannage the sql database.
  
  The circuit:
  -Lula nodemcu esp8266
  -DHT22 sensor
  -Capacitive soil moisture sensor

  created 2019
  by Nicolas Chollet
  last modified 11 April 2019
  by Rémi Motsch

  This code is in the public domain.

  Visit our website:
  http://www.jardin-electronique.com
  
*/


//Libraries 
#include <DHT.h>
#include <DHT_U.h>
#include <string>
#include <ESP8266WiFi.h>

//SensorID !!!!!----Change the ID to match with the sensor (0 is test sensor)----!!!!

  int id_sensor=0;

//Wifi network information

  const char* ssid = "<InsertSSID>";
  const char* password = "<InsertPassWord>";
  
  const char* host = "83.204.149.28";


//String to mannage HTTP data message
  String data;
  String data1="ID", data2="&AIR_HUMIDITY=",data3="&SOIL_HUMIDITY=", data4="&TEMPERATURE=", data5="&HEAT_INDEX=";
  
//Time managing variable
  int timeSinceLastRead = 0;

//Soil moisture sensor calibrations variables
  float dry_value = 750; //value mesured in the open-air
  float wet_value = 367; //value mesured in the water

//Sensors pins definition
  #define DHTPIN 4    // what digital pin the DHT22 is conected to
  #define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
  const int SOIL_HUMIDITY_SENSOR = A0;
  
//Creation of the DHT object
  DHT dht(DHTPIN, DHTTYPE);

  void setup()
  {
  //Serial initialization
    Serial.begin(9600);
    Serial.setTimeout(2000);
  // Wait for serial to initialize.
    while(!Serial) { } 
    Serial.println("Device Started");
    Serial.println("-------------------------------------");
  
  //Connecting to Wi-fi network
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED)
      {
      delay(500);
      Serial.print(".");
      }
    Serial.println(" connected");
    
  }


  void loop()
  {
    //Read the air humidity
    float air_humidity = dht.readHumidity();
    
    // Read temperature as Celsius 
    float temperature = dht.readTemperature();
    
   // Check if any reads failed and exit early (to try again).
    if (isnan(air_humidity) || isnan(temperature)) 
    {
      //Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    //Read the soil humidity
    float soil_humidity = analogRead(SOIL_HUMIDITY_SENSOR);
    //formula using calibrations variables of the sensor to have a percentage instead of a simple value 
    soil_humidity = 100 - (soil_humidity - wet_value)/(dry_value - wet_value)*100;
    
    //Compute the heat index
    float heat_index = dht.computeHeatIndex(temperature, air_humidity, false);
    
    //Putting together in a bug string to send the data
    data = data1+id_sensor+data2+air_humidity+data3+soil_humidity+data4+temperature+data5+heat_index;
    Serial.println(data);

    //Creating a WifiClient Object to send request 
    WiFiClient client;
    
    Serial.printf("\n[Connecting to %s ... ", host);

    //Sending the HTTP Request
    if (client.connect(host, 80))
    {
        client.println("POST /insert.php HTTP/1.1");
        client.println("Host: 83.204.149.28");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.print(data);
        Serial.println("\n[Disconnected]");
      
      //Reading what the page sent to debug potential HTTP responses
      while (client.connected() || client.available())
      {
        if (client.available())
        {
          String line = client.readStringUntil('\n');
          Serial.println(line);
        }
            
      }
    }
    //If connection fails
    else
    {
      Serial.println("connection failed!]");
      client.stop();
    }

    //Mesures every minute : 900s - 5s of process
    ESP.deepSleep(55e6);
    
  }
