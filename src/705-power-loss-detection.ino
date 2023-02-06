/*
 * Project 705-power-loss-detection
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 2 February 2023
 */

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

/*
  Particle specific configurations for modes, power, and data. 
  See the following: https://docs.particle.io/reference/device-os/api/system-modes/automatic-mode-threading-enabled/
*/
SYSTEM_MODE(AUTOMATIC); // this defaults to device immediately establishing cloud connection. Afterwords it executes setup() and loop()
SYSTEM_THREAD(ENABLED); // handles data/ cloud messaging in a seperate thread from our setup() and loop()
//TODO: evaluate the semi-automatic modes to see if battery check if going to be required before we attempt a cloud data operation
//https://docs.particle.io/reference/device-os/api/system-modes/semi-automatic-mode/

/*
  Global variables, including class definitions
*/
FuelGauge fuel; // fuel object used to access the battery monitoring circuit
SerialLogHandler logHandler;
int debug; // control variable for print statements
float cell_sig_str; //celluar signal strength
float cell_sig_qual; //celluar signal quality
float battery_voltage; // voltage in volts, returns -1.0 if it cannot be read
bool isOnWallPower;
unsigned long lastSync = millis();
// char *cell_private_ip; //cellular connection private ip, TBD
int LED1 = D5;

/*
  Function prototypes for C pre-processor
*/
void get_battery_voltage();
void check_day_time_sync();
void flash_led(int LED, int time);
void pinMode(uint16_t pin, PinMode mode);


// setup() runs once, when the device is first turned on.
void setup() {
  // This is how we expose a variable to GET requests from the cloud
  Particle.variable("debug", debug);

  // Put initialization like pinMode and begin functions here.
  pinMode(LED1, OUTPUT);               // sets pin as output
  debug = 1; // 0  represents no debug, 1 represents debug

   // We are also going to declare a Particle.function so that we can turn the LED on and off from the cloud.
   Particle.function("led",ledToggle);
   // This is saying that when we ask the cloud for the function "led", it will employ the function ledToggle() from this app.


  if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      if(debug){
        // Prints out the cellular 
        Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);
        // Prints out the local (private) IP over Serial
        Log.info("localIP: %s", Cellular.localIP().toString().c_str());

        Serial.begin(9600);
      }
    }

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  check_day_time_sync();
  if(debug){
    // Serial.println("Beginning main loop in debug.");
    // Serial.println("Flashing LED: ");
    flash_led(LED1, 200);

    if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);

    }
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
  TEST/Debug function.
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

/*
  Checked at top of every loop, sync time with cloud at least daily.
*/
void check_day_time_sync() {
  if (millis() - lastSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Particle Device Cloud
    Particle.syncTime();
    lastSync = millis();
  }
}

/*
  Example "Particle" cloud event handler function. We use the standard connotation for return value success or failures.
*/
int ledToggle(String command) {
    /* Particle.functions always take a string as an argument and return an integer.
    Since we can pass a string, it means that we can give the program commands on how the function should be used.
    In this case, telling the function "on" will turn the LED on and telling it "off" will turn the LED off.
    Then, the function returns a value to us to let us know what happened.
    In this case, it will return 1 for the LEDs turning on, 0 for the LEDs turning off,
    and -1 if we received a totally bogus command that didn't do anything to the LEDs.
    */

    if (command=="on") {
        //digitalWrite(led1,HIGH);
        //digitalWrite(led2,HIGH);
        return 1;
    }
    else if (command=="off") {
        //digitalWrite(led1,LOW);
        //digitalWrite(led2,LOW);
        return 0;
    }
    else {
        return -1;
    }
}