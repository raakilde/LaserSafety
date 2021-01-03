/********************************************************************/
// First we include the libraries
#include <OneWire.h> 
#include <DallasTemperature.h>
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

void flow () // Interrupt function
{
   flow_frequency++;
}


void setup(void) 
{ 
  // start serial port 
  Serial.begin(9600); 
  Serial.println("Dallas Temperature IC Control Library Demo"); 
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
  }

  if (noWarning != noWarningOldState)
  {
    noWarningOldState = noWarning;
    digitalWrite(ABORT_PIN, noWarningOldState);
    Serial.println("Relay state");
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