// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22  
#define INTERVAL 10000 

#define analogMQ7 A1 

DHT dht(DHTPIN, DHTTYPE);

// Declare global variables
double temp;
double hum;
float tempThreshold = 40;
double humThreshold = 60;
unsigned long lastCheck = 0;

bool readingType = true;
bool buzzerTest = false;

int MQ7sensorValue = 0; 

// Declare peripherals
int redLED = A5;
int yellowLED = D12;
int greenLED = D9;
int defaultLED = D7;
int buzzer = D6;

void readSensor() 
{
    hum = dht.getHumidity();
    temp = dht.getTempCelcius();
}

// Callback function to allow the Particle console access to readSensor()
int checkSensorRead(String command) 
{
    readSensor();
    return 1;
}

// Function to change maximum threshold of humidity
int newHumThreshold(String command)
{
    humThreshold = command.toInt();
    Particle.publish("Debug", "Humidity Threshold Updated", int(hum), PRIVATE);
    return 1;
}

int toggleReading(String command)
{
    if (command.equals("humidity") || command.equals("temperature"))
    {
        readingType = false;
        return 1;
    }
    else if (command.equals("CO"))
    {
        readingType = true;
        return 0;
    }
    else
    {
        // Unknown option
        return -1;
    }
}

void tempAndHumRead()
{
    if (lastCheck + INTERVAL < millis()) 
    {
        lastCheck = millis();
        readSensor();

        // Check if sensor can read, retrys again if failed
        if (isnan(hum) || isnan(temp)) 
        {
            // Error message 
            Particle.publish("ERROR", "Cannot detect readings from sensor!");
            return;
        }
        
        // Send values to mobile companion app
        Particle.publish("Temperature", String(temp) + "°C", PRIVATE);
        Particle.publish("Humidity", String(hum) + "%", PRIVATE);
    }
    
    // Send warning if humidity is too high
    // Turn on red LED and activate buzzer if humidity is too high
    // Green LED flashing indicates humidity has decreased under 60%
    if (hum > humThreshold)
    {
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, HIGH);
        tone(buzzer, 2093, 650);
        Particle.publish("WARNING", "High humidity, please activate dehumidifier");
        delay(1000);
        digitalWrite(redLED, LOW);
        delay(1000);
    }
    if (hum < humThreshold) 
    {
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, HIGH);
        delay(1000);
        digitalWrite(greenLED, LOW);
        delay(1000);
    }
}

void coRead()
{
    // LOOP CANNOT BE PARALLELISED - for demonstration purposes, only show one function at a time
    // Gas sensor readings
    analogWrite(analogMQ7, HIGH);   // initial increase heating power
    delay(10000);                   // heat for 1 minute
    analogWrite(analogMQ7, 71.4);   // reduce heating power 
    delay(15000);                   // wait 90 seconds
    analogWrite(analogMQ7, HIGH);
    delay(25);
    MQ7sensorValue = analogRead(analogMQ7);
    
    if (MQ7sensorValue <= 200) 
    {
        Particle.publish("Air-Quality", "CO perfect");
    }
    else if ((MQ7sensorValue > 200) || (MQ7sensorValue <= 800))
    {
        Particle.publish("Air-Quality", "CO normal");
    }
    else if ((MQ7sensorValue > 800))
    {
        Particle.publish("Air-Quality", "CO high");
    }
    else
    {
        Particle.publish("ERROR", "Can't read air quality sensor!");
    }
}

int alarmTest(String command)
{
    if (command.equals("on"))
    {
        digitalWrite(redLED, HIGH);
        tone(buzzer, 2093, 650);
        Particle.publish("ALARM TEST", "Alarm functioning correctly");
        delay(1000);
        digitalWrite(redLED, LOW);
        delay(1000);
        digitalWrite(redLED, HIGH);
        tone(buzzer, 2093, 650);
        delay(1000);
        digitalWrite(redLED, LOW);
        delay(1000);
        digitalWrite(redLED, HIGH);
        tone(buzzer, 2093, 650);
        delay(1000);
        digitalWrite(redLED, LOW);
        delay(1000);
        return 1;
    }
    else
    {
        // Unknown option
        return -1;
    }
}

void setup() 
{
    dht.begin();

    Particle.variable("temperature", temp);
    Particle.variable("humidity", hum);
    Particle.function("readSensor", checkSensorRead);
    Particle.function("Set New Humidity Threshold", newHumThreshold);
    Particle.function("Change Device Reading", toggleReading);
    Particle.function("Test Device Alarm", alarmTest);
    
    pinMode(redLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(greenLED, OUTPUT);
    pinMode(defaultLED, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(analogMQ7, INPUT);
}

void loop() 
{
    while (readingType)
    {
        coRead();
    }
    
    tempAndHumRead();
}

