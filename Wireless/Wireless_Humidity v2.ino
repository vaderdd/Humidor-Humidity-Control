#include <Time.h>
#include <TimeLib.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>    //DHT Sensor Library
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define DHTPIN D3      // Digital pin connected to the DHT sensor
#define DHTPIN2 D4     // Digital pin connected to the DHT sensor
#define RELAYPIN1 D6   // Digital Pin connected to Hum Fan
#define RELAYPIN2 D7   // Digital Pin connected to Circ Fan

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4); // (Hex address, colums, rows)

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password

const long utcOffsetInSeconds = -3000;
unsigned long epochTime = 0;
String f, currentDate, z;
unsigned long restarttime = 0;
unsigned long bigtime = 0;

WiFiClient client;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

//Thingspeak Keys to update channels and post to twitter
unsigned long myChannelNumber = xxxxxxxx;
const char * myWriteAPIKey = WRITE API KEY HERE";
//char thingSpeakAddress[] = "api.thingspeak.com";
//String thingtweetAPIKey = SECRET_TWEET_APIKEY;
String Twitter = "HumidorStatus";
unsigned long lastConnectionTime = 0;
int failedCounter = 0;
int x, t;
String myStatus, myStatus1, myStatus2, myStatusT, myStatusH;
String h;

//Inputs

int HumRelayState = LOW;
int CircRelayState = LOW;
unsigned long PrevMS = 0;  //Store last time clock was updated
unsigned long CircMS = millis() - 1800000;  //Store circulation fan time on
unsigned long ThingMS = 0; //Store last update to ThingSpeak
unsigned long TweetMS = 0;
long ThingNow = 300000;    //Update Thingspeak every 5 minutes
long TweetNow = 21600000;   //Tweet every 6 hours
long CircFan = 300000;     //Turn circulating fan on for 5 minutes
long CircFanON = 1800000;  //Turn circulating fan on every half hour

DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
long ReadDHT = 3000;       //3 second read delay in ms
const float HumFanON = 68;       //Humidity level to turn ON humidifying fans
const float HumFanOFF = 72;      //Humidity level to turn OFF humidifying fans
float hum = 0;           // initializing Humidity and Temp
float temp = 0;
float hum2 = 0;
float temp2 = 0;
float avghum = 0;
float avgtemp = 0;
float humcal = -7;
unsigned long CurMS = 0;


byte one[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b01000
};

byte two[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b10100,
  0b00100,
  0b01000,
  0b11100
};

byte avg[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b10100,
  0b11100,
  0b10100
};

byte deg[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};


void setup() {
  pinMode(RELAYPIN1, OUTPUT);
  pinMode(RELAYPIN2, OUTPUT);
  digitalWrite(RELAYPIN1, HumRelayState);
  digitalWrite(RELAYPIN2, CircRelayState);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  
  dht.begin();
  dht2.begin();

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, avg);
  lcd.createChar(1, one);
  lcd.createChar(2, two);
  lcd.createChar(3, deg);

  lcd.clear();

  lcd.println("Connecting to WiFI");
  lcd.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
  lcd.println("WiFi Connected");
  lcd.print("IP: ");
  lcd.println(WiFi.localIP());
  delay(1000);

  timeClient.begin();
   
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T");
  lcd.setCursor(1, 0);
  lcd.write(1);
  lcd.setCursor(2, 0);
  lcd.print(":");
  lcd.setCursor(3, 0);
  lcd.print(temp);
  lcd.setCursor(8, 0);
  lcd.write(3);
  lcd.setCursor(10, 0);
  lcd.print("H");
  lcd.setCursor(11, 0);
  lcd.write(1);
  lcd.setCursor(12, 0);
  lcd.print(":");
  lcd.setCursor(13, 0);
  lcd.print(hum);
  lcd.setCursor(18, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("T");
  lcd.setCursor(1, 1);
  lcd.write(2);
  lcd.setCursor(2, 1);
  lcd.print(":");
  lcd.setCursor(3, 1);
  lcd.print(temp2);
  lcd.setCursor(8, 1);
  lcd.write(3);
  lcd.setCursor(10, 1);
  lcd.print("H");
  lcd.setCursor(11, 1);
  lcd.write(2);
  lcd.setCursor(12, 1);
  lcd.print(":");
  lcd.setCursor(13, 1);
  lcd.print(hum2);
  lcd.setCursor(18, 1);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("Hum:");
  lcd.setCursor(5, 2);
  lcd.print("OFF");
  lcd.setCursor(10, 2);
  lcd.print("Circ:");
  lcd.setCursor(16, 2);
  lcd.print("OFF");
  lcd.setCursor(0, 3);
  lcd.print("T");
  lcd.setCursor(1, 3);
  lcd.write(0);
  lcd.setCursor(2, 3);
  lcd.print(":");
  lcd.setCursor(3, 3);
  lcd.print(avgtemp);
  lcd.setCursor(8, 3);
  lcd.write(3);
  lcd.setCursor(10, 3);
  lcd.print("H");
  lcd.setCursor(11, 3);
  lcd.write(0);
  lcd.setCursor(12, 3);
  lcd.print(":");
  lcd.setCursor(13, 3);
  lcd.print(avghum);
  lcd.setCursor(18, 3);
  lcd.print("%");
}


