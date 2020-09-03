#include <Adafruit_Sensor.h>
#include <DHT.h>    //DHT Sensor Library
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2
#define DHTPIN2 3         // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define RELAYPIN1 6   // Digital Pin connected to Hum Fan
#define RELAYPIN2 7   // Digital Pin connected to Circ Fan
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4); // (Hex address, colums, rows)

int HumRelayState = LOW;
int CircRelayState = LOW;
unsigned long PrevMS = 0;  //Store last time clock was updated
unsigned long CircMS = 0;  //Store circulation fan time on
long CircFan = 300000;     //Turn circulating fan on for 5 minutes
long CircFanON = 1800000;  //Turn circulating fan on every half hour

DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
long ReadDHT = 3000;       //3 second read delay in ms
const float HumFanON = 67;       //Humidity level to turn ON humidifying fans
const float HumFanOFF = 70;      //Humidity level to turn OFF humidifying fans
float hum = 0;           // initializing Humidity and Temp
float temp = 0;
float hum2 = 0;
float temp2 = 0;
float avghum = 0;
float avgtemp = 0;
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
  Serial.begin(9600);
  dht.begin();
  dht2.begin();

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, avg);
  lcd.createChar(1, one);
  lcd.createChar(2, two);
  lcd.createChar(3, deg);

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
    hum = dht.readHumidity();
    temp = dht.readTemperature(true);     // Read temperature as Fahrenheit (isFahrenheit = true)
    hum2 = dht2.readHumidity();
    temp2 = dht2.readTemperature(true);     // Read temperature as Fahrenheit (isFahrenheit = true)

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
  if((CircRelayState == LOW) && (millis()-CircMS > CircFanON))
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
}
