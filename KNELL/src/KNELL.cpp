/*
 * Project 705-power-loss-detection (KNELL)
 * Keen Nameless Electronic Loss Lookout
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 6 November 2023
 */

#include "DiagnosticsHelperRK.h"
#include "assert.h"
#include "SystemDefinitions.h"
#include "HelloWorldTest.h"

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
unsigned long lastSync = millis();
String powerSourceStr;
String status;

/*
  BLE definitions
*/
// TODO: integrate custom KNELL/KNESL service to look for and
// only connect to those devices.
// BlePeerDevice peer;
// const BleUuid envMonitoringService(0x181A);
// const BleUuid tempUUID(0x2A6E);
// BleCharacteristic temperatureMeasurementCharacteristic("temp",
//  BleCharacteristicProperty::NOTIFY, BleUuid(0x2A6E), envMonitoringService);
// BleCharacteristic peerTempCharacteristic;
// BleCharacteristic peerBattCharacteristic;
// const BleAddress DEVICE_ADDRESS("F8:A2:E9:EF:04:65");

/*
  Function prototypes for C pre-processor
*/
void get_battery_voltage();
void check_day_time_sync();
void detect_power_state();
void publish_power_loss();
void publish_power_restored();
// void onDataReceived(BleOnDataReceivedCallback callback, void* context);


void setup() {
  Particle.variable("battery_voltage", battery_voltage);
  Particle.function("HelloWorldTest", HelloWorldTest);

  if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      #ifdef CELLULAR_DEBUG
        // Prints out the cellular 
        Log.info("Cellular ready at startup: %f strength and %f quality", cell_sig_str, cell_sig_qual);
        // Prints out the local (private) IP over Serial
        Log.info("localIP: %s", Cellular.localIP().toString().c_str());
        Serial.begin(9600);
      #endif
    }
    // BLE.on();
    // BLE.setDeviceName("KNELL");
    // // temperatureMeasurementCharacteristic.onDataReceived(onDataReceived, NULL);
    // if(!peer.connected()){
    //   Vector<BleScanResult> scanResult = BLE.scan();
    //     if (scanResult.size()) {
    //       #ifdef BLE_DEBUG
    //         Log.info("%d devices found", scanResult.size());
    //       #endif
    //       for (int ii = 0; ii < scanResult.size(); ii++) {
    //         if (scanResult[ii].address().toString() == DEVICE_ADDRESS.toString()) {
    //           #ifdef BLE_DEBUG
    //             Serial.println("address found.");
    //           #endif
    //           peer = BLE.connect(scanResult[ii].address());
    //           if (peer.connected()) {
    //             #ifdef BLE_DEBUG
    //               Serial.println("CONNECTED.");
    //             #endif
    //             peer.getCharacteristicByUUID(peerTempCharacteristic, tempUUID);
    //             peerTempCharacteristic.onDataReceived(onDataReceived);
    //           }
    //         }
    //       }
    //     }
    // }
}


void loop() {
  check_day_time_sync();
  detect_power_state();
  delay(loopDelayTime);
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
    //Serial.println("power source str: %s", powerSourceStr.c_str());
  #endif
    if (powerSource != initPowerSource && powerSource != lastPowerSource) {
        #ifdef POWER_DEBUG
          Log.info("Potential Power source change: %s", powerSourceStr.c_str());
        #endif
        // we need to wait and do a re-read of the power source 
        delay(loopDelayTime);
        powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
        if(powerSource != initPowerSource) {
          #ifdef POWER_DEBUG
            Log.info("Confirmed Power source change: %s", powerSourceStr.c_str());
          #endif
          if(powerSource == 5) {
            publish_power_loss();
          }
          if(powerSource == 1) {
            publish_power_restored();
          }
          if(powerSource == 2 || powerSource == 3 || powerSource == 4) {
            #ifdef POWER_DEBUG
              Serial.println("State error! Experiment with cases.")
            #endif
            publish_power_restored();
          }
        }
    }
    if(powerSource == initPowerSource && lastPowerSource != powerSource && lastPowerSource != -1) {
      #ifdef POWER_DEBUG
        Log.info("Potential Power source change: %s", powerSourceStr.c_str());
      #endif 
      delay(loopDelayTime);
      powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);

      if(powerSource == initPowerSource) {
        publish_power_restored();
      }
  }
}

/*
  Function publishes power loss message to the particle
  cloud. We keep trying until successful ACK
*/
void publish_power_loss(){
  get_battery_voltage();
  status = String::format("{\"powerSource\":\"%s\"}", powerLossStr);
  success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  lastPowerSource = powerSource;
  while(!success) {
    success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  }
}

/*
  Function publishes power restored message to the particle
  cloud. We keep trying until successful ACK
*/
void publish_power_restored(){
  status = String::format("{\"powerSource\":\"%s\"}", powerRestoredStr);
  success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  while(!success) {
    success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  }
  lastPowerSource = powerSource;
}

/*
  Call back function to handle KNESL temperature value on the
  GATT env monitoring service.
*/
// void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
// {
//   if (len == 2)
//   {
//     int16_t rawValue = (data[1] << 8) | data[0];
//     float receivedTemp = rawValue / 100.0;
//     Serial.print("Received Temp: ");
//     Serial.println(receivedTemp);
//   }
// }
