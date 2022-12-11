/******************************
*          POOR MAN's         *
*   BUT MOST OVERENGINEERED   * 
*        ADVENT WREATH        *
*          BY RAYDIY          *
*******************************/

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h> 
#elif defined(ESP32)
#include <WiFi.h> 
#endif

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "time.h"

/*** CHANGE YOUR SETINGS HERE *********************/

const char ssid[] = "YOUR_SSID";          // your network SSID (name)
const char pass[] = "YOUR_PASS";          // your network password

#define DATA_PIN 15                       // The data pin the led strip is connected to
#define GMT_TIMEZONE 1                    // Your GMT timezone offset, i.e. Germany would be 1, New York EST would be -3

/*** END OF YOUR SETTINGS *************************/



/*************************************************/
/*** FUNCTION DECLATIONS *************************/

bool getLocalTimeESP(struct tm * info, uint32_t ms);
byte calculateAdvent();
void connectWifi();
time_t createTimestampFromDate(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);
void disconnectWifi();
void connectNTP();
void candleOn(byte candle);
void candleOff(byte candle);
void candlesOff();
void candlesUpdates();
void updateNTPTime();

/*************************************************/
/*************************************************/

#define NUM_LEDS 4                              // Number of LEDs
#define FPS 10                                  // the frames per second in update loop
#define PI 3.1415926535897932384626433832795    // Pi
byte advent = 0;                                // holds the current number of advent
unsigned long lastTimeUpdate = 0;               // timestamp when the last time has been updated via NTP server
byte firstRun = true;                           // if this is the first run to immediately update time

// initialise Neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(DATA_PIN, DATA_PIN, NEO_GRB + NEO_KHZ800);


/* SETUP ****************************************/

void setup() {
  Serial.begin(115200);
  delay(500);
  while (!Serial) ; // Needed for Leonardo only
  delay(500);

  Serial.println("Init LEDs()");

  // Init LEDs
  strip.begin();
  strip.setBrightness(25);
  strip.show(); // Initialize all pixels to 'off'

  Serial.println("setup() finished");
}

/* LOOP *****************************************/
void loop() {
  updateNTPTime();
  candlesUpdates();
  strip.show();
  delay(1000/FPS);
}



/*************************************************/
/*** FUNCTION DEFINITIONS*************************/


/*************************************************
 * updateNTPTime()
 */
void updateNTPTime()
{
  if ((millis() - lastTimeUpdate >= 5*60*1000UL) || (firstRun == true)) 
  {
    Serial.println("updateNTPTime()");

    lastTimeUpdate = millis();
    firstRun = false;

    // connect WiFi
    connectWifi();
    
    // Init and get the time
    connectNTP();

    // Calculate the current advent number 
    advent = calculateAdvent(); 

    // Disconnect Wifi
    disconnectWifi();
  }
}



/*************************************************
 * connectWifi()
 */
void connectWifi()
{
  Serial.println("connectWifi()");

  // Connect to Wi-Fi
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully.");
}


/*************************************************
 * disconnectWifi()
 */
