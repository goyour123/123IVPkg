
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PciLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#define MAX_BUS_NUM 255
#define MAX_DEV_NUM 31
#define MAX_FUNC_NUM 7

#define XHCI_BASE_CLASS 0x0c
#define XHCI_SUB_CLASS  0x03
#define XHCI_INTERFACE  0x30

typedef struct {
  UINT8  Bus;
  UINT8  Device;
  UINT8  Function;
  UINT16 VendorId;
  UINT16 DeviceId;
} PCI_CONFIG;

typedef struct {
  PCI_CONFIG      *PciConfigBuf;
  UINT8           XhcNum;
} USB3_CONTROLLER;

EFI_STATUS
WaitForKeyStroke (
  OUT  EFI_INPUT_KEY           *Key
  );

USB3_CONTROLLER*
ScanXhc (
);

EFI_STATUS
EFIAPI
XhciViewerMain (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
{
  EFI_STATUS                   Status;
  USB3_CONTROLLER              *Xhc;
  EFI_INPUT_KEY                Key;
  UINT8                        Index;

  Xhc = ScanXhc ();
  gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

  while (Key.UnicodeChar != 'Q' && Key.UnicodeChar != 'q') {
    gST->ConOut->ClearScreen(gST->ConOut);
    Print (L"Index  Bus  Dev  Func  VendorId  DeviceId\n");
    for (Index = 0; Index < Xhc->XhcNum; Index++) {
      Print (L"    %x   %02x   %02x    %02x      %04x      %04x\n",
        Index, 
        (Xhc->PciConfigBuf + Index * sizeof(PCI_CONFIG))->Bus, 
        (Xhc->PciConfigBuf + Index * sizeof(PCI_CONFIG))->Device, 
        (Xhc->PciConfigBuf + Index * sizeof(PCI_CONFIG))->Function, 
        (Xhc->PciConfigBuf + Index * sizeof(PCI_CONFIG))->VendorId, 
        (Xhc->PciConfigBuf + Index * sizeof(PCI_CONFIG))->DeviceId
        );
    }
    Status = WaitForKeyStroke (&Key);
  }

  gST->ConOut->ClearScreen(gST->ConOut);
  return EFI_SUCCESS;
}

/**
  Wait for a key to be pressed by user.

  @param Key         The key which is pressed by user.

  @retval EFI_SUCCESS The function always completed successfully.

**/
EFI_STATUS
WaitForKeyStroke (
  OUT  EFI_INPUT_KEY           *Key
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  while (TRUE) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
    if (!EFI_ERROR (Status)) {
      break;
    }

    if (Status != EFI_NOT_READY) {
      continue;
    }
    
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
  }
  return Status;
}

USB3_CONTROLLER*
ScanXhc (
)
{
  UINT16                       Bus;
  UINT8                        Dev;
  UINT8                        Func;
  UINT16                       VendorId, DeviceId;
  UINT8                        BaseClass, SubClass, Interface;
  USB3_CONTROLLER              *Xhc;

  Xhc = AllocateZeroPool (sizeof (USB3_CONTROLLER));
  Xhc->PciConfigBuf = AllocateZeroPool (sizeof (PCI_CONFIG));

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
          Xhc->XhcNum++;
          Xhc->PciConfigBuf = ReallocatePool ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG), Xhc->XhcNum * sizeof (PCI_CONFIG), Xhc->PciConfigBuf);
          (Xhc->PciConfigBuf + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG)))->Bus      = (Bus & 0xff);
          (Xhc->PciConfigBuf + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG)))->Device   = Dev;
          (Xhc->PciConfigBuf + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG)))->Function = Func;
          (Xhc->PciConfigBuf + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG)))->VendorId = VendorId;
          (Xhc->PciConfigBuf + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG)))->DeviceId = DeviceId;
        }
      }
    }
  }

  return Xhc;
}
