// Hardware test of hw::UsbDevice on CH32V307: enumeration and a bulk
// loopback on a vendor interface named "CMSIS-DAP v2". Each bulk OUT packet
// on EP 0x01 goes back unchanged on EP 0x81. See README.md in this directory.
//
// The chapter 9 code here is the minimum for enumeration on Linux. It is
// test code only: the firmware uses the libArT USB stack above the driver.
// Events are processed in the interrupt callback, which is acceptable for a
// test; a real device queues them.

#include <UsbDevice.h>
#include <cstdint>

namespace {

constexpr uint16_t VendorId = 0x1209;  // pid.codes
constexpr uint16_t ProductId = 0x0001; // pid.codes test PID, not for products
constexpr uint8_t BulkOut = 0x01;
constexpr uint8_t BulkIn = 0x81;

const uint8_t deviceDescriptor[18] = {
   18, 1, 0x00, 0x02, // bcdUSB 2.00
   0x00, 0x00, 0x00,  // class at interface level
   64,                // EP0 packet size
   VendorId & 0xFF, VendorId >> 8, ProductId & 0xFF, ProductId >> 8,
   0x00, 0x01, // bcdDevice 1.00
   1, 2, 3,    // manufacturer, product, serial strings
   1,          // one configuration
};

const uint8_t deviceQualifier[10] = { 10, 6, 0x00, 0x02, 0x00, 0x00, 0x00, 64, 1, 0 };

const char *const strings[] = { nullptr, "ch32v00x-system", "CH32V307 USBHS loopback (CMSIS-DAP)", "0001",
                                "CMSIS-DAP v2" };
constexpr uint8_t StringCount = sizeof(strings) / sizeof(strings[0]);

hw::UsbDevice usb;

// EP0 control transfer state
uint8_t ctrlData[128];
const uint8_t *ctrlNext = nullptr;
uint16_t ctrlLeft = 0;
bool ctrlZlp = false;
bool ctrlInData = false;
int16_t pendingAddress = -1;
uint8_t configuration = 0;
uint8_t loop[hw::UsbDevice::BulkMaxPacketSize];

uint16_t bulkPacketSize() {
   return usb.speed() == hw::UsbDevice::Speed::High ? 512 : 64;
}

uint16_t buildConfiguration(uint8_t type) {
   // type 2: configuration of the current speed; 7: of the other speed.
   bool high = (usb.speed() == hw::UsbDevice::Speed::High) == (type == 2);
   // Bulk packet size, little endian: 512 (0x0200) or 64 (0x0040)
   const uint8_t mpsLo = high ? 0x00 : 0x40;
   const uint8_t mpsHi = high ? 0x02 : 0x00;
   const uint8_t d[32] = {
      9, type, 32, 0, 1, 1, 0, 0x80, 50,              // configuration: 1 interface, bus powered, 100 mA
      9, 4, 0, 0, 2, 0xFF, 0x00, 0x00, 4,             // interface 0: vendor class, 2 endpoints, string 4
      7, 5, BulkOut, 2, mpsLo, mpsHi, 0,               // bulk OUT
      7, 5, BulkIn, 2, mpsLo, mpsHi, 0,                // bulk IN
   };
   for(uint16_t i = 0; i < sizeof(d); ++i) {
      ctrlData[i] = d[i];
   }
   return sizeof(d);
}

uint16_t buildString(uint8_t index) {
   if(index == 0) {
      ctrlData[0] = 4;
      ctrlData[1] = 3;
      ctrlData[2] = 0x09; // English (United States)
      ctrlData[3] = 0x04;
      return 4;
   }
   uint16_t len = 2;
   for(const char *c = strings[index]; *c != 0 && len + 2u <= sizeof(ctrlData); ++c) {
      ctrlData[len++] = static_cast<uint8_t>(*c);
      ctrlData[len++] = 0;
   }
   ctrlData[0] = static_cast<uint8_t>(len);
   ctrlData[1] = 3;
   return len;
}

void stallEp0() {
   usb.setStall(0x80, true);
   usb.setStall(0x00, true);
}

void sendNextPacket() {
   uint16_t n = ctrlLeft > hw::UsbDevice::Ep0MaxPacketSize ? hw::UsbDevice::Ep0MaxPacketSize : ctrlLeft;
   (void)usb.startIn(0x80, hw::span<const uint8_t>(ctrlNext, n));
   ctrlNext += n;
   ctrlLeft = static_cast<uint16_t>(ctrlLeft - n);
}

void replyIn(const uint8_t *data, uint16_t size, uint16_t wLength) {
   if(size > wLength) {
      size = wLength;
   }
   ctrlNext = data;
   ctrlLeft = size;
   ctrlZlp = size < wLength && size % hw::UsbDevice::Ep0MaxPacketSize == 0;
   ctrlInData = true;
   sendNextPacket();
}

void statusIn() {
   ctrlInData = false;
   (void)usb.startIn(0x80, {});
}

void setConfiguration(uint8_t value) {
   usb.closeAllEndpoints();
   configuration = value;
   if(value == 1) {
      uint16_t mps = bulkPacketSize();
      (void)usb.openEndpoint(BulkOut, hw::usb::EndpointType::Bulk, mps);
      (void)usb.openEndpoint(BulkIn, hw::usb::EndpointType::Bulk, mps);
      usb.armOut(BulkOut);
   }
}

void handleSetup(const uint8_t *s) {
   uint8_t requestType = s[0];
   uint8_t request = s[1];
   uint16_t value = static_cast<uint16_t>(s[2] | (s[3] << 8));
   uint16_t index = static_cast<uint16_t>(s[4] | (s[5] << 8));
   uint16_t length = static_cast<uint16_t>(s[6] | (s[7] << 8));
   ctrlInData = false;

   if((requestType & 0x60) != 0) { // class or vendor request
      stallEp0();
      return;
   }
   switch(request) {
   case 0: // GET_STATUS
      ctrlData[0] = ((requestType & 0x1F) == 2 && usb.isStalled(static_cast<uint8_t>(index))) ? 1 : 0;
      ctrlData[1] = 0;
      replyIn(ctrlData, 2, length);
      return;
   case 1: // CLEAR_FEATURE
   case 3: // SET_FEATURE
      if((requestType & 0x1F) == 2 && value == 0 && (index & 0x0F) != 0) { // ENDPOINT_HALT
         usb.setStall(static_cast<uint8_t>(index), request == 3);
         if(request == 1 && index == BulkOut) {
            usb.armOut(BulkOut);
         }
         statusIn();
         return;
      }
      break;
   case 5: // SET_ADDRESS: write the address after the status stage
      pendingAddress = value & 0x7F;
      statusIn();
      return;
   case 6: { // GET_DESCRIPTOR
      uint8_t type = static_cast<uint8_t>(value >> 8);
      uint8_t i = static_cast<uint8_t>(value);
      if(type == 1) {
         replyIn(deviceDescriptor, sizeof(deviceDescriptor), length);
         return;
      }
      if(type == 2 || type == 7) {
         replyIn(ctrlData, buildConfiguration(type), length);
         return;
      }
      if(type == 3 && i < StringCount) {
         replyIn(ctrlData, buildString(i), length);
         return;
      }
      if(type == 6) {
         replyIn(deviceQualifier, sizeof(deviceQualifier), length);
         return;
      }
      break;
   }
   case 8: // GET_CONFIGURATION
      ctrlData[0] = configuration;
      replyIn(ctrlData, 1, length);
      return;
   case 9: // SET_CONFIGURATION
      if(value <= 1) {
         setConfiguration(static_cast<uint8_t>(value));
         statusIn();
         return;
      }
      break;
   case 10: // GET_INTERFACE
      ctrlData[0] = 0;
      replyIn(ctrlData, 1, length);
      return;
   case 11: // SET_INTERFACE: only alternate setting 0
      if(value == 0) {
         statusIn();
         return;
      }
      break;
   default:
      break;
   }
   stallEp0();
}

void onEvent(void *, const hw::usb::Event &event) {
   switch(event.type) {
   case hw::usb::EventType::Reset:
      configuration = 0;
      pendingAddress = -1;
      ctrlInData = false;
      break;
   case hw::usb::EventType::Setup:
      handleSetup(event.setup.data());
      break;
   case hw::usb::EventType::InComplete:
      if(event.endpoint == 0x80) {
         if(ctrlInData) {
            if(ctrlLeft > 0) {
               sendNextPacket();
            } else if(ctrlZlp) {
               ctrlZlp = false;
               sendNextPacket(); // zero-length packet: the reply is shorter than wLength
            } else {
               ctrlInData = false;
               usb.armOut(0x00); // status stage
            }
         } else if(pendingAddress >= 0) {
            usb.setAddress(static_cast<uint8_t>(pendingAddress));
            pendingAddress = -1;
         }
      } else if(event.endpoint == BulkIn) {
         usb.armOut(BulkOut);
      }
      break;
   case hw::usb::EventType::Out:
      if(event.endpoint == BulkOut) {
         uint16_t n = usb.readOut(BulkOut, loop);
         if(!usb.startIn(BulkIn, hw::span<const uint8_t>(loop, n))) {
            usb.armOut(BulkOut); // IN not open: drop the packet
         }
      }
      break; // EP0: the status stage of a control read. Nothing to do.
   default:
      break;
   }
}

} // namespace

int main() {
   usb.begin(onEvent, nullptr);
   usb.connect();
   while(true) {
   }
}
