#include <cstdint>
#include <cstddef>
#include <cstdio>

#include <numeric>
#include <vector>

#include "frame_buffer_config.hpp"
#include "memory_map.hpp"
#include "graphics.hpp"
#include "mouse.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "usb/memory.hpp"
#include "usb/device.hpp"
#include "usb/classdriver/mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/xhci/trb.hpp"
#include "interrupt.hpp"
#include "asmfunc.h"
#include "queue.hpp"
#include "segment.hpp"
#include "paging.hpp"
#include "memory_manager.hpp"
#include "window.hpp"
#include "layer.hpp"
#include "menu.hpp"
//#include <io.h>

#define WALLPAPER_

#ifdef WALLPAPER_
#include "wallpaper.hpp"
char wallpaper_buf[sizeof(WallPaper)];
WallPaper* wallpaper;
#endif

//char menu_buf[sizeof(Menu)];
//Menu* menu;

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

char console_buf[sizeof(Console)];
Console* console;

int printk(const char* format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

char memory_manager_buf[sizeof(BitmapMemoryManager)];
BitmapMemoryManager* memory_manager;

unsigned int mouse_layer_id;
Vector2D<int> mouse_position;

unsigned int menu_layer_id;

void MouseObserver(uint8_t buttons, int8_t displacement_x, int8_t displacement_y) {
  static unsigned int mouse_drag_layer_id = 0;
  static uint8_t previous_buttons = 0;

  const auto oldpos = mouse_position;
  auto newpos = mouse_position + Vector2D<int>{displacement_x, displacement_y};
  // TODO: mouse must in desktop
  mouse_position = newpos;

  const auto posdiff = mouse_position - oldpos;

  //layer_manager->MoveRelative(mouse_layer_id, {displacement_x, displacement_y});
  layer_manager->Move(mouse_layer_id, mouse_position);

  const bool previous_left_pressed = (previous_buttons & 0x01);
  const bool left_pressed = (buttons & 0x01);
  static bool menu_open = false;
  if (!previous_left_pressed && left_pressed) {
    // in menu-button range
    const Vector2D<int> menu_button_pos{10, pixel_writer->Height() - 40};
    const Vector2D<int> menu_button_size{30, 30};
    if (menu_button_pos.x <= mouse_position.x && mouse_position.x <= menu_button_pos.x + menu_button_size.x
     && menu_button_pos.y <= mouse_position.y && mouse_position.y <= menu_button_pos.y + menu_button_size.y) {
       Log(kInfo, "menu_button pushed\n");
       if (menu_open) {
         layer_manager->Hide(menu_layer_id);
         menu_open = false;
       } else {
         layer_manager->UpDown(menu_layer_id, 1);
         menu_open = true;
       }
     }

     // in power_button range
     const Vector2D<int> power_button_pos{0, pixel_writer->Height() - 50 - kMenuHeight};
     const Vector2D<int> power_button_size{kMenuWidth, kMenuHeight};
     if (menu_open
      && power_button_pos.x <= mouse_position.x && mouse_position.x <= power_button_pos.x + power_button_size.x
      && power_button_pos.y <= mouse_position.y && mouse_position.y <= power_button_pos.y + power_button_size.y) {
        Log(kError, "power_button pushed\n");
        ShutdownBIOS();
      }
  }
  layer_manager->Draw();
}

void SwitchEhci2Xhci(const pci::Device& xhc_dev) {
  bool intel_ehc_exist = false;
  for (int i = 0; i < pci::num_device; i++) {
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x20u) /* EHCI */ &&
        0x8086 == pci::ReadVendorId(pci::devices[i])) {
      intel_ehc_exist = true;
      break;
    }
  }
  if (!intel_ehc_exist) {
    return;
  }

  uint32_t superspeed_ports = pci::ReadConfReg(xhc_dev, 0xdc);  // USB3PRM
  pci::WriteConfReg(xhc_dev, 0xd8, superspeed_ports);           // USB3_PSSEN
  uint32_t ehci2xhci_ports  = pci::ReadConfReg(xhc_dev, 0xd4);  // XUSB2PRM
  pci::WriteConfReg(xhc_dev, 0xd0, ehci2xhci_ports);            // XUSB2PR
  Log(kDebug, "SwitchEhci2Xhci: SS = %02, xHCI = %02x\n",
      superspeed_ports, ehci2xhci_ports);
}

usb::xhci::Controller* xhc;

struct Message {
  enum Type {
    kInterruptXHCI,
  } type;
};

