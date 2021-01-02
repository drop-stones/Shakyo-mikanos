#pragma once

#include <stdint.h>

extern "C" {
  // Access to IO address space
  //  - memory address space: for main memory
  //      access directly
  //  - IO address space: for IO register
  //      access by "out" or "in" in assembly
  //  - PCI configuration space: for PCI
  //      access by CONFIG_ADDRESS and CONFIG_DATA register
  void IoOut32(uint16_t addr, uint32_t data);
  uint32_t IoIn32(uint16_t addr);
}