/*
 * Project 705-power-loss-detection
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 2 February 2023
 */

// The following is our call to the community library for device diagnostics
// https://github.com/rickkas7/DiagnosticsHelperRK
#include "DiagnosticsHelperRK.h"
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
particle::Future<bool> publish(const char* name, const char* data);
FuelGauge fuel; // fuel object used to access the battery monitoring circuit
SerialLogHandler logHandler;
int debug; // control variable for print statements
float cell_sig_str; //celluar signal strength
float cell_sig_qual; //celluar signal quality
double battery_voltage; // voltage in volts, returns -1.0 if it cannot be read
bool isOnWallPower;
unsigned long lastSync = millis();
// char *cell_private_ip; //cellular connection private ip, TBD
int LED1 = D5;
int initPowerSource;
int powerSource;
String powerSourceStr;
String status;
int lastPowerSource = -1;

/*
  define a structure to map the integer power codes to names of power sources
      POWER_SOURCE_UNKNOWN = 0,
		  POWER_SOURCE_VIN = 1,
			POWER_SOURCE_USB_HOST = 2,
			POWER_SOURCE_USB_ADAPTER = 3,
			POWER_SOURCE_USB_OTG = 4,
			POWER_SOURCE_BATTERY = 5
*/ 
typedef struct power_codes {
  int key;
  const char *value;
} POWER_CODES;

POWER_CODES p_table[] = {
  {0, "UNKNOWN POWER"}, 
  {1, "VIN POWER"}, 
  {2, "WALL AC POWER"}, 
  {3, "WALL AC POWER"},
  {4, "WALL AC POWER"},
  {5, "BATTERY POWER"}, 
};

/*
  Function prototypes for C pre-processor
*/
void get_battery_voltage();
void check_day_time_sync();
void test_flash_led(int LED, int time);
void detect_power_source();
void pinMode(uint16_t pin, PinMode mode);


// setup() runs once, when the device is first turned on.
void setup() {
  // This is how we expose a variable to GET requests from the cloud
  // Particle.variable("debug", debug);
  Particle.variable("battery_voltage", battery_voltage);
  initPowerSource = 3; //this should be USB/ wall power for our design
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


void loop() {

  if(debug){
    // Serial.println("Beginning main loop in debug.");
    // Serial.println("Flashing LED: ");
    test_flash_led(LED1, 200);
    if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);
      get_battery_voltage();
    }
    detect_power_source();
  }
  else{
    check_day_time_sync();
    detect_power_source();
  }
  delay(10000);
  
}


/*
  Reads battery voltage and updates global value
*/
void get_battery_voltage(){
  battery_voltage = fuel.getVCell();
  if(debug == 1){
    Log.info("Current battery voltage: %f", battery_voltage);
  }
}

/*
  TEST/Debug function.
  Flash the LED: turning on for a set time and then turning off.
  LED: the led setup to be turned on and off
  time: time in milliseconds, as an integer
*/
void test_flash_led(int LED, int time){
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
  Checked at top of every loop and during setup() to see if power has changed from its initialization source
      POWER_SOURCE_UNKNOWN = 0,
		  POWER_SOURCE_VIN = 1,
			POWER_SOURCE_USB_HOST = 2,
			POWER_SOURCE_USB_ADAPTER = 3,
			POWER_SOURCE_USB_OTG = 4,
			POWER_SOURCE_BATTERY = 5
*/
void detect_power_source() {
  bool success;
  powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
  powerSourceStr = p_table[powerSource].value;
  //String powerSourceStr = "test string!!!!";
  if(debug) {
    Log.info("power src str: %s", powerSourceStr.c_str());
    Log.info("power src INT: %d", powerSource);
    //Serial.println("power source str: %s", powerSourceStr.c_str());
  }
  if (powerSource != initPowerSource && powerSource != lastPowerSource) {
      // the power source just changed from its initial 
      // wait 5 seconds and double check to see if it was just a blip/ misread then affirm or break
      if(debug) {
        Log.info("Potential Power source change: %s", powerSourceStr.c_str());
      } 
      delay(5000);
      powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
      Log.info("power src INT: %d", powerSource);
      powerSourceStr = p_table[powerSource].value;
      if(powerSource != initPowerSource) {
        if(debug) {
          Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
        }
        if(powerSource == 5) {
          get_battery_voltage();
          //String status = String("Changed from "+String(initPowerSource)+" to: battery @"+battery_voltage+" V.");
          status = String::format("{\"powerSource\":\"%s\"}", powerSourceStr.c_str());
          success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          lastPowerSource = powerSource;
          while(!success) {
            // get here if event publish did not work, reattempt
            success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          }
        }
        if(powerSource == 1) {
          //String status = String("Changed from "+powerSourceStr+" to: VIN.");
          status = String::format("{\"powerSource\":\"%s\"}", powerSourceStr.c_str());
          success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          while(!success) {
            // get here if event publish did not work, reattempt
            success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          }
          lastPowerSource = powerSource;
        }
        // should never get here if we initialized initPowerSource to USB correctly
        if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
          //String status = String((initPowerSource)+" to: USB/Wall power.");
          status = String::format("{\"powerSource\":\"%s\"}", powerSourceStr.c_str());
          success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          while(!success) {
            // get here if event publish did not work, reattempt
            success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          }
          lastPowerSource = powerSource;
        }
      }
  }
  if(powerSource == initPowerSource && lastPowerSource != powerSource && lastPowerSource != -1) {
    // Getting here indicates that we set lastPowerSource (changed from init to seperate power)
    // and then we restored back to the original initialized power source 
    // Basically, this is going onto battery for a while and then back to USB power
    if(debug) {
      Log.info("Potential Power source change: %s", powerSourceStr.c_str());
    } 
    delay(5000);
    powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
    powerSourceStr = p_table[powerSource].value;
    if(powerSource == initPowerSource) {
      if(debug) {
        Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
        Log.info("power src INT: %d", powerSource);
      }
        if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
          //String status = String("Changed from "+powerSourceStr+" to: USB/Wall power.");
          status = String::format("{\"powerSource\":\"%s\"}", powerSourceStr.c_str());
          success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          while(!success) {
            // get here if event publish did not work, reattempt
            success = Particle.publish("power_change s", status, PRIVATE, WITH_ACK);
          }
          lastPowerSource = powerSource;
        }
    }
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
