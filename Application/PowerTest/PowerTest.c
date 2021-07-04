
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/RtcLib.h>

#define WAKE_AFTER_SECONDS 3

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
  if (BcdMode) {
    Sec = DecimalToBcd8 (Sec);
    Min = DecimalToBcd8 (Min);
    Hour = DecimalToBcd8 (Hour);
  }

  //
  // Set RTC alarm
  //
  WriteRtc8 (RTC_REG_ALARM_SECONDS, Sec);
  WriteRtc8 (RTC_REG_ALARM_MINUTES, Min);
  WriteRtc8 (RTC_REG_ALARM_HOURS, Hour);
  Print (L"Set Alarm Time to: %d:%d:%d\n", Hour, Min, Sec);

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

  // To do: Program PM registers

  return EFI_SUCCESS;
}

