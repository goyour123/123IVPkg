
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

#include <Library/UefiBootServicesTableLib.h>
#include <PiDxe.h>
#include <Protocol/MpService.h>

EFI_STATUS
EFIAPI
MpServiceProtocolMain (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_MP_SERVICES_PROTOCOL     *MpService;
  UINTN                        ProcessorNum;
  UINTN                        EnabledProcessorNum;

  Status = gBS->LocateProtocol (&gEfiMpServiceProtocolGuid, NULL, (VOID **) &MpService);
  if (EFI_ERROR (Status)) {
    Print (L"Locate MP Services Protocol %r\n", Status);
    return Status;
  }

  Status = MpService->GetNumberOfProcessors (
                        MpService,
                        &ProcessorNum,
                        &EnabledProcessorNum
                        );
  if (!EFI_ERROR (Status)) {
    Print (L"MP Services Protocol Number of Processors: %x\n", ProcessorNum);
    Print (L"MP Services Protocol Number of Enabled Processors: %x\n", EnabledProcessorNum);
  } else {
    Print (L"MP Services GetNumberOfProcessors: %r\n", Status);
  }

  return EFI_SUCCESS;
}

