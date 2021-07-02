
#include "InternalRtcLib.h"

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT  0x71

/**
  Read an 8-bit value from the offset of RTC

  @param Offset Offset of RTC

  @retval An 8-bit value read from the offset of RTC

**/
UINT8
ReadRtc8 (
  IN UINT8 Offset
  )
{
  UINT8 Value;

  IoWrite8 (RTC_INDEX_PORT, Offset);
  Value = IoRead8 (RTC_DATA_PORT);

  return Value;
}

/**
 Write an 8-bit value to the offset of RTC

 @param[in] Offset Offset of RTC
 @param[in] Data   Data written into RTC offset
**/
VOID
WriteRtc8 (
  IN UINT8 Offset,
  IN UINT8 Data
  )
{
  IoWrite8 (RTC_INDEX_PORT, Offset);
  IoWrite8 (RTC_DATA_PORT, Data);

  return;
}