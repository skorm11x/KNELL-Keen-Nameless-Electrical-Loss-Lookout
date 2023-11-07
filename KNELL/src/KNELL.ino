/*
 * Project 705-power-loss-detection (KNELL)
 * Keen Nameless Electronic Loss Lookout
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

/*
  We store these for more diagnostic messages for the future.
*/
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
void detect_power_state();
void pinMode(uint16_t pin, PinMode mode);
void dev_tests();


// setup() runs once, when the device is first turned on.
void setup() {
  // This is how we expose a variable to GET requests from the cloud
  // Particle.variable("debug", debug);
  Particle.variable("battery_voltage", battery_voltage);
  initPowerSource = 3; //this should be USB/ wall power for our design
  debug = 0; // 0  represents no debug, 1 represents debug

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
    BLE.on();
    BLE.setDeviceName("KNELL");

}


void loop() {

  if(debug){
    if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);
      get_battery_voltage();
    }
    detect_power_state();
  }
  else{
    check_day_time_sync();
    detect_power_state();
  }
  //dev_tests();
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
  See POWER_CODES structure for string messages associated with message reads.
*/
void detect_power_state() {
  bool success;
  powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
  //powerSourceStr = p_table[powerSource].value;
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
      //powerSourceStr = p_table[powerSource].value;
      if(powerSource != initPowerSource) {
        if(debug) {
          Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
        }
        if(powerSource == 5) {
          // Power changed from last and we are now on battery power: yields power loss!
          powerSourceStr = "POWER LOSS AT DMOC";
          get_battery_voltage();
          status = String::format("{\"powerSource\":\"%s\"}", powerSourceStr.c_str());
          success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          lastPowerSource = powerSource;
          while(!success) {
            // get here if event publish did not work, reattempt
            success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
          }
        }
        if(powerSource == 1) {
          // Power changed from last and we are now on VIN power: yields power restored!
          powerSourceStr = "POWER RESTORED AT DMOC";
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
          // Power changed from last and we are running off of USB now: yields power restored / on AC power!
          powerSourceStr = "POWER RESTORED AT DMOC";
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
    //powerSourceStr = p_table[powerSource].value;
    powerSourceStr = "POWER RESTORED AT DMOC";
    if(powerSource == initPowerSource) {
      if(debug) {
        Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
        Log.info("power src INT: %d", powerSource);
      }
        if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
          // Power changed from last and we are running off of USB now: yields power restored / on AC power!
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

void dev_tests() {
   String devStr = "HELLO WORLD";
   String test_status = String::format("{\"DEV\":\"%s\"}", devStr.c_str());

   bool tst_success = Particle.publish("dev_events", test_status, PRIVATE, WITH_ACK);
   while(!tst_success) {
        // get here if event publish did not work, reattempt
        tst_success = Particle.publish("dev_events", test_status, PRIVATE, WITH_ACK);
      }

}
