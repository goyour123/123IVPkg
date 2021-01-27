
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

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
PrintRegs (
  VOID*  Buffer,
  UINTN  Length
  )
{
  UINTN Index;

  // Print column header
  Print (L"   ");
  for (Index = 0; Index < 0x10; Index++) {
    Print (L" %02X", Index);
  }
  Print (L"\n");

  for (Index = 0; Index < Length; Index++) {
    // Print row header
    if (Index % 0x10 == 0) {
      Print (L" %02X", Index % 0x10);
    }

    Print (L" %02X", ((UINT8 *)Buffer)[Index]);

    if (Index % 0x10 == 0xf) {
      Print (L"\n");
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
  EFI_STATUS Status;
  UINTN      Index;
  UINT8      Data;
  UINT8      *Buffer;
  UINTN      BufferLen;

  Data = 0;
  BufferLen = 0x100;
  Buffer = AllocateZeroPool (BufferLen);

  for (Index = 0; Index < BufferLen; Index++) {
    Status = ReadEc (&((UINT8) Index), &Data);
    if (!EFI_ERROR (Status)) {
      Buffer[Index] = Data;
    }
  }

  PrintRegs (Buffer, BufferLen);

  return EFI_SUCCESS;
}
