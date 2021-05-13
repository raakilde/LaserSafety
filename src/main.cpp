/********************************************************************/
// First we include the libraries
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
/********************************************************************/
// Data wire is plugged into pin 3 on the Arduino 
#define ONE_WIRE_BUS 3
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 

#define ABORT_PIN 11
#define SWTICH_PIN 12
int val = 0;      // variable to store the read value

/*
Arduino Water flow meter
YF- S201 Hall Effect Water Flow Sensor
Water Flow Sensor output processed to read in litres/hour
*/
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; // Calculated litres/hour
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;

float temperature = 0;
bool lidOpen = false;
bool noWarning = false;
bool noWarningOldState = false;

LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
const String displayLineOne("Flow:0000 l/h ST");
const String displayLineTwo("Temp:00.00 Lid:1");
const String errorDetected("ST");
const String noErrorDetected("OK");

void flow () // Interrupt function
{
  flow_frequency++;
}


void setup(void) 
{ 
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.clear();
  String version("Laser Heat");
  String version2("Control V1.0");
  lcd.setCursor(0,0); // Move cursor to 0,0 (first line, first column)
  lcd.print(version);
  lcd.setCursor(0,1); // Move cursor to 0,0 (first line, first column)
  lcd.print(version2);
  // Start up the library 
  sensors.begin(); 

  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  Serial.begin(9600);
  attachInterrupt(0, flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

  pinMode(SWTICH_PIN, INPUT);
  pinMode(ABORT_PIN, OUTPUT);
  delay(5000);

  
  lcd.setCursor(0,0); // Move cursor to 0,0 (first line, first column)
  lcd.print(displayLineOne);
  lcd.setCursor(0,1); // Move cursor to 0,0 (first line, first column)
  lcd.print(displayLineTwo);
} 

void HandleWarning(String * error)
{
  // Serial.println(*error);
}

void handleLid()
{
  lidOpen = digitalRead(SWTICH_PIN);
  if (lidOpen)
  {
    String error = String("Lid is open");
    HandleWarning(&error);
    noWarning = false;
  }
}

void handleFlow()
{
  // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
  l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
  flow_frequency = 0; // Reset Counter

  if (l_hour < 10)
  {
    String error = String("No flow is ") + l_hour;
    HandleWarning(&error);
    noWarning = false;
  }
}

void handleTemperature()
{
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  temperature = sensors.getTempCByIndex(0);

  if (temperature > 30)
  {
    String error = String("Temperature over ") + temperature; 
    HandleWarning(&error);
    noWarning = false;
  }
  
}

void loop(void) 
{
  currentTime = millis();
  // Every second, calculate and print litres/hour
  if(currentTime >= (cloopTime))  
  {
    cloopTime = currentTime + 1000; // Updates cloopTime

    noWarning = true;
    handleLid();
    handleTemperature();
    handleFlow();

    Serial.print("Flow: ");
    Serial.print(l_hour, DEC);
    Serial.print(" L/hour, ");
    Serial.print("Temperature: "); 
    Serial.print(temperature);
    Serial.print(", LID open: "); 
    Serial.println(lidOpen);

    lcd.setCursor(5,0); // Move cursor to 0,5 (first line, first column)
    char buffer[4] = {0};
    sprintf(buffer, "%04d", l_hour);
    lcd.print(buffer);
    lcd.setCursor(5,1); // Move cursor to 1,5 (first line, first column)
    lcd.print(temperature);
    lcd.setCursor(15,1); // Move cursor to 1,5 (first line, first column)
    lcd.print(lidOpen);
  }

  if (noWarning != noWarningOldState)
  {
    noWarningOldState = noWarning;
    digitalWrite(ABORT_PIN, noWarningOldState);
    lcd.setCursor(14,0); // Move cursor to 1,5 (first line, first column)

    if(noWarningOldState == true)
    {
      lcd.print(noErrorDetected);
    }
    else
    {
      lcd.print(errorDetected);
    }
  }
  //  if(currentTime >= (cloopTime + 1000))
  //  {
  //     cloopTime = currentTime; // Updates cloopTime
  //     // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
  //     l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
  //     flow_frequency = 0; // Reset Counter
  //     Serial.print(l_hour, DEC); // Print litres/hour
  //     Serial.println(" L/hour");

  //     sensors.requestTemperatures(); // Send the command to get temperature readings 
  //     Serial.print("Temperature is: "); 
  //     Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"?  
  //     val = digitalRead(SWTICH_PIN);   // read the input pin
  //     Serial.print("Lid is open: "); 
  //     if (val == LOW)
  //     {
  //       Serial.println("Closed");
  //     }else
  //     {
  //         Serial.println("Open");
  //     }

  //     digitalWrite(ABORT_PIN, HIGH);
  //  }
} 