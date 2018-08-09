
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PciLib.h>

#define MAX_BUS_NUM 255
#define MAX_DEV_NUM 31
#define MAX_FUNC_NUM 7

EFI_STATUS
EFIAPI
PciScanMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT16   Bus;
  UINT8    Dev;
  UINT8    Func;
  UINT16   VendorId, DeviceId;

  Print (L"Bus  Dev  Func  VendorId  DeviceId\n");

  for (Bus = 0; Bus <= MAX_BUS_NUM; Bus++) {
    for (Dev = 0; Dev <= MAX_DEV_NUM; Dev++) {
      for (Func = 0; Func <= MAX_FUNC_NUM; Func++) {

        VendorId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0));
        DeviceId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 2));

        if (VendorId == 0xffff) {
          continue;
        }
        Print (L" %02x   %02x    %02x      %04x      %04x\n", Bus, Dev, Func, VendorId, DeviceId);
      }
    }
  }

  return EFI_SUCCESS;
}
