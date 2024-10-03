/*
 * Project 705-power-loss-detection (KNELL)
 * Keen Nameless Electronic Loss Lookout
 * Description: Detect building 942 power loss (on mains)
 * Author: Christopher J. Kosik
 * Date: 6 November 2023
 */
// #define DEV_DEBUG
// #define POWER_DEBUG
// #define CELLULAR_DEBUG

#include <stdint.h>
#include "DiagnosticsHelperRK.h"
// #include "assert.h"
#include "SystemDefinitions.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

particle::Future<bool> publish(const char* name, const char* data);
FuelGauge fuel;
SerialLogHandler logHandler;
uint64_t lastSync;
String powerSourceStr;
String status;

void get_battery_voltage();
void check_day_time_sync();
void detect_power_state();
void publish_power_loss();
void publish_power_restored();
void setup();
void loop();


void setup() {
  Particle.variable("battery_voltage", battery_voltage);

  if (Cellular.ready()) {
      CellularSignal sig = Cellular.RSSI();
      cell_sig_str = sig.getStrength();
      cell_sig_qual = sig.getQuality();
      #ifdef CELLULAR_DEBUG
        Log.info("Cellular ready at startup: %f strength and %f quality",
          cell_sig_str, cell_sig_qual);
        Log.info("localIP: %s", Cellular.localIP().toString().c_str());
        Serial.begin(9600);
      #endif
      lastSync = millis();
      Serial.begin(9600);
    }
}


void loop() {
  get_battery_voltage();
  check_day_time_sync();
  detect_power_state();
  delay(loopDelayTime);
}

/*
  Reads battery voltage and updates stack value
*/
void get_battery_voltage() {
  battery_voltage = fuel.getVCell();
}

/*
  Checked at top of every loop, sync time with cloud at least daily.
*/
void check_day_time_sync() {
  if (millis() - lastSync > ONE_DAY_MILLIS) {
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
    if (powerSource != initPowerSource && powerSource != lastPowerSource) {
        #ifdef POWER_DEBUG
          Log.info("Potential Power source change: %s", powerSourceStr.c_str());
        #endif
        delay(loopDelayTime);
        powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
        if (powerSource != initPowerSource) {
          #ifdef POWER_DEBUG
            Log.info("Confirmed Power source change: %s",
              powerSourceStr.c_str());
          #endif
          if (powerSource == 5) {
            publish_power_loss();
          }
          if (powerSource == 1) {
            publish_power_restored();
          }
          if (powerSource == 2 || powerSource == 3 || powerSource == 4) {
            publish_power_restored();
          }
        }
    }
    if (powerSource == initPowerSource && lastPowerSource
        != powerSource && lastPowerSource != -1) {
      #ifdef POWER_DEBUG
        Log.info("Potential Power source change: %s", powerSourceStr.c_str());
      #endif
      delay(loopDelayTime);
      powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);

      if (powerSource == initPowerSource) {
        publish_power_restored();
      }
  }
}

/*
  Function publishes power loss message to the particle
  cloud. We keep trying until successful ACK
*/
void publish_power_loss() {
  status = String::format("{\"powerSource\":\"%s\"}", powerLossStr);
  success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  lastPowerSource = powerSource;
  while (!success) {
    success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  }
}

/*
  Function publishes power restored message to the particle
  cloud. We keep trying until successful ACK
*/
void publish_power_restored() {
  status = String::format("{\"powerSource\":\"%s\"}", powerRestoredStr);
  success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  while (!success) {
    success = Particle.publish("power_change", status, PRIVATE, WITH_ACK);
  }
  lastPowerSource = powerSource;
}
