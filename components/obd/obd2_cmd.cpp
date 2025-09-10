#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// #include "esp_log.h"
#include "iso-tp.h"
#include "obd2.h"

static const char* TAG = "OBD2";

void OBD2::log_print(const char* format, ...) {
#ifdef OBD_DEBUG
  va_list args;
  va_start(args, format);
  esp_log_writev(ESP_LOG_INFO, TAG, format, args);
  va_end(args);
#endif
}

// Prints appropriate error description if an error has occurred
void OBD2::printError() {
  // Serial.print(F("Received: "));
  // Serial.println(payload);

  // if (nb_rx_state == OBD_SUCCESS)
  //   Serial.println(F("OBD_SUCCESS"));
  // else if (nb_rx_state == OBD_NO_RESPONSE)
  //   Serial.println(F("ERROR: OBD_NO_RESPONSE"));
  // else if (nb_rx_state == OBD_BUFFER_OVERFLOW)
  //   Serial.println(F("ERROR: OBD_BUFFER_OVERFLOW"));
  // else if (nb_rx_state == OBD_UNABLE_TO_CONNECT)
  //   Serial.println(F("ERROR: OBD_UNABLE_TO_CONNECT"));
  // else if (nb_rx_state == OBD_NO_DATA)
  //   Serial.println(F("ERROR: OBD_NO_DATA"));
  // else if (nb_rx_state == OBD_STOPPED)
  //   Serial.println(F("ERROR: OBD_STOPPED"));
  // else if (nb_rx_state == OBD_TIMEOUT)
  //   Serial.println(F("ERROR: OBD_TIMEOUT"));
  // else if (nb_rx_state == OBD_BUFFER_OVERFLOW)
  //   Serial.println(F("ERROR: BUFFER OVERFLOW"));
  // else if (nb_rx_state == OBD_GENERAL_ERROR)
  //   Serial.println(F("ERROR: OBD_GENERAL_ERROR"));
  // else
  //   Serial.println(F("No error detected"));

  // delay(100);
}

OBD2::OBD2(IsoTp* driver, uint16_t timeout) :
    iso_tp_(driver),
    timeout_ms(timeout) {}

/* Creates a query stack to be sent to OBD2

 Inputs:
 -------
  * uint16_t service - Service number of the queried PID
  * uint32_t pid     - PID number of the queried PID
  * uint8_t num_responses - see function header for "queryPID()"*/
void OBD2::formatQueryArray(const uint8_t& service,
                            const uint16_t& pid,
                            const uint8_t& num_responses) {
  log_print("Service: %d PID: %d", service, pid);

  // isMode0x22Query = (service == 0x22 &&
  //                    pid <= 0xFF);  // mode 0x22 responses always zero-pad
  //                    the
  //                                   // pid to 4 chars, even for a 2-char pid

  // query[0] = ((service >> 4) & 0xF) + '0';
  // query[1] = (service & 0xF) + '0';

  // // determine PID length (standard queries have 16-bit PIDs,
  // // but some custom queries have PIDs with 32-bit values)
  // if (pid & 0xFF00) {
  //   log_print("Long query detected");

  //   longQuery = true;

  //   query[2] = ((pid >> 12) & 0xF) + '0';
  //   query[3] = ((pid >> 8) & 0xF) + '0';
  //   query[4] = ((pid >> 4) & 0xF) + '0';
  //   query[5] = (pid & 0xF) + '0';

  //   if (specifyNumResponses) {
  //     if (num_responses > 0xF) {
  //       query[6] = ((num_responses >> 4) & 0xF) + '0';
  //       query[7] = (num_responses & 0xF) + '0';
  //       query[8] = '\0';
  //     } else {
  //       query[6] = (num_responses & 0xF) + '0';
  //       query[7] = '\0';
  //       query[8] = '\0';
  //     }
  //   } else {
  //     query[6] = '\0';
  //     query[7] = '\0';
  //     query[8] = '\0';
  //   }
  // } else {
  //   log_print("Normal length query detected");

  //   longQuery = false;

  //   query[2] = ((pid >> 4) & 0xF) + '0';
  //   query[3] = (pid & 0xF) + '0';

  //   if (specifyNumResponses) {
  //     if (num_responses > 0xF) {
  //       query[4] = ((num_responses >> 4) & 0xF) + '0';
  //       query[5] = (num_responses & 0xF) + '0';
  //       query[6] = '\0';
  //       query[7] = '\0';
  //       query[8] = '\0';
  //     } else {
  //       query[4] = (num_responses & 0xF) + '0';
  //       query[5] = '\0';
  //       query[6] = '\0';
  //       query[7] = '\0';
  //       query[8] = '\0';
  //     }
  //   } else {
  //     query[4] = '\0';
  //     query[5] = '\0';
  //     query[6] = '\0';
  //     query[7] = '\0';
  //     query[8] = '\0';
  //   }
  // }

  log_print("Query string: %s", query);
}

