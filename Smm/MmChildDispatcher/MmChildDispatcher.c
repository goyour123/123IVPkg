
#include <Uefi.h>

#include <Library/SmmServicesTableLib.h>

#include <Protocol/MmIoTrapDispatch.h>

EFI_STATUS
EFIAPI
Cf9TrapCallback (
  IN EFI_HANDLE        DispatchHandle,
  IN CONST VOID        *Context,
  IN OUT VOID          *CommBuffer,
  IN OUT UINTN         *CommBufferSize
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RegisterIoTrap (
  )
{
  EFI_STATUS                            Status;
  EFI_MM_IO_TRAP_DISPATCH_PROTOCOL      *MmIoTrapProtocol;
  EFI_MM_IO_TRAP_REGISTER_CONTEXT       MmIoTrapRegisterContext;
  EFI_HANDLE                            MmIoTrapHandle;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiMmIoTrapDispatchProtocolGuid,
                    NULL,
                    &MmIoTrapProtocol
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MmIoTrapRegisterContext.Address = 0xcf9;
  MmIoTrapRegisterContext.Length  = 1;
  MmIoTrapRegisterContext.Type    = WriteTrap;

  Status = MmIoTrapProtocol->Register (
                               MmIoTrapProtocol,
                               Cf9TrapCallback,
                               &MmIoTrapRegisterContext,
                               &MmIoTrapHandle
                               );
  if EFI_ERROR (Status) {
    return Status;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
MmChildDispatcherEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                            Status;

  Status = RegisterIoTrap ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}