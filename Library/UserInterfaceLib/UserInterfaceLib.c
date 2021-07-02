
#include "InternalUserInterfaceLib.h"

/**
  Wait for a key to be pressed by user.

  @param Key The key which is pressed by user.

  @retval EFI_SUCCESS The function always completed successfully.

**/
EFI_STATUS
WaitForKeyStroke (
  OUT EFI_INPUT_KEY *Key
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  while (TRUE) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
    if (!EFI_ERROR (Status)) {
      break;
    }

    if (Status != EFI_NOT_READY) {
      continue;
    }

    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
  }
  return Status;
}