/*
 Description:
 ------------
  * Determines if a time-out has occurred


 Return:
 -------
  * bool - whether or not a time-out has occurred
*/
// bool OBD2::timeout() {
//   currentTime = millis();
//   if ((currentTime - previousTime) >= timeout_ms)
//     return true;
//   return false;
// }
/* converts a decimal or hex char to an int

 Inputs:
 -------
  * uint8_t value - char to be converted

 Return:
 -------
  * uint8_t - int value of parameter "value"*/
uint8_t OBD2::ctoi(uint8_t value) {
  if (value >= 'A')
    return value - 'A' + 10;
  else
    return value - '0';
}

/* Finds and returns the first char index of
  numOccur'th instance of target in str

 Inputs:
 -------
  * char const *str    - string to search target within
  * char const *target - String to search for in str
  * uint8_t numOccur   - Which instance of target in str

 Return:
 -------
  * int8_t - First char index of numOccur'th
  instance of target in str. -1 if there is no
  numOccur'th instance of target in str
*/
int8_t OBD2::nextIndex(char const* str, char const* target, uint8_t numOccur) {
  char const* p = str;
  char const* r = str;
  uint8_t count;

  for (count = 0;; ++count) {
    p = strstr(p, target);

    if (count == (numOccur - 1))
      break;

    if (!p)
      break;

    p++;
  }

  if (!p)
    return -1;

  return p - r;
}

/* Removes all instances of each char in string "remove" from the string "from"

 Inputs:
 -------
  * char *from         - String to remove target(s) from
  * char const *remove - Chars to find/remove
*/
void OBD2::removeChar(char* from, const char* remove) {
  size_t i = 0, j = 0;
  while (from[i]) {
    if (!strchr(remove, from[i]))
      from[j++] = from[i];
    i++;
  }
  from[j] = '\0';
}
/* Converts the OBD2's response into its correct, numerical value. Returns 0 if
 numExpectedBytes > numPayChars

 Inputs:
 -------
  * uint64_t response        - OBD2's response
  * uint8_t numExpectedBytes - Number of valid bytes from the response to
 process
  * double scaleFactor       - Amount to scale the response by
  * double bias               - Amount to bias the response by

 Return:
 -------
  * double - Converted numerical value
*/
double OBD2::conditionResponse(const uint8_t& numExpectedBytes,
                               const double& scaleFactor,
                               const double& bias) {
  uint8_t numExpectedPayChars = numExpectedBytes * 2;
  uint8_t payCharDiff         = numPayChars - numExpectedPayChars;

  if (numExpectedBytes > 8) {
    log_print(
        "WARNING: Number of expected response bytes is greater than 8 - "
        "returning 0");

    return 0;
  }

  if (numPayChars < numExpectedPayChars) {
    log_print(
        "WARNING: Number of payload chars is less than the number of expected "
        "response chars returned by OBD2 - returning 0");

    return 0;
  } else if (numPayChars & 0x1) {
    log_print(
        "WARNING: Number of payload chars returned by OBD2 is an odd value - "
        "returning 0");

    return 0;
  } else if (numExpectedPayChars == numPayChars) {
    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return response;
    else
      return (response * scaleFactor) + bias;
  }

  // If there were more payload bytes returned than we expected, test the first
  // and last bytes in the returned payload and see which gives us a higher
  // value. Sometimes OBD2's return leading zeros and others return trailing
  // zeros. The following approach gives us the best chance at determining where
  // the real data is. Note that if the payload returns BOTH leading and
  // trailing zeros, this will not give accurate results!

  log_print("Looking for lagging zeros");

  uint16_t numExpectedBits  = numExpectedBytes * 8;
  uint64_t laggingZerosMask = 0;

  for (uint16_t i = 0; i < numExpectedBits; i++)
    laggingZerosMask |= (1 << i);

  if (!(laggingZerosMask & response))  // Detect all lagging zeros in `response`
  {
    log_print("Lagging zeros found");

    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return (response >> (4 * payCharDiff));
    else
      return ((response >> (4 * payCharDiff)) * scaleFactor) + bias;
  } else {
    log_print("Lagging zeros not found - assuming leading zeros");

    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return response;
    else
      return (response * scaleFactor) + bias;
  }
}

