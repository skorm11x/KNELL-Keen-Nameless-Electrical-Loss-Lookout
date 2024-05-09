/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/christopherkosik/Documents/codes/KNELL_SYS/KNESL/src/KNESL.ino"
/*
 * Project 705-sensor-nodes (KNESL) 
 * Keen Nameless Electronic Sensor Lookout
 * Description: Node power loss detection & sensor additions routed to KNELL gateway
 * Author: Christopher J. Kosik
 * Date: 6 NOV 2023
 * 
 * 
 * NOTES: 
 * This node is implemented on a borgon board but can be abstracted
 * to any WIFI or BLE capable microprocessor board We rely on
 * direct wifi between devices for dasta transfer
 */

// #define DEV_DEBUG
// #define BLE_DEBUG
// #define POWER_DEBUG
// #define CELLULAR_DEBUG
// #define SENSOR_DEBUG
void setup();
void loop();
void detect_power_state();
void convert_to_gatt_format(float value, uint8_t* buf);
void convert_to_gatt_format(double value, uint8_t* buf);
#line 20 "/Users/christopherkosik/Documents/codes/KNELL_SYS/KNESL/src/KNESL.ino"
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

// The following is our call to the community library for device diagnostics
// https://github.com/rickkas7/DiagnosticsHelperRK
// The following for DS18/OneWire library 
// https://github.com/particle-iot/OneWireLibrary/tree/master
#include "DiagnosticsHelperRK.h"
#include "DS18.h"

/*
  Particle specific configurations for modes, power, and data. 
  See the following: https://docs.particle.io/reference/device-os/api/system-modes/automatic-mode-threading-enabled/
*/
SYSTEM_MODE(AUTOMATIC); // this defaults to device immediately establishing cloud connection. Afterwords it executes setup() and loop()
SYSTEM_THREAD(ENABLED); // handles data/ cloud messaging in a seperate thread from our setup() and loop()

/*
  Global variables, including class definitions
*/
FuelGauge fuel; // fuel object used to access the battery monitoring circuit
SerialLogHandler logHandler;
DS18 sensorBus(D5, true);

const unsigned long UPDATE_INTERVAL_MS = 2000;
unsigned long lastUpdate = 0;

float cell_sig_str; //celluar signal strength
float cell_sig_qual; //celluar signal quality
double battery_voltage = 3.7; // voltage in volts, returns -1.0 if it cannot be read
uint8_t batt_percent = (uint8_t)((battery_voltage/3.7)*100);
bool isOnWallPower;
unsigned long lastSync = millis();
int initPowerSource;
int powerSource;
String powerSourceStr;
String status;
uint8_t lastPowerSource = -1;
float last_temp_bus = 25.0; // 77.0 deg F initialized

/*
  BLE definitions
*/
// The "environment " service is 0x181A
// See https://www.bluetooth.com/specifications/gatt/services/
const BleUuid envMonitoringService(0x181A);
const BleUuid powerStateService("0a59feb8-625f-460b-a9dc-e3698701d9ff");

// We're using a well-known characteristics UUID. They're defined here:
// https://www.bluetooth.com/specifications/gatt/characteristics/
// The temperature-measurement is 16-bit UUID 0x2A6E
BleCharacteristic temperatureMeasurementCharacteristic("temp",
 BleCharacteristicProperty::NOTIFY, BleUuid(0x2A6E), envMonitoringService);

 // The battery level service allows the battery level to be monitored
BleUuid batteryLevelService(BLE_SIG_UUID_BATTERY_SVC);
// The battery_level characteristic shows the battery level of
BleCharacteristic batteryLevelCharacteristic("bat", BleCharacteristicProperty::NOTIFY, BleUuid(0x2A19), batteryLevelService);

BleCharacteristic powerStateCharacteristic("power state",
 BleCharacteristicProperty::NOTIFY, BleUuid("bddfdfad-b9bf-488c-a86c-7017a160b3c8"), powerStateService);

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
void poll_temp_c();
void convert_to_gatt_format(float, uint8_t*);
void convert_to_gatt_format(double, uint8_t*);

void setup() {
  initPowerSource = 3; //this should be USB/ wall power for our design

  if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      #ifdef CELLULAR_DEBUG
        // Prints out the cellular 
        Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);
        // Prints out the local (private) IP over Serial
        Log.info("localIP: %s", Cellular.localIP().toString().c_str());
      #endif  
      Serial.begin(9600);
    }
    // https://docs.particle.io/reference/device-os/bluetooth-le/
    BLE.on();
    BLE.setDeviceName("KNESL-0");
    BLE.addCharacteristic(temperatureMeasurementCharacteristic);
    BLE.addCharacteristic(batteryLevelCharacteristic);
    BLE.addCharacteristic(powerStateCharacteristic);
    powerStateCharacteristic.setValue(&lastPowerSource, 1);
    batteryLevelCharacteristic.setValue(&batt_percent, 1);

    BleAdvertisingData advData;
    // While we support both the health thermometer service and the battery service, we
    // only advertise the health thermometer. The battery service will be found after
    // connecting.
    advData.appendServiceUUID(envMonitoringService);
    advData.appendServiceUUID(powerStateService);
    advData.deviceName("KNESL-0", 7);

    // Continuously advertise when not connected
    BLE.advertise(&advData);

}

