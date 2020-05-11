
#include <HX711.h>
#include <Wire.h>
#include <Servo.h>
#define motor_pin 9                 //motor pin on arduino
#define pressure_clock A0           //pressure sensor clock pin
#define pressure_out A1             //pressure sensor output pin
//#define flowPin A2           //flowrate  sensor pin
#define COMPRESSION_INTERVAL_KNOB A3     // COMPRESSION_INTERVAL KNOB
#define COMPRESSION_LENGHT_KNOB A4     // COMPRESSION_lenght KNOB
byte sensorInterrupt = 0;  // 0 = digital pin 2
#define FlowMeterPulsePerSecondPerLiter 6.9
//#define FlowMeterPulsePerSecondPerLiter 7.5
float FlowCalculationDuration = 1000;

float FlowCalculationStartTime;
volatile float PulseCount;
float LPM;
int LPMdecimal;


int  compression_interval = 0;
int debug = 0;
//declarations
Servo motor;
HX711 pressure_scale;
void pressure_init();
void run_motor(int intervals);
void motor_function(int average_val, int compression_interval);



void setup() {
Serial.begin(115200);
  
  LPM               = 0;
  LPMdecimal        = 2;
  attachInterrupt(sensorInterrupt, pulseCounter2, CHANGE);
  FlowCalculationStartTime = millis();
  motor.attach(motor_pin);            // motor set pin as input
  // interrupt
  // attachInterrupt(0, Flow, RISING);  //Configures interrupt 0 (pin 2 on the Arduino Uno) to run the function "Flow"
  pressure_init(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  int cLenght = analogRead(COMPRESSION_LENGHT_KNOB);     //varing lenght
  int intervals = analogRead(COMPRESSION_INTERVAL_KNOB);    // read the value from the sensor
  // compression rates
if (intervals < 171) {
    compression_interval = 1000;
  } else if (intervals < 341) {
    compression_interval = 2000;
  } else if (intervals < 512) {
    compression_interval = 3000;
  } else if (intervals < 683) {
    compression_interval = 4000;
  } else if (intervals < 851) {
    compression_interval = 5000;
  } else if (intervals < 1025) {
    compression_interval = 6000;
  }
  int average_val = pressure_scale.get_units(10);
  PulseCount = 0;
  //    count = 0;      // Reset the counter so we start counting from 0 again
  interrupts();   //Enables interrupts on the Arduino
  delay(1000);   //Wait 1 second
  noInterrupts(); //Disable the interrupts on the Arduino
 // Serial.println(compression_interval);
  motor_function(average_val, compression_interval, cLenght);
  
  
  Serial.print(cLenght);
  Serial.print(" ");
  Serial.print(intervals);
  Serial.print(" ");
  Serial.print(pressure_scale.get_units(), 1);
  Serial.print(" "); 
  Serial.println(CalculateFlow2(), 2);
  pressure_scale.power_down();                                                  // put the ADC in sleep mode
  pressure_scale.power_up();
}




void run_motor(int intervals, int cLenght) {
  cLenght = map(cLenght, 0, 1025, 22, 360);
  //motor.write(cLenght);
  motor.write(20);
  delay(2000);
  motor.write(360);
  delay(intervals);
  if (debug > 0) {
    Serial.print("\n\t| lenght val:\t\n");
    Serial.println(cLenght);
    Serial.print("\n\t| inytervals:\t\n");
    Serial.println(intervals);
  }

}

void motor_function(int average_val, int compression_interval, int clenght) {
  int pressure_norm = 100;
  if (!(average_val > pressure_norm)) {    // check if pressure level is fine
    if (debug > 0) {
      Serial.print("\n\t| average:\t\n");
      Serial.println(average_val, 1);
    }
    run_motor(compression_interval, clenght);
  } else {
    delay(1000);                    //delay motor for 1 sec
    if (debug > 0) {
      Serial.print("\n\t motor was delayed due to respirational inbalance\t\n");
      Serial.println(average_val, 1);
    }
  }
}

void pressure_init() {
  //pressure sensor init
 Serial.println("Compression_Length Compression_interval Pressure_level Flow_rate");

  pressure_scale.begin(pressure_out, pressure_clock);

  

  int r , i, v, u = 0;
  r = pressure_scale.read();                                  // print a raw reading from the ADC



  i = pressure_scale.read_average(20);              // print the average of 20 readings from the ADC


  v = pressure_scale.get_value(
        5);                        // print the average of 5 readings from the ADC minus the tare weight (not set yet)



  u = pressure_scale.get_units(5);  // print the average of 5 readings from the ADC minus tare weight (not set) divided

  pressure_scale.set_scale(
    2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details

  pressure_scale.tare();                                                                   // reset the scale to 0

  r = pressure_scale.read();                 // print a raw reading from the ADC



  i = pressure_scale.read_average(20);       // print the average of 20 readings from the ADC


  v = pressure_scale.get_value(
        5);                        // print the average of 5 readings from the ADC minus the tare weight, set with tare()



  u = pressure_scale.get_units(5);       // print the average of 5 readings from the ADC minus tare weight, divided


}


float CalculateFlow2()
{
  if (millis() - FlowCalculationStartTime > FlowCalculationDuration)
  {
    LPM = PulseCount / (2 * FlowMeterPulsePerSecondPerLiter * (FlowCalculationDuration / 1000));
    FlowCalculationStartTime = millis();
    PulseCount = 0;
    return LPM;
  }
  return LPM;
}


void pulseCounter2()
{
  PulseCount++;
}


void DisplayLPM()
{
  Serial.println(LPM, LPMdecimal);
}