/* Provides a means to pass in a user-defined function to process the response.
 Used for PIDs that don't use the common scaleFactor + Bias formula to calculate
 the value from the response data. Also useful for processing OEM custom PIDs
 which are too numerous and varied to encode in the lib.

 Inputs:
 -------
  * (*func)() - pointer to function to do calculate response value

 Return:
 -------
  * double - Converted numerical value
*/

double OBD2::conditionResponse(double (*func)()) {
  return func();
}

/* Create a PID query command string and send the command

  Inputs:
  -------
  * uint8_t service       - The diagnostic service ID. 01 is "Show current data"
  * uint16_t pid          - The Parameter ID (PID) from the service
  * uint8_t num_responses - Number of lines of data to receive - see OBD
  datasheet "Talking to the vehicle". This can speed up retrieval of information
  if you know how many responses will be sent. Basically the OBD scanner will
  not wait for more responses if it does not need to go through final timeout.
  Also prevents OBD scanners from sending mulitple of the same response.

  Return:
  -------
  * void
*/
void OBD2::queryPID(uint8_t service, uint16_t pid, uint8_t num_responses) {
  formatQueryArray(service, pid, num_responses);
  // sendCommand(query);
}

/* Queries OBD2 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * uint8_t service          - The diagnostic service ID. 01 is "Show current
 data"
  * uint16_t pid             - The Parameter ID (PID) from the service
  * uint8_t num_responses    - Number of lines of data to receive - see OBD
 datasheet "Talking to the vehicle". This can speed up retrieval of information
 if you know how many responses will be sent. Basically the OBD scanner will not
 wait for more responses if it does not need to go through final timeout. Also
 prevents OBD scanners from sending mulitple of the same response.
  * uint8_t numExpectedBytes - Number of valid bytes from the response to
 process
  * double scaleFactor        - Amount to scale the response by
  * double bias               - Amount to bias the response by

 Return:
 -------
  * double - The PID value if successfully received, else 0.0
*/
double OBD2::processPID(const uint8_t service,
                        const uint16_t pid,
                        const uint8_t num_responses,
                        const uint8_t numExpectedBytes,
                        const double scaleFactor,
                        const double bias) {
  // if (nb_query_state == SEND_COMMAND) {
  //   queryPID(service, pid, num_responses);
  //   nb_query_state = WAITING_RESP;
  // } else if (nb_query_state == WAITING_RESP) {
  //   get_response();
  //   if (nb_rx_state == OBD_SUCCESS) {
  //     nb_query_state =
  //         SEND_COMMAND;  // Reset the query state machine for next command
  //     findResponse();

  //     /* This data manipulation seems duplicative of the responseByte_0,
  //        responseByte_1, etc vars and it is. The duplcation is deliberate to
  //        provide a clear way for the calculator functions to access the
  //        relevant data bytes from the response in the format they are
  //        commonly expressed in and without breaking backward compatability
  //        with existing code that may be using the responseByte_n vars.

  //        In addition, we need to place the response values into static vars
  //        that can be accessed by the (static) calculator functions. A future
  //        (breaking!) change could be made to eliminate this duplication.
  //     */
  //     uint8_t responseBits      = numExpectedBytes * 8;
  //     uint8_t extractedBytes[8] = {0};  // Store extracted bytes

  //     // Extract bytes only if shift is non-negative
  //     for (int i = 0; i < numExpectedBytes; i++) {
  //       int shiftAmount = responseBits - (8 * (i + 1));  // Compute shift
  //       amount if (shiftAmount >= 0) {                          //  Ensure
  //       valid shift
  //         extractedBytes[i] =
  //             (response >> shiftAmount) & 0xFF;  // Extract uint8_t
  //       }
  //     }

  //     // Assign extracted values to response_A, response_B, ..., response_H
  //     // safely
  //     response_A = extractedBytes[0];
  //     response_B = extractedBytes[1];
  //     response_C = extractedBytes[2];
  //     response_D = extractedBytes[3];
  //     response_E = extractedBytes[4];
  //     response_F = extractedBytes[5];
  //     response_G = extractedBytes[6];
  //     response_H = extractedBytes[7];

  //     double (*calculator)() = selectCalculator(pid);

  //     if (nullptr == calculator) {
  //       // Use the default scaleFactor + Bias calculation
  //       return conditionResponse(numExpectedBytes, scaleFactor, bias);
  //     } else {
  //       return conditionResponse(calculator);
  //     }
  //   } else if (nb_rx_state != OBD_GETTING_MSG)
  //     nb_query_state = SEND_COMMAND;  // Error or timeout, so reset the query
  //                                     // state machine for next command
  // }
  return 0.0;
}

