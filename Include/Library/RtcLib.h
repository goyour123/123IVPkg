
#ifndef __RTC_LIB___
#define __RTC_LIB___

#define RTC_REG_SECONDS        0x00
#define RTC_REG_ALARM_SECONDS  0x01
#define RTC_REG_MINUTES        0x02
#define RTC_REG_ALARM_MINUTES  0x03
#define RTC_REG_HOURS          0x04
#define RTC_REG_ALARM_HOURS    0x05
#define RTC_REG_STATUS_A       0x0A
#define RTC_REG_STATUS_B       0x0B
#define   RTC_REG_STATUS_B_AIE   BIT5
#define RTC_REG_STATUS_C       0x0C
#define RTC_REG_STATUS_D       0x0D

/**
  Read an 8-bit value from the offset of RTC

  @param Offset Offset of RTC

  @retval An 8-bit value read from the offset of RTC

**/
UINT8
ReadRtc8 (
  IN UINT8 Offset
  );

/**
 Write an 8-bit value to the offset of RTC.

 @param[in] Offset Offset of RTC.
 @param[in] Data   Data written into RTC offset.
**/
VOID
WriteRtc8 (
  IN UINT8 Offset,
  IN UINT8 Data
  );

#endif