
#ifndef __USER_INTERFACE_LIB___
#define __USER_INTERFACE_LIB___

/**
  Wait for a key to be pressed by user.

  @param Key         The key which is pressed by user.

  @retval EFI_SUCCESS The function always completed successfully.

**/
EFI_STATUS
EFIAPI
WaitForKeyStroke (
  OUT  EFI_INPUT_KEY           *Key
  );

#endif