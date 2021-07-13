
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/UserInterfaceLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>

#include <Library/JsonLib.h>

#define TIME_OUT               0xffff

// EC Register
#define EC_COMMAND_STATUS_PORT 0x666
#define EC_DATA_PORT           0x662

typedef union {
  struct {
    UINT8  Obf:1;
    UINT8  Ibf:1;
    UINT8  Ign0:1;
    UINT8  Cmd:1;
    UINT8  Burst:1;
    UINT8  SciEvt:1;
    UINT8  SmiEvt:1;
    UINT8  Ign1:1;
  } Field;
  UINT8 Value;
} EC_STATUS;

// EC Command
#define READ_EC          0x80
#define WRITE_EC         0x81
#define BURST_ENABLE_EC  0x82
#define BURST_DISABLE_EC 0x83
#define Query_EC         0x84

typedef struct {
  UINTN Column;
  UINTN Row;
} CURSOR_POSITION;

EFI_STATUS
WaitInputBufferEmpty (
  IN UINTN EcStatusPort
  )
{
  EFI_STATUS Status;
  EC_STATUS  EcStatus;
  UINT32     TimeOut;

  Status = EFI_DEVICE_ERROR;

  for (TimeOut = 0; TimeOut < TIME_OUT; TimeOut++) {
    EcStatus.Value = IoRead8 (EcStatusPort);
    if (EcStatus.Field.Ibf == 0) {
      Status = EFI_SUCCESS;
      break;
    }

    // Delay
    IoRead8 (0x80);
  }

  return Status;
}

EFI_STATUS
WaitOutputBufferFull (
  IN UINTN EcStatusPort
  )
{
  EFI_STATUS Status;
  EC_STATUS  EcStatus;
  UINT32     TimeOut;

  Status = EFI_DEVICE_ERROR;

  for (TimeOut = 0; TimeOut < TIME_OUT; TimeOut++) {
    EcStatus.Value = IoRead8 (EcStatusPort);
    if (EcStatus.Field.Obf == 1) {
      Status = EFI_SUCCESS;
      break;
    }

    // Delay
    IoRead8 (0x80);
  }

  return Status;
}

EFI_STATUS
ReadEc (
  IN  UINT8 *Offset,
  OUT UINT8 *Data
  )
{
  EFI_STATUS Status;

  // Wait IBF empty
  Status = WaitInputBufferEmpty (EC_COMMAND_STATUS_PORT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Send read EC command
  IoWrite8 (EC_COMMAND_STATUS_PORT, READ_EC);

  // Wait IBF empty
  Status = WaitInputBufferEmpty (EC_COMMAND_STATUS_PORT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Send read EC command address
  IoWrite8 (EC_DATA_PORT, *Offset);

  // Wait OBF full
  Status = WaitOutputBufferFull (EC_COMMAND_STATUS_PORT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Read output data
  *Data = IoRead8 (EC_DATA_PORT);

  return Status;
}

VOID
KeyInHandler (
  IN OUT EFI_INPUT_KEY   *Key,
  IN     UINT8           Width,
  IN     UINTN           Column,
  IN     UINTN           Row
  )
{
  BOOLEAN         KeyInProcess;
  UINT8           Count;
  UINT8           Value8;

  KeyInProcess = FALSE;
  Count = 0;
  Value8 = 0;

  do {
    KeyInProcess = TRUE;
    if (Count < Width * 2) {
      if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'f') {
        Value8 = Value8 * 0x10 + ((Key->UnicodeChar & 0xFF) - 0x57);
      } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'F') {
        Value8 = Value8 * 0x10 + ((Key->UnicodeChar & 0xFF) - 0x37);
      } else if (Key->UnicodeChar >= '0' && Key->UnicodeChar <= '9') {
        Value8 = Value8 * 0x10 + ((Key->UnicodeChar & 0xFF) - 0x30);
      } else {
        KeyInProcess = FALSE;
      }
    } else {
      KeyInProcess = FALSE;
    }

    if (KeyInProcess) {
      gST->ConOut->SetCursorPosition (gST->ConOut, Column - Width, Row);
      Print (L"%02X", Value8);
      Count++;
    }

    WaitForKeyStroke (Key);
    if (Key->ScanCode == SCAN_ESC) {
      break;
    }

  } while (TRUE);
}

VOID
RegsFrame (
  VOID*  Regs,
  UINTN  Length,
  UINT8  Width
  )
{
  UINTN           Index;
  UINT8           RegIndex;
  UINT8           OldRegIndex;
  CURSOR_POSITION *Cursor;
  EFI_INPUT_KEY   Key;

  RegIndex = 0;
  OldRegIndex = 0;

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLUE));

  // Print column header
  Print (L"   ");
  for (Index = 0; Index < 0x10; Index++) {
    Print (L" %02X", Index);
  }
  Print (L"\n");

  Cursor = AllocateZeroPool (sizeof (CURSOR_POSITION) * Length);

  for (Index = 0; Index < Length; Index++) {
    // Print row header
    if (Index % 0x10 == 0) {
      Print (L" %02X", Index / 0x10);
    }

    // Print Register value
    Print (L" %02X", ((UINT8 *)Regs)[Index]);
    Cursor[Index].Column = gST->ConOut->Mode->CursorColumn - 1;
    Cursor[Index].Row = gST->ConOut->Mode->CursorRow;

    if (Index % 0x10 == 0xf) {
      Print (L"\n");
    }
  }

  while (TRUE) {

    // Show current register index on left-top corner
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLUE));
    Print (L" ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_RED, EFI_GREEN));
    Print (L"%02X", RegIndex);

    // Select register
    gST->ConOut->SetCursorPosition (gST->ConOut, Cursor[RegIndex].Column - 1, Cursor[RegIndex].Row);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_BLACK, EFI_GREEN));
    Print (L"%02X", ((UINT8 *)Regs)[RegIndex]);

    WaitForKeyStroke (&Key);

    if (Key.ScanCode == SCAN_ESC) {
      break;
    }

    if ((Key.UnicodeChar >= L'0' && Key.UnicodeChar <= L'9') || \
      (Key.UnicodeChar >= L'A' && Key.UnicodeChar <= L'F') || \
      (Key.UnicodeChar >= L'a' && Key.UnicodeChar <= L'f')) {
      KeyInHandler (&Key, Width, Cursor[RegIndex].Column, Cursor[RegIndex].Row);
    }

    OldRegIndex = RegIndex;
    if (Key.ScanCode == SCAN_DOWN) {
      if (RegIndex / 0x10 == 0xf) {
        RegIndex -= 0xf0;
      } else {
        RegIndex += 0x10;
      }
    } else if (Key.ScanCode == SCAN_UP) {
      if (RegIndex / 0x10 == 0) {
        RegIndex += 0xf0;
      } else {
        RegIndex -= 0x10;
      }
    } else if (Key.ScanCode == SCAN_LEFT) {
      if (RegIndex % 0x10 == 0) {
        RegIndex += 0xf;
      } else {
        RegIndex -= 1;
      }
    } else if (Key.ScanCode == SCAN_RIGHT) {
      if (RegIndex % 0x10 == 0xf) {
        RegIndex -= 0xf;
      } else {
        RegIndex += 1;
      }
    }

    // Restore Old selected register
    if (RegIndex != OldRegIndex) {
      gST->ConOut->SetCursorPosition (gST->ConOut, Cursor[OldRegIndex].Column - 1, Cursor[OldRegIndex].Row);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLUE));
      Print (L"%02X", ((UINT8 *)Regs)[OldRegIndex]);
    }
  }
}

