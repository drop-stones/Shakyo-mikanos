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

  // Get code segment selector
  uint16_t GetCS(void);
  // register user-defined IDT
  void LoadIDT(uint16_t limit, uint64_t offset);
  // register user-defined GDT
  void LoadGDT(uint16_t limit, uint64_t offset);
  // setting ss(stack segment register) + cs(code segment register)
  void SetCSSS(uint16_t cs, uint16_t ss);
  // setting segment registers (ds, es, fs, gs)
  void SetDSAll(uint16_t value);
  // setting cr3 register
  void SetCR3(uint64_t value);

  // BIOS Shutdown (https://wiki.osdev.org/Shutdown#Emulator-specific_methods)
  void ShutdownBIOS();
  // APM Shutdown (https://stackoverflow.com/questions/678458/shutdown-the-computer-using-assembly)
  void ShutdownAPM();
  // Shutdown QEMU
  void ShutdownQEMU();
}