/* Sends a command/query for
 Non-Blocking PID queries

 Inputs:
 -------
  * const char *cmd - Command/query to send to OBD2
*/
void OBD2::sendCommand(Message* cmd) {
  // clear payload buffer
  // memset(payload, 0, sizeof(payload));

  // // reset input serial buffer and number of received bytes
  // recBytes  = 0;
  // connected = false;

  // // Reset the receive state ready to start receiving a response message
  // nb_rx_state = OBD_GETTING_MSG;

  // log_print("Sending the following command/query: %s", cmd);

  // elm_port->print(cmd);
  // elm_port->print('\r');

  // iso_tp_->send()

  //     // prime the timeout timer
  //     previousTime = millis();
  // currentTime      = previousTime;
}

/* Sends a
 command/query and waits for a respoonse (blocking function) Sometimes it's
 desirable to use a blocking command, e.g when sending an AT command. This
 function removes the need for the caller to set up a loop waiting for the
 command to finish. Caller is free to parse the payload string if they need to
 use the response.

 Inputs:
 -------
  * const char *cmd - Command/query to send to OBD2

 Return:
 -------
  * int8_t - the OBD_XXX status of getting the OBD response
*/
// int8_t OBD2::sendCommand_Blocking(const char* cmd) {
//   // sendCommand(cmd);
//   // uint32_t startTime = millis();
//   // while (get_response() == OBD_GETTING_MSG) {
//   //   if (millis() - startTime > timeout_ms)
//   //     break;
//   // }
//   // return nb_rx_state;
//   return 0;
// }

/* Non Blocking (NB) receive OBD scanner
response. Must be called repeatedly until the status progresses past
OBD_GETTING_MSG. Return:
 -------
  * int8_t - the OBD_XXX status of getting the OBD response
*/
// int8_t OBD2::get_response() {
// // buffer the response of the OBD327 until either the
// // end marker is read or a timeout has occurred
// // last valid idx is PAYLOAD_LEN but want to keep one free for terminating
// // '\0' so limit counter to < PAYLOAD_LEN
// if (!elm_port->available()) {
//   nb_rx_state = OBD_GETTING_MSG;
//   if (timeout())
//     nb_rx_state = OBD_TIMEOUT;
// } else {
//   char recChar = elm_port->read();

//   // display each received character, make non-printables printable
//   if (recChar == '\f')
//     log_print("\tReceived char: \\f");
//   else if (recChar == '\n')
//     log_print("\tReceived char: \\n");
//   else if (recChar == '\r')
//     log_print("\tReceived char: \\r");
//   else if (recChar == '\t')
//     log_print("\tReceived char: \\t");
//   else if (recChar == '\v')
//     log_print("\tReceived char: \\v");
//   // convert spaces to underscore, easier to see in debug output
//   else if (recChar == ' ')
//     log_print("\tReceived char: _");
//   // display regular printable
//   else
//     log_print("\tReceived char: %c", recChar);

//   // this is the end of the OBD response
//   if (recChar == '>') {
//     log_print("Delimiter found.");

//     nb_rx_state = OBD_MSG_RXD;
//   } else if (!isalnum(recChar) && (recChar != ':') && (recChar != '.') &&
//              (recChar != '\r'))
//     // Keep only alphanumeric, decimal, colon, CR. These are needed for
//     // response parsing decimal places needed to extract floating point
//     // numbers, e.g. battery voltage
//     nb_rx_state = OBD_GETTING_MSG;  // Discard this character
//   else {
//     if (recBytes < PAYLOAD_LEN) {
//       payload[recBytes] = recChar;
//       recBytes++;
//       nb_rx_state = OBD_GETTING_MSG;
//     } else
//       nb_rx_state = OBD_BUFFER_OVERFLOW;
//   }
// }

