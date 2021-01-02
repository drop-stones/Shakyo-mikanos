// p.124

#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
  // CONFIG_ADDRESS register address
  const uint16_t kConfigAddress = 0x0cf8;
  // CONFIG_DATA register address
  const uint16_t kConfigData = 0xcfc;

  // write to CONFIG_ADDRESS
  void WriteAddress(uint32_t address);
  // write to CONFIG_DATA
  void WriteData(uint32_t value);
  // read from CONFIG_DATA
  uint32_t ReadData();

  // read Vendor ID
  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
  // read Device ID
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
  // read Header Type
  uint8_t  ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
  // read Class code
  //  - 31:24 : Base Class
  //  - 23:16 : Sub Class
  //  - 15:8  : Interface
  //  - 0:7   : Revision ID
  uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

  // read Bus Number
  //  - 23:16 : Sub ordinate bus number
  //  - 15:8  : Secondary bus number
  //  - 7:0   : Revision number
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);

  bool IsSingleFunctionDevice(uint8_t header_type);

  struct Device {
    uint8_t bus, device, function, header_type;
  };

  // inline: identical of all translation unit
  //   useful global variable definition in header file
  inline std::array<Device, 32> devices;
  inline int num_device;

  Error ScanAllBus();
}