
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/ShellParameters.h>

EFI_STATUS
EFIAPI
ResetTestMain (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
{
  EFI_STATUS                     Status;
  UINTN                          Argc;
  CHAR16                         **Argv;
  EFI_SHELL_PARAMETERS_PROTOCOL  *ShellParameters;
  UINTN                          Count;
  UINTN                          DataSize;
  UINT32                         Attributes;
  UINTN                          Index;
  UINT8                          WaitSecs;

  WaitSecs = 3;

  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **)&ShellParameters
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Argc = ShellParameters->Argc;
  Argv = ShellParameters->Argv;

  if (Argc < 3 || (StrCmp (Argv[1], L"-c") != 0 && StrCmp (Argv[1], L"-C") != 0)) {
    Print (L"Usage: ResetTest.efi -c <Count>\n");
    Print (L"Example: ResetTest.efi -c 5\n");
    return EFI_INVALID_PARAMETER;
  }

  Count = StrDecimalToUintn (Argv[2]);
  if (Count == 0) {
    Print (L"Count must be greater than zero!\n");
    return EFI_INVALID_PARAMETER;
  }

  DataSize = sizeof (Count);
  Status = gRT->GetVariable (
                  L"ResetTestCount",
                  &gEfi123ivVariableGuid,
                  &Attributes,
                  &DataSize,
                  &Count
                  );
  if (Status == EFI_NOT_FOUND) {
    // First time run
    Print (L"Initial run, setting reset count to %d\n", Count);
    Status = gRT->SetVariable (
                    L"ResetTestCount",
                    &gEfi123ivVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (Count),
                    &Count
                    );
    if (EFI_ERROR (Status)) {
      Print (L"SetVariable Error: %r\n", Status);
      return Status;
    }
  } else if (EFI_ERROR (Status)) {
    Print (L"GetVariable Error: %r\n", Status);
    return Status;
  } else {
    // Subsequent run
    if (Count <= 1) {
      Print (L"Reset test completed!\n");
      // Clear the variable
      Status = gRT->SetVariable (
                      L"ResetTestCount",
                      &gEfi123ivVariableGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      0,
                      NULL
                      );
      if (EFI_ERROR (Status)) {
        Print (L"SetVariable Error: %r\n", Status);
        return Status;
      }
      return EFI_SUCCESS;
    }

    Count--;
    Print (L"Resetting... Remaining count is %d\n", Count);
    Status = gRT->SetVariable (
                    L"ResetTestCount",
                    &gEfi123ivVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (Count),
                    &Count
                    );
    if (EFI_ERROR (Status)) {
      Print (L"SetVariable Error: %r\n", Status);
      return Status;
    }
  }

  Print (L"System will reset in %d seconds\n", WaitSecs);
  for (Index = WaitSecs; Index > 0; Index--) {
    Print (L"Reset in %d seconds...\n", Index);
    gBS->Stall (1000000);
  }

  Print (L"Resetting now...\n");
  gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

  return EFI_SUCCESS;
}