ArrayQueue<Message>* main_queue;

__attribute__((interrupt))  // interrupt handler
void IntHandlerXHCI(InterruptFrame* frame) {
  main_queue->Push(Message{Message::kInterruptXHCI});
  NotifyEndOfInterrupt();
}

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(
    const FrameBufferConfig& frame_buffer_config_ref,
    const MemoryMap& memory_map_ref) {
  // copy to new stack
  FrameBufferConfig frame_buffer_config{frame_buffer_config_ref};
  MemoryMap memory_map{memory_map_ref};

  switch (frame_buffer_config.pixel_format) {
  case kPixelRGBResv8BitPerColor:
    // placement new
    pixel_writer = new(pixel_writer_buf)
      RGBResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  case kPixelBGRResv8BitPerColor:
    pixel_writer = new(pixel_writer_buf)
      BGRResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  }

  DrawDesktop(*pixel_writer);

  console = new(console_buf) Console{
    kDesktopFGColor, kDesktopBGColor
  };
  console->SetWriter(pixel_writer);
  printk("Welcome to Shakyo-MikanOS!\n");
  SetLogLevel(kWarn);
  //SetLogLevel(kDebug);

  SetupSegments();

  const uint16_t kernel_cs = 1 << 3;
  const uint16_t kernel_ss = 2 << 3;
  SetDSAll(0);
  SetCSSS(kernel_cs, kernel_ss);
  
  SetupIdentityPageTable();

  ::memory_manager = new(memory_manager_buf) BitmapMemoryManager;

  const auto memory_map_base = reinterpret_cast<uintptr_t>(memory_map.buffer);
  uintptr_t available_end = 0;
  for (uintptr_t iter = memory_map_base;
       iter < memory_map_base + memory_map.map_size;
       iter += memory_map.descriptor_size) {
    auto desc = reinterpret_cast<MemoryDescriptor*>(iter);
    if (available_end < desc->physical_start) {
      // find empty space!!
      memory_manager->MarkAllocated(
        FrameID{available_end / kBytesPerFrame},
        (desc->physical_start - available_end) / kBytesPerFrame
      );
    }

    const auto physical_end = 
      desc->physical_start + desc->number_of_pages * kUEFIPageSize;
    if (IsAvailable(static_cast<MemoryType>(desc->type))) {
      available_end = physical_end;
    } else {
      // in use
      memory_manager->MarkAllocated(
        FrameID{desc->physical_start / kBytesPerFrame},
        desc->number_of_pages * kUEFIPageSize / kBytesPerFrame
      );
    }
  }
  memory_manager->SetMemoryRange(FrameID{1}, FrameID{available_end / kBytesPerFrame});
  
  if (auto err = InitializeHeap(*memory_manager)) {
    Log(kError, "failed to allocate pages: %s at %s:%d\n",
        err.Name(), err.File(), err.Line());
    exit(1);
  }

  std::array<Message, 32> main_queue_data;
  ArrayQueue<Message> main_queue{main_queue_data};
  ::main_queue = &main_queue;   // "::" means global variable

  auto err = pci::ScanAllBus();
  Log(kDebug, "ScanAllBus: %s\n", err.Name());
  
  for (int i = 0; i < pci::num_device; i++) {
    const auto& dev = pci::devices[i];
    auto vendor_id  = pci::ReadVendorId(dev.bus, dev.device, dev.function);
    auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
    Log(kDebug, "%d.%d.%d: vend %04x, class %08x, head %02x\n",
        dev.bus, dev.device, dev.function,
        vendor_id, class_code, dev.header_type);
  }

  // Search xHC (priority Intel)
  pci::Device* xhc_dev = nullptr;
  for (int i = 0; i < pci::num_device; i++) {
    // Base class(0x0cu): Serial bus controlor
    // Sub class(0x03u) : USB controlor
    // Interface(0x30u) : xHCI
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u)) {
      xhc_dev = &pci::devices[i];

      // Intel vendorID: 0x8086
      if (0x8086 == pci::ReadVendorId(*xhc_dev)) {
        break;
      }
    }
  }
  
  if (xhc_dev) {
    Log(kInfo, "xHC has been found: %d.%d.%d\n",
        xhc_dev->bus, xhc_dev->device, xhc_dev->function);
  }

  const uint16_t cs = GetCS();
  SetIDTEntry(idt[InterruptVector::kXHCI], MakeIDTAttr(DescriptorType::kInterruptGate, 0),
              reinterpret_cast<uint64_t>(IntHandlerXHCI), cs);
  LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));

  // BSP : bootstrap processor
  // APIC: advanced programmatic interrupt controller
  // 31:24 of 0xfee00020 : Local APIC ID
  const uint8_t bsp_local_apic_id = 
    *reinterpret_cast<const uint32_t*>(0xfee00020) >> 24;
  pci::ConfigureMSIFixedDestination(
    *xhc_dev, bsp_local_apic_id,
    pci::MSITriggerMode::kLevel, pci::MSIDeliveryMode::kFixed,
    InterruptVector::kXHCI, 0
  );

  // xhc_bar: 64bit address for MMIO of xHC
  //   - MMIO: memory mapped io
  //   - BAR : base address register
  const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
  Log(kDebug, "ReadBar: %s\n", xhc_bar.error.Name());
  // Lower 4bit of BAR is flag
  const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf);
  Log(kDebug, "xHC mmio_base = %08lx\n", xhc_mmio_base);

  usb::xhci::Controller xhc{xhc_mmio_base};

  if (0x8086 == pci::ReadVendorId(*xhc_dev)) {
    SwitchEhci2Xhci(*xhc_dev);
  }
  {
    auto err = xhc.Initialize();
    Log(kDebug, "xhc.Initialize: %s\n", err.Name());
  }

  Log(kInfo, "xHC starting\n");
  xhc.Run();

  ::xhc = &xhc;

  usb::HIDMouseDriver::default_observer = MouseObserver;

  for (int i = 0; i <= xhc.MaxPorts(); i++) {
    auto port = xhc.PortAt(i);
    Log(kDebug, "Port %d: IsConnected=%d\n", i, port.IsConnected());

    if (port.IsConnected()) {
      if (auto err = ConfigurePort(xhc, port)) {
        Log(kError, "failed to configure port: %s at %s:%d\n",
            err.Name(), err.File(), err.Line());
        continue;
      }
    }
  }

  const int kFrameWidth = frame_buffer_config.horizontal_resolution;
  const int kFrameHeight = frame_buffer_config.vertical_resolution;

  auto bgwindow = std::make_shared<Window>(kFrameWidth, kFrameHeight);
  auto bgwriter = bgwindow->Writer();

  DrawDesktop(*bgwriter);
  console->SetWriter(bgwriter);

  auto mouse_window = std::make_shared<Window>(
    kMouseCursorWidth, kMouseCursorHeight);
  mouse_window->SetTransparentColor(kMouseTransparentColor);
  DrawMouseCursor(mouse_window->Writer(), {0, 0});

  // menu setting
  auto menu_window = std::make_shared<Window>(
    kMenuWidth, kMenuHeight);
  DrawMenu(menu_window->Writer(), {0, 0});
  //DrawMenu(menu_window->Writer(), {0, kFrameHeight - 100 - kMenuHeight});

  layer_manager = new LayerManager;
  layer_manager->SetWriter(pixel_writer);

  auto bglayer_id = layer_manager->NewLayer()
    .SetWindow(bgwindow)
    .Move({0, 0})
    .ID();
  mouse_layer_id = layer_manager->NewLayer()
    .SetWindow(mouse_window)
    .Move({200, 200})
    .ID();
  menu_layer_id = layer_manager->NewLayer()
    .SetWindow(menu_window)
    .Move({0, kFrameHeight - 50 - kMenuHeight})
    .ID();

  layer_manager->UpDown(bglayer_id, 0);
  layer_manager->UpDown(mouse_layer_id, 1);
  layer_manager->Hide(menu_layer_id);
  layer_manager->Draw();

  while (true) {
    // clear interrupt flag (do not accept interrupt)
    __asm__("cli");
    if (main_queue.Count() == 0) {
      // set interrupt flag (accept interrupt)
      __asm__("sti\n\thlt");
      continue;
    }

    Message msg = main_queue.Front();
    main_queue.Pop();
    __asm__("sti");

    switch (msg.type) {
      case Message::kInterruptXHCI:
        while (xhc.PrimaryEventRing()->HasFront()) {
          if (auto err = ProcessEvent(xhc)) {
            Log(kError, "Error while ProcessEvnt: %s at %s:%d\n",
                err.Name(), err.File(), err.Line());
          }
        }
        break;
      default:
        Log(kError, "Unknown message type: %d\n", msg.type);
    }
  }

  while (1) __asm__("hlt");
}

extern "C" void __cxa_pure_virtual() {
  while (1) __asm__("hlt");
}