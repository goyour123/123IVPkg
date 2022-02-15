
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UserInterfaceLib.h>

#include <Protocol/SerialIo.h>

VOID
PrintMainQuest (
  )
{
  Print (L"\n");
  Print (L"1. Write Serial Port from keyboard\n");
  Print (L"2. Read Serial Port\n");
  Print (L"3. Retrieves the status of the control bits\n");
  Print (L"Press \"Q\" or ESC to quit\n");
}

EFI_STATUS
EFIAPI
SerialIoMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  EFI_SERIAL_IO_PROTOCOL *SerialIo;
  EFI_SERIAL_IO_MODE     *SerialIoMode;
  UINTN                  BufLen;
  EFI_INPUT_KEY          Key;
  CHAR8                  AsciiChar;
  UINT32                 ControlBits;

  Status = gBS->LocateProtocol (&gEfiSerialIoProtocolGuid, NULL, &SerialIo);
  if (EFI_ERROR (Status)) {
    Print (L"LocateProtocol gEfiSerialIoProtocolGuid: %r\n", Status);
    return Status;
  }

  SerialIoMode = SerialIo->Mode;
  Print (L"Serial IO Mode:\n");
  Print (L"  ControlMask:      0x%04X\n", SerialIoMode->ControlMask);
  Print (L"  Timeout:          0x%08X\n", SerialIoMode->Timeout);
  Print (L"  BaudRate:         0x%04X\n", SerialIoMode->BaudRate);
  Print (L"  ReceiveFifoDepth: 0x%04x\n", SerialIoMode->ReceiveFifoDepth);
  Print (L"  DataBits:         0x%04x\n", SerialIoMode->DataBits);
  Print (L"  Parity:           0x%04x\n", SerialIoMode->Parity);
  Print (L"  StopBits:         0x%04x\n", SerialIoMode->StopBits);
  Print (L"\n");

  //
  // Set Serial IO attributes
  //
  // Status = SerialIo->SetAttributes (
  //                      SerialIo,
  //                      SerialIoMode->BaudRate,
  //                      SerialIoMode->ReceiveFifoDepth,
  //                      SerialIoMode->Timeout,
  //                      SerialIoMode->Parity,
  //                      0,
  //                      SerialIoMode->StopBits
  //                      );
  // if (EFI_ERROR (Status)) {
  //   Print (L"SerialIo->SetAttributes: %r\n", Status);
  //   return Status;
  // }

  PrintMainQuest ();

  while (TRUE) {

    WaitForKeyStroke (&Key);

    if (Key.ScanCode == SCAN_ESC || Key.UnicodeChar == L'q' || Key.UnicodeChar == L'Q') {
      break;
    }

    if (Key.UnicodeChar == L'1') {
      Print (L"Writing Serial Port from Keyboard...\n");
      Print (L"Press ESC to quit\n");
      Print (L"\n");
      while (TRUE) {
        WaitForKeyStroke (&Key);
        if (Key.ScanCode == SCAN_ESC) {
          break;
        }

        AsciiChar = (CHAR8) Key.UnicodeChar;
        Print (L"Keyboard In: %c\n", AsciiChar);
        BufLen = sizeof (AsciiChar);
        Status = SerialIo->Write (SerialIo, &BufLen, &AsciiChar);
        if (EFI_ERROR (Status)) {
          Print (L"SerialIo->Write: %r\n", Status);
          return Status;
        }
      }
    } else if (Key.UnicodeChar == L'2') {
      Print (L"Reading Serial Port...\n");
      Print (L"\n");
      BufLen = 1;
      Status = SerialIo->Read (SerialIo, &BufLen, &AsciiChar);
      if (EFI_ERROR (Status)) {
        Print (L"SerialIo->Read: %r\n", Status);
      }
      Print (L"Buffer Length: %x\n", BufLen);
      Print (L"Read Out: %c\n", AsciiChar);
    } else if (Key.UnicodeChar == L'3') {
      Status = SerialIo->GetControl (SerialIo, &ControlBits);
      if (EFI_ERROR (Status)) {
        Print (L"SerialIo GetControl: %r\n", Status);
        return Status;
      }
      Print (L"Control Bits: 0x%x\n", ControlBits);
    }

    PrintMainQuest ();
  }

  return EFI_SUCCESS;
}
