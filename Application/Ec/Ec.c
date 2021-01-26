
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/IoLib.h>

// EC Register
#define EC_COMMAND_STATUS_PORT 0x66
#define EC_DATA_PORT           0x62

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
  IN UINT16 EcStatusPort
)
{
  EFI_STATUS Status;
  EC_STATUS  EcStatus;
  UINT16     TimeOut;

  Status = EFI_DEVICE_ERROR;

  for (TimeOut = 0; TimeOut < 0xFFFF; TimeOut++) {
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
EFIAPI
EcMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Hello World\n");
  return EFI_SUCCESS;
}