void disconnectWifi()
{
  Serial.println("disconnectWifi()");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


/*************************************************
 * connectNTP()
 */
void connectNTP()
{
  Serial.println("connectNTP()");

  // the ntp server we want to connect
  const char* ntpServer = "pool.ntp.org";

  // the timezone offset to GMT in seconds
  const long  gmtOffset_sec = 3600 * GMT_TIMEZONE;

  // additional daylight saving time in seconds
  // since christmas is in winter, no dayligt savings time should be added
  // winter time is the "normal time", summer time has an additional hour in some countries
  const int   daylightOffset_sec = 0;

  // config connection
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}


/*************************************************
 * calculateAdvent()
 * return 0 to 4 for the current number of advent
 */
byte calculateAdvent()
{
  Serial.println("calculateAdvent()");

  // get current time from NTP server
  struct tm timeinfo;
  if(!getLocalTimeESP(&timeinfo, 5000)){
    Serial.println("Failed to obtain time");
    return 0;
  }

  // get the current year
  int current_year = timeinfo.tm_year + 1900;
  Serial.print( "current_year: " );
  Serial.println( current_year );

  // get the current time as timestamp = now
  // get now as timestamp
  time_t timestamp_now = mktime(&timeinfo);
  //time_t timestamp_now = createTimestampFromDate(current_year, 12, 18, 0, 0, 0); // enter test dates here
  Serial.print( "timestamp_now: " );
  Serial.println( timestamp_now );

  // get the timestamp of 25th of the current year
  
  time_t timestamp_25th = createTimestampFromDate(current_year, 12, 25, 0, 0, 0);
  struct tm tm_25th;
  localtime_r(&timestamp_25th, &tm_25th);
  Serial.print( "tm_25th.tm_weekday: " );
  Serial.println( tm_25th.tm_wday );
  
  // check how many days from 25th to the next sunday before 
  // the 4th advent is always the sunday before 25th
  byte diffDaysTo4thAdvent = tm_25th.tm_wday;
  if (tm_25th.tm_wday == 0 ) { diffDaysTo4thAdvent = 7; }
  //Serial.print( "diffDaysTo4thAdvent: " );
  //Serial.println( diffDaysTo4thAdvent );

  // get the 4th advent as timestamp
  time_t timestamp_4thAdvent = timestamp_25th - diffDaysTo4thAdvent * 86400;
  //Serial.print( "timestamp_4thAdvent: " );
  //Serial.println( timestamp_4thAdvent );

  struct tm tm_4thAdvent;
  localtime_r(&timestamp_4thAdvent, &tm_4thAdvent);
  //Serial.print( "tm_4thAdvent.tm_mon: " );
  //Serial.println( tm_4thAdvent.tm_mon );
  //Serial.print( "tm_4thAdvent.tm_mday: " );
  //Serial.println( tm_4thAdvent.tm_mday );
  //Serial.print( "tm_4thAdvent.tm_weekday: " );
  //Serial.println( tm_4thAdvent.tm_wday );

  // get the 3rd advent (seven days befor 4th advent)
  time_t timestamp_3rdAdvent = timestamp_4thAdvent - 7 * 86400;

  // get the 2nd advent (seven days befor 3rd advent)
  time_t timestamp_2ndAdvent = timestamp_3rdAdvent - 7 * 86400;

  // get the 1st advent (seven days befor 2nd advent)
  time_t timestamp_1stAdvent = timestamp_2ndAdvent - 7 * 86400;

  // if now >= 4th advent > light 4 LEDs; return;
  if ( timestamp_now >= timestamp_4thAdvent)
  {
    Serial.println( "4th Advent!" );
    candleOn(3);
    candleOn(2);
    candleOn(1);
    candleOn(0);
    return 4;
  }

  // if now >= 3rd advent > light 3 LEDs; return;
  if ( timestamp_now >= timestamp_3rdAdvent)
  {
    Serial.println( "3rd Advent!" );
    candleOn(2);
    candleOn(1);
    candleOn(0);
    return 3;
  }

  // if now >= 2nd advent > light 2 LEDs; return;
  if ( timestamp_now >= timestamp_2ndAdvent)
  {
    Serial.println( "2nd Advent!" );
    candleOn(1);
    candleOn(0);
    return 2;
  }

  // if now >= 1st advent > light 1 LEDs; return;
  if ( timestamp_now >= timestamp_1stAdvent)
  {
    Serial.println( "1st Advent!" );
    candleOn(0);
    return 1;
  }

  Serial.println( "No Advent :(" );
  candlesOff();
  return 0;
}


/*************************************************
 * createTimestampFromDate()
 */
time_t createTimestampFromDate(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  struct tm tmSet;
  tmSet.tm_year = YYYY - 1900;
  tmSet.tm_mon = MM - 1;
  tmSet.tm_mday = DD;
  tmSet.tm_hour = hh;
  tmSet.tm_min = mm;
  tmSet.tm_sec = ss;
  return mktime(&tmSet);         //convert to time_t
}


/*************************************************
 * candleOn()
 */
void candleOn(byte candle)
{
  strip.setPixelColor(candle, strip.Color(255, 0, 0));
  strip.show();
}


/*************************************************
 * candleOff()
 */
void candleOff(byte candle)
{
  strip.setPixelColor(candle, strip.Color(0, 0, 0));
}


/*************************************************
 * candlesOff()
 */
void candlesOff()
{
  for (size_t i = 0; i < NUM_LEDS; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }

}


/*************************************************
 * candlesUpdates()
 */
void candlesUpdates()
{
  if (advent == 0){ return; }

  for (size_t i = 1; i <= advent; i++)
  {
    uint16_t h = 8000;
    uint8_t s = 255;
    uint8_t v = random(180,255);
    strip.setPixelColor(i-1, strip.ColorHSV(h,s,v));
  }
}



/*************************************************
 * getLocalTimeESP()
 * This is teh exact copy of the function getLocalTime() from time.h library
 * Since this function is only a convenince function for ESP32 I copied here
 * to work with ESP8266 as well
 * ms default is 5000
 */
bool getLocalTimeESP(struct tm * info, uint32_t ms)
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}