void loop()
{
  // Wait a few seconds between measurements.
  CurMS = millis();
  if ((CurMS - PrevMS >= ReadDHT))
  {
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    hum = dht.readHumidity() + humcal;
    temp = dht.readTemperature(true);     // Read temperature as Fahrenheit (isFahrenheit = true)
    hum2 = dht2.readHumidity() + humcal;
    temp2 = dht2.readTemperature(true);     // Read temperature as Fahrenheit (isFahrenheit = true)
    if(isnan(hum) || isnan(temp) || isnan(hum2) || isnan(temp2))
      {
         ESP.restart();
      }

    avgtemp = (temp + temp2) / 2;
    avghum = (hum + hum2) / 2;
    
    PrevMS = CurMS;

    lcd.setCursor(3,0);
    lcd.print(temp);
    lcd.setCursor(13,0);
    lcd.print(hum);
    lcd.setCursor(3,1);
    lcd.print(temp2);
    lcd.setCursor(13,1);
    lcd.print(hum2);
    lcd.setCursor(3,3);
    lcd.print(avgtemp);
    lcd.setCursor(13,3);
    lcd.print(avghum);  
  }
  
  if ((HumRelayState == LOW) && (avghum < HumFanON))     //Turn on humidity fans if humidity < Set Humidity Level
  {
    HumRelayState = HIGH;
    digitalWrite(RELAYPIN1, HIGH);
    lcd.setCursor(5, 2);
    lcd.print("ON ");
  }
  else if ((HumRelayState == HIGH) && (avghum >= HumFanOFF))
  {
    HumRelayState = LOW;
    digitalWrite(RELAYPIN1, LOW);
    lcd.setCursor(5, 2);
    lcd.print("OFF");
  }
  if((CircRelayState == LOW) && (millis() - CircMS > CircFanON))
  {
    CircRelayState = HIGH;
    digitalWrite(RELAYPIN2, HIGH);
    lcd.setCursor(16, 2);
    lcd.print("ON ");
    CircMS = millis();    
  }
  else if((CircRelayState == HIGH) && (millis() - CircMS > CircFan))
  {
    CircRelayState = LOW;
    digitalWrite(RELAYPIN2, LOW);
    lcd.setCursor(16, 2);
    lcd.print("OFF");    
  }
  if (millis() - ThingMS > ThingNow)
  {
    while( WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      delay(5000);
    }
    timeClient.update();
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, temp2);
    ThingSpeak.setField(3, avgtemp);
    ThingSpeak.setField(4, hum);
    ThingSpeak.setField(5, hum2);
    ThingSpeak.setField(6, avghum);
    h = avghum;
    bigtime = timeClient.getEpochTime();
    if(restarttime == 0)
    {
      restarttime = bigtime;
    }
    f = " " + String(month(restarttime)) + "/" + String(day(restarttime)) + " " + String(hour(restarttime)) + ":" + String(minute(restarttime));
    epochTime = bigtime - 4200;
    z = " is your humidity #humidorstatus  " + String(hour(epochTime)) + ":" + String(minute(epochTime)) + " " + String(month(epochTime)) + "-" + String(day(epochTime)) + "-" + String(year(epochTime)) + f;
//    Serial.println(z);
    if (millis() - TweetMS > TweetNow)
    {
      t = ThingSpeak.setTwitterTweet(Twitter, String(h + z));
      Serial.print("Twitter: ");
      Serial.println(t);
      TweetMS = millis();
    }
    x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    ThingMS = millis();
//    Serial.print("Update Status: ");
//    Serial.println(x);
  }
}

/*
HTTP ERROR CODES
 200  OK / Success
 404  Incorrect API key (or invalid ThingSpeak server address)
-101  Value is out of range or string is too long (> 255 characters)
-201  Invalid field number specified
-210  setField() was not called before writeFields()
-301  Failed to connect to ThingSpeak
-302  Unexpected failure during write to ThingSpeak
-303  Unable to parse response
-304  Timeout waiting for server to respond
-401  Point was not inserted (most probable cause is the rate limit of once every 15 seconds)
0   Other error
 */
