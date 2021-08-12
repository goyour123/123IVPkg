
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/RtcLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>

#include <Protocol/AcpiSystemDescriptionTable.h>

#define WAKE_AFTER_SECONDS     3
#define ACPI_PM1A_EVT_BLK_ADDR 0x400
#define ACPI_PM1A_CNT_BLK_ADDR 0x404

#define SLP_TYP5               BIT10 + BIT12

EFI_STATUS
EFIAPI
PowerTestMain (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
{
  UINT8                     WakeAfterSec;
  UINT8                     Value8;
  UINT8                     Sec, Min, Hour;
  UINT8                     Rem;
  BOOLEAN                   BcdMode;
  EFI_STATUS                Status;
  EFI_ACPI_SDT_PROTOCOL     *AcpiSdtProtocol;
  UINTN                     Index;
  EFI_ACPI_SDT_HEADER       *AcpiSdtHeader;
  EFI_ACPI_TABLE_VERSION    AcpiTableVersion;
  UINTN                     TableKey;
  BOOLEAN                   FadtFound;
  UINT32                    Pm1aEvtReg, Pm1aCntReg;

  WakeAfterSec = WAKE_AFTER_SECONDS;

  //
  // Check RTC is BCD or Binary mode
  //
  Value8  = ReadRtc8 (RTC_REG_STATUS_B);
  BcdMode = (Value8 & BIT2) ? FALSE : TRUE;

  //
  // Get Current time
  //
  Sec = ReadRtc8 (RTC_REG_SECONDS);
  Min = ReadRtc8 (RTC_REG_MINUTES);
  Hour   = ReadRtc8 (RTC_REG_HOURS);
  if (BcdMode) {
    Sec = BcdToDecimal8 (Sec);
    Min = BcdToDecimal8 (Min);
    Hour = BcdToDecimal8 (Hour);
  }
  Print (L"Get Current Time: %02d:%02d:%02d\n", Hour, Min, Sec);

  Rem  = WakeAfterSec + Sec;
  Sec = Rem % 60;
  Rem  = (Rem / 60) + Min;
  Min = Rem % 60;
  Rem  = (Rem / 60) + Hour;
  Hour   = Rem % 24;
  Print (L"Set Alarm Time to: %02d:%02d:%02d\n", Hour, Min, Sec);

  //
  // Set RTC alarm
  //
  if (BcdMode) {
    Sec = DecimalToBcd8 (Sec);
    Min = DecimalToBcd8 (Min);
    Hour = DecimalToBcd8 (Hour);
  }
  WriteRtc8 (RTC_REG_ALARM_SECONDS, Sec);
  WriteRtc8 (RTC_REG_ALARM_MINUTES, Min);
  WriteRtc8 (RTC_REG_ALARM_HOURS, Hour);

  //
  // Clear Register C and D
  //
  ReadRtc8 (RTC_REG_STATUS_C);
  WriteRtc8 (RTC_REG_STATUS_D, 0);

  //
  // Enable RTC AIE
  //
  Value8 = ReadRtc8 (RTC_REG_STATUS_B);
  Value8 |= RTC_REG_STATUS_B_AIE;
  WriteRtc8 (RTC_REG_STATUS_B, Value8);

  //
  // Get PM register from ACPI SDT protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSdtProtocolGuid, NULL, &AcpiSdtProtocol);
  if (!EFI_ERROR (Status)) {
    FadtFound = FALSE;
    for (Index = 0; FadtFound == FALSE; Index++) {
      Status = AcpiSdtProtocol->GetAcpiTable (Index, &AcpiSdtHeader, &AcpiTableVersion, &TableKey);
      if (EFI_ERROR (Status)) {
        Print (L"AcpiSdtProtocol->GetAcpiTable: %r\n", Status);
        return Status;
      }

      if (AcpiSdtHeader->Signature == SIGNATURE_32 ('F', 'A', 'C', 'P')) {
        Print (L"AcpiTableVersion: %x\n", AcpiTableVersion);
        if (AcpiTableVersion & EFI_ACPI_TABLE_VERSION_5_0) {
          Pm1aEvtReg = ((EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE *) AcpiSdtHeader)->Pm1aEvtBlk;
          Pm1aCntReg = ((EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE *) AcpiSdtHeader)->Pm1aCntBlk;
        } else {
          Pm1aEvtReg = ACPI_PM1A_EVT_BLK_ADDR;
          Pm1aCntReg = ACPI_PM1A_CNT_BLK_ADDR;
        }
        Print (L"Pm1aEvtReg: %x\n", Pm1aEvtReg);
        FadtFound = TRUE;
      }
      FreePool (AcpiSdtHeader);
    }
  } else {
    Pm1aEvtReg = ACPI_PM1A_EVT_BLK_ADDR;
    Pm1aCntReg = ACPI_PM1A_CNT_BLK_ADDR;
  }

  //
  // Clear RTC_STS and set RTC_EN
  //
  IoOr16 (Pm1aEvtReg, BIT10);
  IoOr16 (Pm1aEvtReg + 2, BIT10);

  //
  // Set SLP_TYPx and SLP_EN
  //
  IoAndThenOr16 (Pm1aCntReg, BIT10 + BIT11 + BIT12, SLP_TYP5);
  IoOr16 (Pm1aCntReg, BIT13);

  return EFI_SUCCESS;
}