// // Message is still being received (or is timing out), so exit early
// // without doing all the other checks
// if (nb_rx_state == OBD_GETTING_MSG)
//   return nb_rx_state;

// // End of response delimiter was found
// if (nb_rx_state == OBD_MSG_RXD) {
//   log_print("All chars received: %s", payload);
// }

// if (nb_rx_state == OBD_TIMEOUT) {
//   log_print("Timeout detected with overflow of %ld ms",
//             (currentTime - previousTime) - timeout_ms);
//   return nb_rx_state;
// }

// if (nb_rx_state == OBD_BUFFER_OVERFLOW) {
//   log_print("OBD receive buffer overflow (> %d bytes)", PAYLOAD_LEN);
//   return nb_rx_state;
// }

// // Now we have successfully received OBD response, check if the payload
// // indicates any OBD errors
// if (nextIndex(payload, RESPONSE_UNABLE_TO_CONNECT) >= 0) {
//   log_print("OBD responded with error \"UNABLE TO CONNECT\"");

//   nb_rx_state = OBD_UNABLE_TO_CONNECT;
//   return nb_rx_state;
// }

// connected = true;

// if (nextIndex(payload, RESPONSE_NO_DATA) >= 0) {
//   log_print("OBD responded with error \"NO DATA\"");

//   nb_rx_state = OBD_NO_DATA;
//   return nb_rx_state;
// }

// if (nextIndex(payload, RESPONSE_STOPPED) >= 0) {
//   log_print("OBD responded with error \"STOPPED\"");

//   nb_rx_state = OBD_STOPPED;
//   return nb_rx_state;
// }

// if (nextIndex(payload, RESPONSE_ERROR) >= 0) {
//   log_print("OBD responded with \"ERROR\"");

//   nb_rx_state = OBD_GENERAL_ERROR;
//   return nb_rx_state;
// }

// nb_rx_state = OBD_SUCCESS;
// // Need to process multiline repsonses, remove '\r' from non multiline
// // resp
// if (NULL != strchr(payload, ':')) {
//   parseMultiLineResponse();
// } else {
//   removeChar(payload, " \r");
// }
// recBytes = strlen(payload);
// return nb_rx_state;
//   return 0;
// }

/* Parses a buffered multiline response into
 a single line with the specified data
  * Modifies the value of payload for further processing and removes the
 '\r' chars


*/
// void OBD2::parseMultiLineResponse() {
// uint8_t totalBytes    = 0;
// uint8_t bytesReceived = 0;
// char newResponse[PAYLOAD_LEN];
// memset(newResponse,
//        0,
//        PAYLOAD_LEN * sizeof(char));  // Initialize newResponse to empty
//        string
// char line[256] = "";
// char* start    = payload;
// char* end      = strchr(start, '\r');

// do {  // Step 1: Get a line from the response
//   memset(line, '\0', 256);
//   if (end != NULL) {
//     strncpy(line, start, end - start);
//     line[end - start] = '\0';
//   } else {
//     strncpy(line, start, strlen(start));
//     line[strlen(start)] = '\0';

//     // Exit when there's no more data
//     if (strlen(line) == 0)
//       break;
//   }

//   log_print("Found line in response: %s", line);
//   // Step 2: Check if this is the first line of the response
//   if (0 == totalBytes)
//   // Some devices return the response header in the first line instead of
//   // the data length, ignore this line Line containing totalBytes
//   // indicator is 3 hex chars only, longer first line will be a header.
//   {
//     if (strlen(line) > 3) {
//       log_print("Found header in response line: %s", line);
//     } else {
//       if (strlen(line) > 0) {
//         totalBytes = strtol(line, NULL, 16) * 2;
//         log_print("totalBytes = %d", totalBytes);
//       }
//     }
//   }
//   // Step 3: Process data response lines
//   else {
//     if (strchr(line, ':')) {
//       char* dataStart     = strchr(line, ':') + 1;
//       uint8_t dataLength  = strlen(dataStart);
//       uint8_t bytesToCopy = (bytesReceived + dataLength > totalBytes)
//                                 ? (totalBytes - bytesReceived)
//                                 : dataLength;
//       if (bytesReceived + bytesToCopy > PAYLOAD_LEN - 1) {
//         bytesToCopy = (PAYLOAD_LEN - 1) - bytesReceived;
//       }
//       strncat(newResponse, dataStart, bytesToCopy);
//       bytesReceived += bytesToCopy;

