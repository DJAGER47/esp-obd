#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"

/* Read Vehicle Identification Number
   (VIN). This is a blocking function.

   Inputs:
   -------
      * char vin[] - pointer to c-string in which to store VIN
          Note: (allocate memory for 18 character c-string in calling
   function)

   Return:
   -------
    * int8_t - the OBD_XXX status of getting the VIN
  */
int8_t OBD2::get_vin_blocking(char vin[]) {
  char temp[3] = {0};
  char* idx;
  uint8_t vin_counter = 0;
  uint8_t ascii_val;

  log_print("Getting VIN...");

  sendCommand("0902");  // VIN is command 0902
  while (get_response() == OBD_GETTING_MSG)
    ;

  // strcpy(payload, "0140:4902013144341:475030305235352:42313233343536");
  if (nb_rx_state == OBD_SUCCESS) {
    memset(vin, 0, 18);
    // **** Decoding ****
    if (strstr(payload, "490201")) {
      // OBD scanner provides this multiline response:
      // 014                        ==> 0x14 = 20 bytes following
      // 0: 49 02 01 31 44 34       ==> 49 02 = Header. 01 = 1 VIN number in
      // message. 31, 44, 34 = First 3 VIN digits 1: 47 50 30 30 52 35 35
      // ==> 47->35 next 7 VIN digits 2: 42 31 32 33 34 35 36    ==> 42->36
      // next 7 VIN digits
      //
      // The resulitng payload buffer is:
      // "0140:4902013144341:475030305235352:42313233343536" ==>
      // VIN="1D4GP00R55B123456" (17-digits)
      idx = strstr(payload, "490201") +
            6;  // Pointer to first ASCII code digit of first VIN digit
      // Loop over each pair of ASCII code digits. 17 VIN digits + 2 skipped
      // line numbers = 19 loops
      for (int i = 0; i < (19 * 2); i += 2) {
        temp[0] = *(idx + i);      // Get first digit of ASCII code
        temp[1] = *(idx + i + 1);  // Get second digit of ASCII code
        // No need to add string termination, temp[3] always == 0

        if (strstr(temp, ":"))
          continue;  // Skip the second "1:" and third "2:" line numbers

        ascii_val = strtol(temp, 0, 16);  // Convert ASCII code to integer
        snprintf(vin + vin_counter++,
                 sizeof(uint8_t),
                 "%c",
                 ascii_val);  // Convert ASCII code integer back to character
                              // Serial.printf("Chars %s, ascii_val=%d[dec]
                              // 0x%02hhx[hex] ==> VIN=%s\n", temp, ascii_val,
                              // ascii_val, vin);
      }
    }
    log_print("VIN: %s", vin);
  } else {
    log_print("No VIN response");
    printError();
  }
  return nb_rx_state;
}
