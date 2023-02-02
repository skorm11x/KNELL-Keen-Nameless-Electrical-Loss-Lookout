/*
 * Project 705-power-loss-detection
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 2 February 2023
 */

/*
  global variables, including class definitions
*/
int debug; // control variable for print statements
float battery_voltage; // voltage in volts, returns -1.0 if it cannot be read
FuelGauge fuel; // fuel object used to access the battery monitoring circuit
int LED1 = D1;

/*
  Function prototypes for C pre-processor
*/
void get_battery_voltage()
void flash_led(int LED, int time)
void pinMode(uint16_t pin, PinMode mode)


// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(LED, OUTPUT);               // sets pin as output
  debug = 1; // 0  represents no debug, 1 represents debug
  Serial.begin();
  Serial.println("Hello World!");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  
  if(debug){
    println("Beginning main loop in debug.")
    println("Flashing LED: ")
    flash_led(D1, 200);
  }
  else{
    //TBD
  }

  
}


/*
  Reads battery voltage and updates global value
*/
void get_battery_voltage(){
  battery_voltage = fuel.getVCell();
  if(debug == 1){
    Serial.printf("voltage=%.2f", battery_voltage);
  }
}

/*
  Flash the LED: turning on for a set time and then turning off.
  LED: the led setup to be turned on and off
  time: time in milliseconds, as an integer
*/
void flash_led(int LED, int time){
  //TODO: probably store this information in addressable array to see setup config
  //Ensure LED exists/ pinmode enabled
    digitalWrite(LED, HIGH);          // sets the LED on
    delay(time);                      
    digitalWrite(LED, LOW);           // sets the LED off
    delay(time);  
}