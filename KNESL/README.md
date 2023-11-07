# KNESL: Keen Nameless Electrical Sensor Lookout

A component of the Nameless lookout system started with KNELL. This component is the firmware for a particle device node that aggregates sensor data and routes to KNELL, our gateway, to communicate with the particle cloud. The purpose of this sensor data aggregation and collection is to ascertain, through data, abnormalities in conditions in the facility/ area independent of chiller operation or alert status.

## System description

The system acts as a bluetooth low energy peripheral that polls a variable number of DS18 temperature sensors. It is intended to be "cloneable" generic firmware for any board capable of BLE but was originally ran on a Boron 404x. The intention is for KNELL to get sensors values via some network (ble in this case) and act as the gateway for cloud export.

### Account credentials
None! KNELL is the component with all the accounts and tokens to concern yourself with. Pay attention to the unique UUID's we define in the document.

## Device setup/ Hardware
The included schematic should be enough to get the project going on a breadboard. Currently, all the attached sensors to KNELL operate in active mode and need a +3.3V supplied to them. The minimum pull up resistance on the data wire per sensors should be 4.7K ohms.
![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/production/KNESL/docs/KNESL.png/)

## Future work
1. Create a collection to handle variable sensor values and send them all to gateway if needed.
2. Proto out hardware schematic for real product.
