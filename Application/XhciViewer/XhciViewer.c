
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PciLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UserInterfaceLib.h>

#define MAX_BUS_NUM  255
#define MAX_DEV_NUM  31
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
  UINT32 BaseAddress[2];
} PCI_CONFIG;

typedef struct {
  PCI_CONFIG      *PciConfigBuf;
  UINT8           XhcNum;
} USB3_CONTROLLER;

USB3_CONTROLLER*
ScanXhc (
);

VOID
FrontPage (
);

// VOID
// Selector (
//   UINTN                        Column,
//   UINTN                        Row
// );

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
  UINT8                        Cntlr;

  Xhc = ScanXhc ();
  gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

  while (Key.UnicodeChar != 'Q' && Key.UnicodeChar != 'q') {
    gST->ConOut->ClearScreen (gST->ConOut);
    FrontPage (Xhc);
    Status = WaitForKeyStroke (&Key);
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      Cntlr = 0;
      while (Key.UnicodeChar != 'Q' && Key.UnicodeChar != 'q' && Key.ScanCode != SCAN_ESC) {
        gST->ConOut->ClearScreen (gST->ConOut);
        Print (L" Base Address Register: %08x", (Xhc->PciConfigBuf + Cntlr * sizeof(PCI_CONFIG))->BaseAddress[0]);
        Status = WaitForKeyStroke (&Key);
      }
    }
  }

  gST->ConOut->ClearScreen (gST->ConOut);
  return EFI_SUCCESS;
}

// VOID
// Selector (
//   UINTN                        Column,
//   UINTN                        Row
// )
// {
//   gST->ConOut->EnableCursor(gST->ConOut, FALSE);
//   gST->ConOut->SetCursorPosition(gST->ConOut, Column, Row);
//   Print(L">");
//   gST->ConOut->SetCursorPosition(gST->ConOut, Column, Row);
// }

VOID
FrontPage (
  USB3_CONTROLLER              *Xhc
)
{
  UINT8                        Index;
  Print (L" Index  Bus  Dev  Func  VendorId  DeviceId\n");
  for (Index = 0; Index < Xhc->XhcNum; Index++) {
    Print (L"     %x   %02x   %02x    %02x      %04x      %04x\n",
      Index,
      ((PCI_CONFIG*)((UINT8*)Xhc->PciConfigBuf + Index * sizeof (PCI_CONFIG)))->Bus,
      ((PCI_CONFIG*)((UINT8*)Xhc->PciConfigBuf + Index * sizeof (PCI_CONFIG)))->Device,
      ((PCI_CONFIG*)((UINT8*)Xhc->PciConfigBuf + Index * sizeof (PCI_CONFIG)))->Function,
      ((PCI_CONFIG*)((UINT8*)Xhc->PciConfigBuf + Index * sizeof (PCI_CONFIG)))->VendorId,
      ((PCI_CONFIG*)((UINT8*)Xhc->PciConfigBuf + Index * sizeof (PCI_CONFIG)))->DeviceId
      );
  }
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
  UINT32                       Bar0, Bar1;
  USB3_CONTROLLER              *Xhc;

  Xhc = AllocateZeroPool (sizeof (USB3_CONTROLLER));

  for (Bus = 0; Bus <= MAX_BUS_NUM; Bus++) {
    for (Dev = 0; Dev <= MAX_DEV_NUM; Dev++) {
      for (Func = 0; Func <= MAX_FUNC_NUM; Func++) {

        VendorId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x0));
        if (VendorId == 0xffff) {
          continue;
        }

        BaseClass = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0xb));
        SubClass  = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0xa));
        Interface = PciRead8 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x9));

        if ((BaseClass == XHCI_BASE_CLASS) && (SubClass == XHCI_SUB_CLASS) && (Interface == XHCI_INTERFACE)) {
          Xhc->XhcNum++;
          Xhc->PciConfigBuf = ReallocatePool ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG), Xhc->XhcNum * sizeof (PCI_CONFIG), Xhc->PciConfigBuf);

          DeviceId = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x2));
          Bar0     = PciRead32 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x10));
          Bar1     = PciRead32 (PCI_LIB_ADDRESS (Bus, Dev, Func, 0x14));

          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->Bus            = (Bus & 0xff);
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->Device         = Dev;
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->Function       = Func;
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->VendorId       = VendorId;
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->DeviceId       = DeviceId;
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->BaseAddress[0] = Bar0;
          ((PCI_CONFIG*)((UINT8*)(Xhc->PciConfigBuf) + ((Xhc->XhcNum - 1) * sizeof (PCI_CONFIG))))->BaseAddress[1] = Bar1;
        }

      }
    }
  }

  return Xhc;
}
