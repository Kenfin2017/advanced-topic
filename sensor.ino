#include <DHT.h>

#define DHTPIN 6 // what digital pin we're connected to

#define DHTTYPE DHT11 // DHT 11

// Initialize DHT sensor.

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  dht.begin(); 
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Wait a few seconds between measurements.

delay(2000);

// Reading temperature or humidity takes about 250 milliseconds!

int h = dht.readHumidity();

// Read temperature as Celsius (the default)

int t = dht.readTemperature();

Serial.print("Humidity: ");
Serial.print(h);
Serial.print("%");

Serial.print("\t"); // for splitting

Serial.print("Temperature: ");
Serial.print(t);
Serial.print(" C");

Serial.print("\n"); // for new line
}
