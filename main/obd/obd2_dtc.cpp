#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"

/* Resets the stored DTCs in the ECU. This is a blocking
 function. Note: The SAE spec requires that scan tools verify that a reset
 is intended ("Are you sure?") before sending the mode 04 reset command to
 the vehicle. See p.32 of OBD2 datasheet.

 Inputs:
 -------
    * void

 Return:
 -------
  * bool - Indicates the success (or not) of the reset command.*/
bool OBD2::resetDTC() {
  if (sendCommand_Blocking("04") == OBD_SUCCESS) {
    if (strstr(payload, "44") != NULL) {
      log_print("OBD: DTC successfully reset.");

      return true;
    }
  } else {
    log_print("OBD: Resetting DTC codes failed.");
  }

  return false;
}

/* Get the list of current
 DTC codes. This method is blocking by default, but can be run in
 non-blocking mode if desired with optional boolean argument. Typical use
 involves calling the monitorStatus() function first to get the number of
 DTC current codes stored, then calling this function to retrieve those
 codes. This would  not typically be done in NB mode in a loop, but optional
 NB mode is supported.

  * To check the results of this query, inspect the DTC_Response struct:
 DTC_Response.codesFound will contain the number of codes present and
 DTC_Response.codes is an array of 5 char codes that were retrieved.

 Inputs:
 -------
  * bool isBlocking - optional arg to set (non)blocking mode - defaults to
 true / blocking mode
*/
void OBD2::currentDTCCodes(const bool& isBlocking) {
  char* idx;
  char codeType      = '\0';
  char codeNumber[5] = {0};
  char temp[6]       = {0};

  if (isBlocking)  // In blocking mode, we loop here until get_response() is
                   // past OBD_GETTING_MSG state
  {
    sendCommand("03");  // Check DTC is always Service 03 with no PID
    while (get_response() == OBD_GETTING_MSG)
      ;
  } else {
    if (nb_query_state == SEND_COMMAND) {
      sendCommand("03");
      nb_query_state = WAITING_RESP;
    }

    else if (nb_query_state == WAITING_RESP)
      get_response();
  }

  if (nb_rx_state == OBD_SUCCESS) {
    nb_query_state =
        SEND_COMMAND;  // Reset the query state machine for next command
    memset(DTC_Response.codes, 0, DTC_CODE_LEN * DTC_MAX_CODES);

    if (strstr(payload, "43") !=
        NULL)  // Successful response to Mode 03 request
    {
      // OBD scanner will provide a response that contains one or more lines
      // indicating the codes present. Each response line will start with
      // "43" indicating it is a response to a Mode 03 request. See p. 31 of
      // OBD2 datasheet for details and lookup table of code types.

      uint8_t codesFound =
          strlen(payload) /
          8;  // Each code found returns 8 chars starting with "43"
      idx = strstr(payload, "43") + 4;  // Pointer to first DTC code digit
                                        // (third char in the response)

      if (codesFound >
          DTC_MAX_CODES)  // I don't think the OBD is capable of returning
      {                   // more than 0xF (16) codes, but just in case...
        codesFound = DTC_MAX_CODES;
        log_print("DTC response truncated at %d codes.", DTC_MAX_CODES);
      }

      DTC_Response.codesFound = codesFound;

      for (int i = 0; i < codesFound; i++) {
        memset(temp, 0, sizeof(temp));
        memset(codeNumber, 0, sizeof(codeNumber));

        codeType      = *idx;        // Get first digit of second uint8_t
        codeNumber[0] = *(idx + 1);  // Get second digit of second uint8_t
        codeNumber[1] = *(idx + 2);  // Get first digit of third uint8_t
        codeNumber[2] = *(idx + 3);  // Get second digit of third uint8_t

        switch (codeType)  // Set the correct type prefix for the code
        {
          case '0':
            strcat(temp, "P0");
            break;

          case '1':
            strcat(temp, "P1");
            break;

          case '2':
            strcat(temp, "P2");
            break;
          case '3':
            strcat(temp, "P3");
            break;

          case '4':
            strcat(temp, "C0");
            break;

          case '5':
            strcat(temp, "C1");
            break;

          case '6':
            strcat(temp, "C2");
            break;

          case '7':
            strcat(temp, "C3");
            break;

          case '8':
            strcat(temp, "B0");
            break;

          case '9':
            strcat(temp, "B1");
            break;

          case 'A':
            strcat(temp, "B2");
            break;

          case 'B':
            strcat(temp, "B3");
            break;

          case 'C':
            strcat(temp, "U0");
            break;

          case 'D':
            strcat(temp, "U1");
            break;

          case 'E':
            strcat(temp, "U2");
            break;

          case 'F':
            strcat(temp, "U3");
            break;

          default:
            break;
        }

        strcat(temp, codeNumber);  // Append the code number to the prefix
        strcpy(DTC_Response.codes[i],
               temp);   // Add the fully parsed code to the list (array)
        idx = idx + 8;  // reset idx to start of next code

        log_print("OBD: Found code: %s", temp);
      }
    } else {
      log_print("OBD: DTC response received with no valid data.");
    }
    return;
  } else if (nb_rx_state != OBD_GETTING_MSG) {
    nb_query_state = SEND_COMMAND;  // Error or timeout, so reset the query
                                    // state machine for next command

    log_print("OBD: Getting current DTC codes failed.");
    printError();
  }
}
