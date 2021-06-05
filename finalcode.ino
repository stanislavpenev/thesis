#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "ThingSpeak.h" 

#define SEALEVELPRESSURE_HPA (1013.25)
#define HUMIDITY_SENSOR_PIN     34
#define HUMIDITY_LOWER_BOUND    2800
#define HUMIDITY_UPPER_BOUND    1700
#define PUMPPIN    32
Adafruit_BME680 bme; 

char ssid[] = "iPhone";   // your network SSID (name) 
char pass[] = "djanamka";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = 1324064;
const char * myWriteAPIKey = "IJ5V032JA6T9B6LV";

int number1 = 0;
int number2 = 0;
int number3 = 0;
int number4 = 0;


String myStatus = "";

unsigned long long int t1 = 0;
int initial_send = true;

hw_timer_t* timer = NULL;

void checkHumidity(void) {
  int humidity = analogRead(HUMIDITY_SENSOR_PIN);
  Serial.println("Check humidity");
  Serial.println(humidity);

  if (humidity > HUMIDITY_LOWER_BOUND) {
    Serial.println("Starting pump");
    digitalWrite(PUMPPIN,HIGH);
  }

  if (humidity < HUMIDITY_UPPER_BOUND) {
    Serial.println("Stopping pump");
    digitalWrite(PUMPPIN,LOW);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("BME680 test"));
  pinMode(PUMPPIN, OUTPUT);
 
  if (!bme.begin(0x77)) 
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

//  timer = timerBegin(0, 160, true);
//  timerAttachInterrupt(timer, &checkHumidity, true);
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  
  if ((millis() - t1 >= 600000) || initial_send)
  {
    t1 = millis();
    initial_send = 0;
    if (! bme.performReading()) 
    {
      Serial.println("Failed to perform reading 🙁");
      return;
    }
    Serial.print("Temperature = ");
    Serial.print(bme.temperature);
    Serial.println(" *C");
   
    Serial.print("Pressure = ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");
   
    Serial.print("Humidity = ");
    Serial.print(bme.humidity);
    Serial.println(" %");
   
    Serial.print("Gas = ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(" KOhms");
  
    number1 = bme.temperature;
    number2 =  bme.pressure/ 100.0;
    number3 = bme.humidity;
    number4 = bme.gas_resistance/ 1000.0;
    // set the fields with the values
    ThingSpeak.setField(1, number1);
    ThingSpeak.setField(2, number2);
    ThingSpeak.setField(3, number3);
    ThingSpeak.setField(4, number4 );
    Serial.print(number1);
    
    // set the status
    ThingSpeak.setStatus(myStatus);
    
    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
  }

  checkHumidity();
  delay(1000);
  
}
