
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/UsbIo.h>
#include <Protocol/SimpleNetwork.h>

EFI_STATUS
EFIAPI
UsbLanMacMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *HandleBuffer;
  UINTN                             HandleCount;
  UINTN                             Index;
  EFI_USB_IO_PROTOCOL               *UsbIo;
  EFI_USB_DEVICE_DESCRIPTOR         UsbDevDesc;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_HANDLE                        Handle;
  MAC_ADDR_DEVICE_PATH              *MacDevPath;
  EFI_STRING                        MacString;
  UINTN                             MacStringSize;

  MacString = NULL;

  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleNetworkProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID **) &DevicePath);
    Print (L"  HandleProtocol Device Path: %r\n", Status);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->LocateDevicePath (&gEfiUsbIoProtocolGuid, &DevicePath, &Handle);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (Handle, &gEfiUsbIoProtocolGuid, (VOID **) &UsbIo);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = UsbIo->UsbGetDeviceDescriptor (UsbIo, &UsbDevDesc);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (UsbDevDesc.IdVendor == 0x0BDA && UsbDevDesc.IdProduct == 0x8153) {
      Print (L"  Realtek USB LAN device found (VID: 0x%04X, PID: 0x%04X)\n", UsbDevDesc.IdVendor, UsbDevDesc.IdProduct);
      Print (L"  Device Path Type: %x, Device Path Subtype: %x\n", DevicePathType(DevicePath), DevicePathSubType(DevicePath));
      if ((DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH) && (DevicePathSubType(DevicePath) == MSG_MAC_ADDR_DP)) {
        MacDevPath = (MAC_ADDR_DEVICE_PATH *) DevicePath;
        MacStringSize = (12 + 5 + 1) * sizeof(CHAR16);
        MacString = AllocateZeroPool(MacStringSize);
        UnicodeSPrintAsciiFormat (
          MacString, 
          MacStringSize, 
          "%02x-%02x-%02x-%02x-%02x-%02x",
          MacDevPath->MacAddress.Addr[0],
          MacDevPath->MacAddress.Addr[1],
          MacDevPath->MacAddress.Addr[2],
          MacDevPath->MacAddress.Addr[3],
          MacDevPath->MacAddress.Addr[4],
          MacDevPath->MacAddress.Addr[5]
          );
        Print (L"  Mac Address: %s", MacString);
        break;
      }
    }
  }

  return EFI_SUCCESS;
}