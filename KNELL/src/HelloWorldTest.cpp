#include "HelloWorldTest.h"

int HelloWorldTest(String cmd) {
  String devStr = "HELLO WORLD";
  String test_status = String::format("{\"DEV\":\"%s\"}", devStr.c_str());

  bool tst_success = Particle.publish("dev_events", test_status, PRIVATE, WITH_ACK);
  while(!tst_success) {
      // get here if event publish did not work, reattempt
      tst_success = Particle.publish("dev_events", test_status, PRIVATE, WITH_ACK);
    }
  return 0;
}