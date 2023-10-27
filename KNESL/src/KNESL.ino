/*
 * Project 705-sensor-nodes (KNESL) 
 * Keen Nameless Electronic Sensor Lookout
 * Description: Node power loss detection & sensor additions routed to KNELL gateway
 * Author: Christopher J. Kosik
 * Date: 27 Oct 2023
 * 
 * 
 * NOTES: 
 * This node is implemented on a borgon board but can be abstracted
 * to any WIFI or BLE capable microprocessor board We rely on
 * direct wifi between devices for data transfer
 */

// The following is our call to the community library for device diagnostics
// https://github.com/rickkas7/DiagnosticsHelperRK
#include "DiagnosticsHelperRK.h"
#include "DS18.h"
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

/*
  Particle specific configurations for modes, power, and data. 
  See the following: https://docs.particle.io/reference/device-os/api/system-modes/automatic-mode-threading-enabled/
*/
SYSTEM_MODE(AUTOMATIC); // this defaults to device immediately establishing cloud connection. Afterwords it executes setup() and loop()
SYSTEM_THREAD(ENABLED); // handles data/ cloud messaging in a seperate thread from our setup() and loop()

/*
  Global variables, including class definitions
*/
particle::Future<bool> publish(const char* name, const char* data);
FuelGauge fuel; // fuel object used to access the battery monitoring circuit
SerialLogHandler logHandler;
int debug; // control variable for print statements
DS18 sensor0(D5);
float cell_sig_str; //celluar signal strength
float cell_sig_qual; //celluar signal quality
double battery_voltage; // voltage in volts, returns -1.0 if it cannot be read
bool isOnWallPower;
unsigned long lastSync = millis();
int initPowerSource;
int powerSource;
String powerSourceStr;
String status;
int lastPowerSource = -1;

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

void setup() {
  initPowerSource = 3; //this should be USB/ wall power for our design
  debug = 1; // 0  represents no debug, 1 represents debug

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
     Serial.printlnf("power src str: %s", powerSourceStr.c_str());
     if (sensor0.read()){
      Serial.printlnf("Temperature: %s", String(sensor0.celsius()));
     }
     delay(1000); //delay in ms
  }
  else{
    check_day_time_sync();
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
  Reads battery voltage and updates global value
*/
void get_battery_voltage(){
  battery_voltage = fuel.getVCell();
  if(debug == 1){
    Log.info("Current battery voltage: %f", battery_voltage);
  }
}