//       log_print("Response data: %s", dataStart);
//     }
//   }
//   if (*(end + 1) == '\0') {
//     start = NULL;
//   } else {
//     start = end + 1;
//   }
//   end = (start != NULL) ? strchr(start, '\r') : NULL;

// } while ((bytesReceived < totalBytes || 0 == totalBytes) && start != NULL);

// // Replace payload with parsed response, null-terminate after totalBytes
// int nullTermPos =
//     (totalBytes < PAYLOAD_LEN - 1) ? totalBytes : PAYLOAD_LEN - 1;
// strncpy(payload, newResponse, nullTermPos);
// payload[nullTermPos] = '\0';  // Ensure null termination
// log_print("Parsed multiline response: %s", payload);
// }

/*
 *
 Parses the buffered OBD2's response and returns the queried data

 Inputs:
 -------
  * const uint8_t& service - The diagnostic service ID. 01 is "Show current
 data"
  * const uint8_t& pid     - The Parameter ID (PID) from the service
*/
uint64_t OBD2::findResponse() {
  // uint8_t firstDatum = 0;
  // char header[7]     = {'\0'};

  // if (longQuery) {
  //   header[0] = query[0] + 4;
  //   header[1] = query[1];
  //   header[2] = query[2];
  //   header[3] = query[3];
  //   header[4] = query[4];
  //   header[5] = query[5];
  // } else {
  //   header[0] = query[0] + 4;
  //   header[1] = query[1];

  //   if (isMode0x22Query)  // mode 0x22 responses always zero-pad the pid to
  //                         // 4 chars, even for a 2-char pid
  //   {
  //     header[2] = '0';
  //     header[3] = '0';
  //     header[4] = query[2];
  //     header[5] = query[3];
  //   } else {
  //     header[2] = query[2];
  //     header[3] = query[3];
  //   }
  // }

  // log_print("Expected response header: %s", header);

  // int8_t firstHeadIndex  = nextIndex(payload, header, 1);
  // int8_t secondHeadIndex = nextIndex(payload, header, 2);

  // if (firstHeadIndex >= 0) {
  //   if (longQuery | isMode0x22Query)
  //     firstDatum = firstHeadIndex + 6;
  //   else
  //     firstDatum = firstHeadIndex + 4;

  //   // Some OBD327s (such as my own) respond with two
  //   // "responses" per query. "numPayChars" represents the
  //   // correct number of bytes returned by the OBD2
  //   // regardless of how many "responses" were returned
  //   if (secondHeadIndex >= 0) {
  //     log_print("Double response detected");

  //     numPayChars = secondHeadIndex - firstDatum;
  //   } else {
  //     log_print("Single response detected");

  //     numPayChars = strlen(payload) - firstDatum;
  //   }

  //   response = 0;
  //   for (uint8_t i = 0; i < numPayChars; i++) {
  //     uint8_t payloadIndex = firstDatum + i;
  //     uint8_t bitsOffset   = 4 * (numPayChars - i - 1);

  //     log_print("\tProcessing hex nibble: %c", payload[payloadIndex]);
  //     response =
  //         response | ((uint64_t)ctoi(payload[payloadIndex]) << bitsOffset);
  //   }

  //   // It is useful to have the response bytes
  //   // broken-out because some PID algorithms (standard
  //   // and custom) require special operations for each
  //   // uint8_t returned

  //   responseByte_0 = response & 0xFF;
  //   responseByte_1 = (response >> 8) & 0xFF;
  //   responseByte_2 = (response >> 16) & 0xFF;
  //   responseByte_3 = (response >> 24) & 0xFF;
  //   responseByte_4 = (response >> 32) & 0xFF;
  //   responseByte_5 = (response >> 40) & 0xFF;
  //   responseByte_6 = (response >> 48) & 0xFF;
  //   responseByte_7 = (response >> 56) & 0xFF;

  //   log_print("64-bit response: %llX", response);

  //   return response;
  // }

  // log_print("Response not detected");

  return 0;
}
