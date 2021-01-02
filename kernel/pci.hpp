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

  // PCI device class code
  struct ClassCode {
    uint8_t base, sub, interface;

    bool Match(uint8_t b) { return b == base; }
    bool Match(uint8_t b, uint8_t s) { return Match(b) && s == sub; };
    bool Match(uint8_t b, uint8_t s, uint8_t i) {
      return Match(b, s) && i == interface;
    }
  };

  struct Device {
    uint8_t bus, device, function, header_type;
    ClassCode class_code;
  };

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
  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

  inline uint16_t ReadVendorId(const Device& dev) {
      return ReadVendorId(dev.bus, dev.device, dev.function);
  }

  // read 32bit register
  uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);
  // write 32bit register
  void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

  // read Bus Number
  //  - 23:16 : Sub ordinate bus number
  //  - 15:8  : Secondary bus number
  //  - 7:0   : Revision number
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);

  bool IsSingleFunctionDevice(uint8_t header_type);

  // inline: identical of all translation unit
  //   useful global variable definition in header file
  inline std::array<Device, 32> devices;
  inline int num_device;

  Error ScanAllBus();

  // BAR: Base Address Register in MMIO(memory mapped io)
  constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
    return 0x10 + 4 * bar_index;
  }

  WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);
}