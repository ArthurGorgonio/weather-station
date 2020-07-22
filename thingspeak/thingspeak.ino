/*
 * ============================================================================
 *
 *       Filename:  thingspeak.ino
 *
 *    Description:  This file contains a code to read the temperature and the
 *        humidity from the environment using a DHT sensor and an ESP32. Also,
 *        it post these values in the Thing Speak plataform.
 *
 *        Version:  1.0
 *        Created:  2020 Jul 15 15:50 -0300
 *    Last Edited:  2020 Jul 15 15:50 -0300
 *       Revision:  none
 *
 *         Author:  Arthur Gorgônio, gorgonioarthur@gmail.com
 *   Organization:
 *
 * ============================================================================
 */
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "secret.h"


//defines
#define DHTTYPE DHT22 // DHT 11 (AM2302)
#define DHTPIN 4 // GPIO ou porta D1 do NodeMCU

// Thing Speak Informations
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned int dataTemp = 1; // Temperature Field in Thing Speak
unsigned int dataHumi = 2; // Humidity Field in Thing Speak
unsigned int postInterval = 300L * 1000L;  // Interval to post a new data in
                                           // ThingSpeak (actual is 5 minutes)

// Global variables
float temp, humi;
unsigned long lastConnectionTime = 0;
long lastUpdate = 0;  // Last update in ms
unsigned int postStatus = 0; // Status of the post data
WiFiClient client;
DHT dht(DHTPIN, DHTTYPE); // Sensor that reads the temperature and humidity

//prototypes
void connectWiFi(bool verbose);
int write2TSData(unsigned int TSFieldTemp, String fieldTemp,
                 unsigned int TSFieldHumi, String fieldHumi);

/*
 * ===  FUNCTION  =============================================================
 *           Name:  write2TSData
 *    Description:  Send data to Thing Speak, into specifics fields.
 *
 *         Params:
 *         TSFieldTemp => The index of the temperature field in Thing Speak.
 *         fieldTemp   => The value of the temperature read by DHT sensor.
 *         TSFieldHumi => The index of the humidity field in Thing Speak.
 *         fieldHumi   => The value of the humidity read by DHT sensor.
 *
 *         Return:  int (HTML code pages)
 *             200 => The Data was posted
 *             Any => The Data was not posted
 * ============================================================================
*/
int write2TSData(unsigned int TSFieldTemp, String fieldTemp,
                 unsigned int TSFieldHumi, String fieldHumi) {
  ThingSpeak.setField(TSFieldTemp, fieldTemp);
  ThingSpeak.setField(TSFieldHumi, fieldHumi);

  int writeSuccess = ThingSpeak.writeFields(channelID, writeAPIKey);
  return writeSuccess;
}

/*
 * ===  FUNCTION  =============================================================
 *           Name:  connectWiFi
 *    Description:  Perform the connetion step between the ESP32 and the router
 *
 *         Params:
 *       verbose => Show in the Serial console the ip if the devise was
 *                connected in Wi-Fi. The default value is false.
 *
 *         Return:  void
 * ============================================================================
*/
void connectWiFi(bool verbose=false) {
  client.stop();
  delay(1000);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  if (verbose) {
    Serial.println("");
    Serial.println("Wi-Fi successful connected!");
    Serial.println("ESP IP address: ");
    Serial.println(WiFi.localIP());
  }
  delay(1000);
}

/*
 * ===  FUNCTION  =============================================================
 *           Name:  setup
 *    Description:  The first function that the ESP will execute.
 *
 *         Params:
 *
 *         Return:  void
 * ============================================================================
*/
void setup() {
  dht.begin(); //Inicia o sensor
  Serial.begin(115200);
  lastConnectionTime = 0;
  connectWiFi();
  ThingSpeak.begin(client);
}

/*
 * ===  FUNCTION  =============================================================
 *           Name:  loop
 *
 *    Description:  The ESP will execute this function everytime
 *
 *         Params:
 *
 *         Return:  void
 * ============================================================================
*/
void loop() {
  if (!client.connected() && (millis() - lastUpdate) >= postInterval) {
    lastUpdate = millis();
    temp = dht.readTemperature();
    humi = dht.readHumidity();
    Serial.println("Temp = " + String(temp) + "ºC\t" +
                   "Humi = " + String(humi) + "%");
    postStatus = write2TSData(dataTemp, String(temp),
                              dataHumi, String(humi));
    if (postStatus == 200) {
      Serial.println("The data was sent");
    } else {
      Serial.println("Cannot send the Data, the Error code is " + postStatus);
    }
  } else {
    Serial.println("LastUpdate = " + String(lastUpdate) +
                   " Millis = " + String(millis()) +
                   " Post Interval = " + String(postInterval));
  }
  delay(1000);
}