void loop() {
  poll_temp_c(); 
  check_day_time_sync();
  detect_power_state();
  get_battery_voltage();
  if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
      lastUpdate = millis();
      if (BLE.connected()) {
          // The Temperature Measurement characteristic data is defined here:
          // https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.temperature_measurement.xml
          uint8_t sensorBus_buf[2];
          convert_to_gatt_format(last_temp_bus, sensorBus_buf);
          temperatureMeasurementCharacteristic.setValue(sensorBus_buf, sizeof(sensorBus_buf));
          batteryLevelCharacteristic.setValue(&batt_percent, sizeof(batt_percent));
      }
    }
}

/*
  Reads battery voltage and updates stack value
*/
void get_battery_voltage(){
  battery_voltage = fuel.getVCell();
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
  powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
  #ifdef POWER_DEBUG
    Log.info("power src str: %s", powerSourceStr.c_str());
    Log.info("power src INT: %d", powerSource);
  #endif
    if(powerSource != initPowerSource && powerSource != lastPowerSource) {
        // the power source just changed from its initial 
        // wait 5 seconds and double check to see if it was just a blip/ misread then affirm or break
        #ifdef POWER_DEBUG
          Log.info("Potential Power source change: %s", powerSourceStr.c_str());
        #endif 
        delay(5000);
        powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
        //powerSourceStr = p_table[powerSource].value;
        if(powerSource != initPowerSource) {
          #ifdef POWER_DEBUG
            Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
          #endif
          // On battery power
          if(powerSource == 5) {
            // Power changed from last and we are now on battery power: yields power loss!
            lastPowerSource = powerSource;
          }
          //On VIN power. Unexpected and should be an error
          if(powerSource == 1) {
            lastPowerSource = powerSource;
          }
          // should never get here if we initialized initPowerSource to USB correctly
          // On Wall AC power
          if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
            // Power changed from last and we are running off of USB now: yields power restored / on AC power!
            lastPowerSource = powerSource;
          }
        }
    }
    if(powerSource == initPowerSource && lastPowerSource != powerSource && lastPowerSource != -1) {
      // Getting here indicates that we set lastPowerSource (changed from init to seperate power)
      // and then we restored back to the original initialized power source 
      // Basically, this is going onto battery for a while and then back to USB power
      #ifdef POWER_DEBUG
        Log.info("Potential Power source change: %s", powerSourceStr.c_str());
      #endif 
      delay(5000);
      powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
      if(powerSource == initPowerSource) {
        #ifdef POWER_DEBUG
          Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
          Log.info("power src INT: %d", powerSource);
        #endif
          // On Wall AC power
          if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
            // Power changed from last and we are running off of USB now: yields power restored / on AC power!
            lastPowerSource = powerSource;
          }
      }
  }
}

/*
  Poll the sensor bus of the DS18 for sensor values.
  All sensors have a unique 64-bit address and publish
  results that we loop around a read as function called.
*/
void poll_temp_c(){
  /*
    Copies the 1-Wire ROM data / address of the last read 
    device in the buffer. All zeros if no device was found
    or search is done.
  */
  uint8_t addr[8];
  sensorBus.addr(addr);
  bool success = sensorBus.read();
  if (success) {
    /*
      You need to decide which sensor on the local bus, if multple
      are attached like in current schematic, is attached to the 
      environmental sensing service/ character.
      //ignore addr of all zero. See: https://github.com/particle-iot/OneWireLibrary/tree/master
      // and the search done section API
    */
    int i;
    if(addr != 0){
      last_temp_bus = sensorBus.celsius();
      #ifdef SENSOR_DEBUG
        Serial.printlnf("temperature: %f", last_temp_bus);
        Serial.printf("address: ");
        for (i = 0; i < 7; i++){
          Serial.printf("%d", addr[i]);
        }
        Serial.println("");
      #endif
    }
  }
}

/*
  Take in a raw float value and a pointer to the 
  buffer uint8_t that we will attach to our 
  BLE characteristic(s)
*/
void convert_to_gatt_format(float value, uint8_t* buf) {
  //scale by a dec factor of 100!
  int16_t raw_value = value * 100;
  buf[0] = raw_value & 0xFF; // first byte masked with max
  buf[1] = (raw_value >> 8) & 0xFF; //shift right & mask with max
}

/*
  Take in a raw float value and a pointer to the 
  buffer uint8_t that we will attach to our 
  BLE characteristic(s)
*/
void convert_to_gatt_format(double value, uint8_t* buf) {
  // scale by a dec factor of 100!
  int16_t raw_value = value * 100;
  buf[0] = raw_value & 0xFF; // first byte masked with max
  buf[1] = (raw_value >> 8) & 0xFF; // shift right & mask with max
}