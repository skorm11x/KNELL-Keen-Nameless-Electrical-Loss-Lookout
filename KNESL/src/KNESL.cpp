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
 * Date: 27 Oct 2023
 * 
 * 
 * NOTES: 
 * This node is implemented on a borgon board but can be abstracted
 * to any WIFI or BLE capable microprocessor board We rely on
 * direct wifi between devices for dasta transfer
 */

// The following is our call to the community library for device diagnostics
// https://github.com/rickkas7/DiagnosticsHelperRK
// The following for DS18/OneWire library 
// https://github.com/particle-iot/OneWireLibrary/tree/master
#include "DiagnosticsHelperRK.h"
#include "DS18.h"
void setup();
void loop();
void convert_to_gatt_format(float temperature, uint8_t* buf);
#line 21 "/Users/christopherkosik/Documents/codes/KNELL_SYS/KNESL/src/KNESL.ino"
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
DS18 sensor0(D5, true);

const unsigned long UPDATE_INTERVAL_MS = 2000;
unsigned long lastUpdate = 0;

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

//TODO: update debug to be C preprocessor part
//TODO: refactor battery detection code to remove cloud connection
//TODO: fragment this out to some other files to clean up the code
float last_temp_0 = 25.0; // 77.0 deg F initialized
uint8_t lastBattery = 100;

/*
  BLE definitions
*/
// The "environment " service is 0x181A
// See https://www.bluetooth.com/specifications/gatt/services/
BleUuid envMonitoringService(0x181A);

// We're using a well-known characteristics UUID. They're defined here:
// https://www.bluetooth.com/specifications/gatt/characteristics/
// The temperature-measurement is 16-bit UUID 0x2A6E
BleCharacteristic temperatureMeasurementCharacteristic("temp",
 BleCharacteristicProperty::NOTIFY, BleUuid(0x2A6E), envMonitoringService);

 // The battery level service allows the battery level to be monitored
BleUuid batteryLevelService(BLE_SIG_UUID_BATTERY_SVC);
// The battery_level characteristic shows the battery level of
BleCharacteristic batteryLevelCharacteristic("bat", BleCharacteristicProperty::NOTIFY, BleUuid(0x2A19), batteryLevelService);

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
void convert_to_gatt_format();

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
  // https://docs.particle.io/reference/device-os/bluetooth-le/
  BLE.on();
  BLE.setDeviceName("KNESL-0");
  BLE.addCharacteristic(temperatureMeasurementCharacteristic);
	BLE.addCharacteristic(batteryLevelCharacteristic);
	batteryLevelCharacteristic.setValue(&lastBattery, 1);

	BleAdvertisingData advData;
  // While we support both the health thermometer service and the battery service, we
	// only advertise the health thermometer. The battery service will be found after
	// connecting.
	advData.appendServiceUUID(envMonitoringService);
  advData.deviceName("KNESL-0", 7);

	// Continuously advertise when not connected
	BLE.advertise(&advData);

}

void loop() {
  poll_temp_c();

  if(debug){
    if (millis() - lastUpdate >= UPDATE_INTERVAL_MS)
      {
        lastUpdate = millis();

        if (BLE.connected()) {
            // The Temperature Measurement characteristic data is defined here:
            // https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.temperature_measurement.xml

            uint8_t sensor0_buf[2];
            convert_to_gatt_format(last_temp_0, sensor0_buf);

            temperatureMeasurementCharacteristic.setValue(sensor0_buf, sizeof(sensor0_buf));

            // The battery starts at 100% and drops to 10% then will jump back up again
            batteryLevelCharacteristic.setValue(&lastBattery, 1);
            if (--lastBattery < 10) {
              lastBattery = 100;
            }
        }
      }
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

void poll_temp_c(){
  bool success = sensor0.read();
  if (success) {
    last_temp_0 = sensor0.celsius();
    if(debug){
      Serial.printlnf("temperature: %f", last_temp_0);
    }
  }
}

void convert_to_gatt_format(float temperature, uint8_t* buf) {
  int16_t raw_temperature = temperature * 100;
  buf[0] = raw_temperature & 0xFF;
  buf[1] = (raw_temperature >> 8) & 0xFF;
}