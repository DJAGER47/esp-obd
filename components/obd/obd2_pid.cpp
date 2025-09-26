#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

// Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)
// std::optional<uint32_t> - Bit encoded booleans of supported PIDs 0x1-0x20
std::optional<uint32_t> OBD2::supportedPIDs_1_20() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_1_20, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  };
  return {};
}

// Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)
// std::optional<uint32_t> - Bit encoded booleans of supported PIDs 0x21-0x20
std::optional<uint32_t> OBD2::supportedPIDs_21_40() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_21_40, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

// Determine which of PIDs 0x41 through 0x60 are supported (bit encoded)
// std::optional<uint32_t> - Bit encoded booleans of supported PIDs 0x41-0x60
std::optional<uint32_t> OBD2::supportedPIDs_41_60() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_41_60, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

// Determine which of PIDs 0x61 through 0x80 are supported (bit encoded)
// std::optional<uint32_t> - Bit encoded booleans of supported PIDs 0x61-0x80
std::optional<uint32_t> OBD2::supportedPIDs_61_80() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_61_80, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/* Checks if a particular PID is
   supported by the connected ECU.

    * This is a convenience method that selects the correct
   supportedPIDS_xx_xx() query and parses the bit-encoded result, returning a
   simple Boolean value indicating PID support from the ECU.

   Inputs:
   -------
    * uint8_t pid - the PID to check for support.

   Return:
   -------
    * bool - Whether or not the queried PID is supported by the ECU.*/
bool OBD2::isPidSupported(uint8_t pid) {
  const uint8_t pidInterval = (pid / PID_INTERVAL_OFFSET) * PID_INTERVAL_OFFSET;
  std::optional<uint32_t> supportedPids;

  switch (pidInterval) {
    case SUPPORTED_PIDS_1_20:
      supportedPids = supportedPIDs_1_20();
      break;

    case SUPPORTED_PIDS_21_40:
      supportedPids = supportedPIDs_21_40();
      pid           = (pid - SUPPORTED_PIDS_21_40);
      break;

    case SUPPORTED_PIDS_41_60:
      supportedPids = supportedPIDs_41_60();
      pid           = (pid - SUPPORTED_PIDS_41_60);
      break;

    case SUPPORTED_PIDS_61_80:
      supportedPids = supportedPIDs_61_80();
      pid           = (pid - SUPPORTED_PIDS_61_80);
      break;

    default:
      return false;
  }

  if (supportedPids.has_value()) {
    // Bit position calculation: PID 1 is bit 31, PID 2 is bit 30, etc.
    uint8_t bitPosition = 32 - pid;
    return ((supportedPids.value() >> bitPosition) & 0x1) != 0;
  }
  return false;
}
