/*
 * Project 705-power-loss-detection
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 1 February 2023
 */

/*
  global variables, including class definitions
*/
int debug; // control variable for print statements
float battery_voltage; // voltage in volts, returns -1.0 if it cannot be read
FuelGauge fuel; // fuel object used to access the battery monitoring circuit

/*
  Function prototypes for C pre-processor
*/
void get_battery_voltage()


// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
debug = 1; // 0  represents no debug, 1 represents debug
Serial.begin();
Serial.println("Hello World!");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  
  

  
}


// Reads battery voltage and updates global value
void get_battery_voltage(){
  battery_voltage = fuel.getVCell();
  if(debug == 1){
    Serial.printf("voltage=%.2f", battery_voltage);
  }
}