
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PciLib.h>

#define MAX_BUS_NUM 255
#define MAX_DEV_NUM 31
#define MAX_FUNC_NUM 7

#define XHCI_BASE_CLASS 0x0c
#define XHCI_SUB_CLASS  0x03
#define XHCI_INTERFACE  0x30

EFI_STATUS
EFIAPI
XhciViewerMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT16   Bus;
  UINT8    Dev;
  UINT8    Func;
  UINT16   VendorId, DeviceId;
  UINT8    BaseClass, SubClass, Interface;
  UINT8    XhcNum;

  XhcNum = 0;

  Print (L"  Bus  Dev  Func  VendorId  DeviceId\n");

  for (Bus = 0; Bus <= MAX_BUS_NUM; Bus++) {
    for (Dev = 0; Dev <= MAX_DEV_NUM; Dev++) {
      for (Func = 0; Func <= MAX_FUNC_NUM; Func++) {

        VendorId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0));
        DeviceId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 2));

        if (VendorId == 0xffff) {
          continue;
        }

        BaseClass = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0xb));
        SubClass  = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0xa));
        Interface = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x9));

        if ((BaseClass == XHCI_BASE_CLASS) && (SubClass == XHCI_SUB_CLASS) && (Interface == XHCI_INTERFACE)) {
          XhcNum++;
          Print (L"%x. %02x   %02x    %02x      %04x      %04x\n", XhcNum, Bus, Dev, Func, VendorId, DeviceId);
        }

      }
    }
  }

  return EFI_SUCCESS;
}
