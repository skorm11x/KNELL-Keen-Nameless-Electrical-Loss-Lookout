#ifndef KNELL_SRC_SYSTEMDEFINITIONS_H_
#define KNELL_SRC_SYSTEMDEFINITIONS_H_

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

/*
  Global variables, including class definitions
*/
float cell_sig_str;  // celluar signal strength
float cell_sig_qual;  // celluar signal quality
double battery_voltage;  // voltage in volts, returns -1.0 if it cannot be read
int isOnWallPower;
int initPowerSource = 3;  // this should be USB/ wall power for our design
// char *cell_private_ip; //cellular connection private ip, TBD
int powerSource;
int lastPowerSource = -1;
float receivedTemp = 0.0;
const char* DEVICE_NAME = "KNESL-0";
String highwinds[7] = {"45", "50", "55", "60", "65", "70", "75"};  // Array containing the high wind values

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

char powerLossStr[255] {"POWER LOSS AT DMOC"};
char powerRestoredStr[255] {"POWER RESTORED AT DMOC"};
int success;
int loopDelayTime = 5000;

#endif  // KNELL_SRC_SYSTEMDEFINITIONS_H_
