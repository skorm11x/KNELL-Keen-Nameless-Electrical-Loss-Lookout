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

#include "DiagnosticsHelperRK.h"
#include <stdint.h>
// #include "assert.h"
#include "SystemDefinitions.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

particle::Future<bool> publish(const char *name, const char *data);
FuelGauge fuel;
SerialLogHandler logHandler;
uint64_t lastSync;
String powerSourceStr;
String status;
String weatherApiData;
String periodFields[9];
int periodFieldIndex = 0;
bool windResult = false;

void get_battery_voltage();
void check_day_time_sync();
void detect_power_state();
void publish_power_loss();
void publish_power_restored();
void get_weather_handler(const char *event, const char *data);
bool high_winds(String);
void post_weather(String);
void get_weather();
void setup();
void loop();

void setup() {
    Particle.variable("battery_voltage", battery_voltage);
    Particle.subscribe("hook-response/get_weather", get_weather_handler, MY_DEVICES);

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
    get_weather();
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
    if (powerSource == initPowerSource && lastPowerSource != powerSource && lastPowerSource != -1) {
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

/*
  Function get weather conditions
  Note: Epoch time 6 hours a head - 8:00 local --> 14:00 Epoch
*/
void get_weather() {
    // Log.info("Time is: %i:%i:%i:", Time.hour(), Time.minute(), Time.second());
    if (Time.hour() == 15 && Time.minute() == 0 && Time.second() < 5) {
        success = Particle.publish("get_weather", PRIVATE, WITH_ACK);

        while (!success) {
            success = Particle.publish("get_weather", PRIVATE, WITH_ACK);
        }
    }
}

/*
  Function to process respond from get_weather weebhook
*/
void get_weather_handler(const char *event, const char *data) {
    // display the webhook response from get_weather call. Note Log class messages outputs only 200 characters
    // Log.info("get_weather_handler response: %d", (int)Time.now());
    // Log.info("event=%s data=%s", event, data);

    weatherApiData.concat(data);

    // check for ]} at the end of weatherApiData, this will tell us respose from cloud is complete
    if (weatherApiData.charAt(weatherApiData.length() - 2) == ']' && weatherApiData.charAt(weatherApiData.length() - 1) == '}') {
        JSONValue outerObj = JSONValue::parseCopy(weatherApiData);
        JSONObjectIterator outerIter(outerObj);

        while (outerIter.next()) {
            if (outerIter.name() == "periods") {
                JSONArrayIterator entriesIter(outerIter.value());
                while (entriesIter.next()) {
                    JSONObjectIterator entryIter(entriesIter.value());
                    while (entryIter.next()) {
                        // save value in periodFields temp array
                        periodFields[periodFieldIndex] = (const char *)entryIter.value().toString();
                        // Log.info("entry key=%s value=%s", (const char *)entryIter.name(), (const char *)entryIter.value().toString());
                        // Log.info("periodFieldIndex: %i", periodFieldIndex);

                        // check for wind condition
                        if (entryIter.name() == "ws") {
                            windResult = high_winds((const char *)entryIter.value().toString());
                        }

                        // check for end of period
                        if (entryIter.name() == "df") {
                            periodFieldIndex = 0;

                            // check if we should post period
                            if (windResult) {
                                // testing line break. \n and char(10) break pushover notification.
                                // <br> will work and must add html 1 on query parameter
                                String periodData = "WEATHER REPORT: " + periodFields[1] + " <br> " +
                                                    "Start Time: " + periodFields[2] + " <br> " +
                                                    "End Time: " + periodFields[3] + " <br> " +
                                                    "Temperature: " + periodFields[4] + " <br> " +
                                                    "Temperature unit: " + periodFields[5] + " <br> " +
                                                    "Wind Speed: " + periodFields[6] + " <br> " +
                                                    "Wind Direction: " + periodFields[7] + " <br> " +
                                                    "Detailed Forecast: " + periodFields[8] + " <br> ";
                                post_weather(periodData);
                            }
                        } else {
                            periodFieldIndex++;
                        }
                    }
                }
            }
        }

        // all data is recieved at this point, we should have processed all 6 periods
        weatherApiData = "";
    }
}

/*
  Function will check for high wind condition
*/
bool high_winds(String windRange) {
    bool returnValue = false;
    // extract low high values from winRanage array
    String lowHigh[2];
    String _windRange = windRange;
    _windRange.replace(" to ", " ");
    _windRange.replace("mph", "");
    size_t winRangeSize = _windRange.length() - 1;
    int lowHighCounter = 0;

    // retrive two wind speeds to lowHigh array
    for (size_t i = 0; i < winRangeSize; i++) {
        if (_windRange.charAt(i) != ' ')
            lowHigh[lowHighCounter] = lowHigh[lowHighCounter] + _windRange.charAt(i);
        else {
            lowHighCounter = 1;
        }
    }

    // Log.info("lowHigh[0]: %s", (const char *)lowHigh[0]);
    // Log.info("lowHigh[1]: %s", (const char *)lowHigh[1]);

    // we have a low and high values, check against highwinds array
    for (size_t i = 0; i < 2; i++) {
        for (size_t ii = 0; ii < sizeof(highwinds) / sizeof(highwinds[0]); ii++) {
            // look for value in highwinds array
            if (highwinds[ii] == lowHigh[i]) {
                returnValue = true;
                break;
            }
        }
    }

    return returnValue;
}

/*
  Function to publish weather condition to pushover
*/
void post_weather(String period) {
    // String mTxt = "Hi from boron <br> Second line";
    String message = String::format("{\"msg\":\"%s\"}", period.c_str());

    success = Particle.publish("post_weather", message, PRIVATE, WITH_ACK);

    while (!success) {
        success = Particle.publish("post_weather", message, PRIVATE, WITH_ACK);
    }

    // wait 1 sec. before sending next call to pushover cloud, experiencing periods out of order.
    delay(1000);
}