EFI_STATUS
EFIAPI
EcMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                      Status;
  UINTN                           Index;
  UINT8                           Data;
  UINT8                           *Buffer;
  UINTN                           BufferLen;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_HANDLE                      DeviceHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE                        *Root;
  EFI_FILE                        *FileHandle;
  UINTN                           EcCommandStatusPort, EcDataPort;
  EDKII_JSON_VALUE                *JsonObj;
  EDKII_JSON_ERROR                JsonErr;

  Status = gBS->HandleProtocol (
             ImageHandle,
             &gEfiLoadedImageProtocolGuid,
             &LoadedImage
             );
  if (EFI_ERROR (Status)) {
    Print (L"HandleProtocol LoadedImage: %r\n", Status);
    return Status;
  }

  Status = gBS->HandleProtocol (
             LoadedImage->DeviceHandle,
             &gEfiDevicePathProtocolGuid,
             &DevicePath
             );
  if (EFI_ERROR (Status)) {
    Print (L"HandleProtocol DevicePath: %r\n", Status);
    return Status;
  }

  Status = gBS->LocateDevicePath (
             &gEfiSimpleFileSystemProtocolGuid,
             &DevicePath,
             &DeviceHandle
             );
  if (EFI_ERROR (Status)) {
    Print (L"LocateDevicePath SimpleFileSystem: %r\n", Status);
    return Status;
  }

  Status = gBS->HandleProtocol (
             DeviceHandle,
             &gEfiSimpleFileSystemProtocolGuid,
             (VOID*)&SimpleFileSystem
             );
  if (EFI_ERROR (Status)) {
    Print (L"HandleProtocol SimpleFileSystem: %r\n", Status);
    return Status;
  }

  Status = SimpleFileSystem->OpenVolume (
             SimpleFileSystem,
             &Root
             );
  if (EFI_ERROR (Status)) {
    Print (L"SimpleFileSystem->OpenVolume: %r\n", Status);
    return Status;
  }

  Status = Root->Open (
             Root,
             &FileHandle,
             L"Ec.json",
             EFI_FILE_MODE_READ,
             0
             );
  if (EFI_ERROR (Status)) {
    Print (L"Root->Open: %r\n", Status);
    return Status;
  }

  BufferLen = 0x100;
  Buffer = AllocateZeroPool (BufferLen);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Status = FileHandle->Read (FileHandle, &BufferLen, Buffer);
  if (EFI_ERROR (Status)) {
    Print (L"FileHandle->Read: %r\n", Status);
    return Status;
  }

  JsonObj = JsonLoadBuffer (Buffer, BufferLen, 0, &JsonErr);
  if (!JsonValueIsObject (JsonObj)) {
    Print (L"Configuration file is not a json object\n");
    return EFI_SUCCESS;
  }

  EcCommandStatusPort = StrHexToUintn (JsonValueGetUnicodeString (JsonObjectGetValue (JsonObj, "CommandStatusPort")));
  EcDataPort = StrHexToUintn (JsonValueGetUnicodeString (JsonObjectGetValue (JsonObj, "DataPort")));

  Data = 0;
  BufferLen = 0x100;
  Buffer = AllocateZeroPool (BufferLen);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < BufferLen; Index++) {
    Status = ReadEc (&((UINT8) Index), &Data);
    if (!EFI_ERROR (Status)) {
      Buffer[Index] = Data;
    }
  }

  gST->ConOut->SetMode(gST->ConOut, 0);
  gST->ConOut->ClearScreen (gST->ConOut);

  RegsFrame (Buffer, BufferLen, sizeof (UINT8));

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK));
  gST->ConOut->Reset(gST->ConOut, FALSE);

  return EFI_SUCCESS;
}
