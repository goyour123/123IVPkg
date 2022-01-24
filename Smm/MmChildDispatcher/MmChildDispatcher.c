
#include "MmChildDispatcher.h"

EFI_STATUS
EFIAPI
SwSmiCallback (
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
SxSmiCallback (
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
IoTrapSmiCallback (
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
RegisterSwSmi (
  )
{
  EFI_STATUS                            Status;
  EFI_MM_SW_DISPATCH_PROTOCOL           *MmSwProtocol;
  EFI_MM_SW_REGISTER_CONTEXT            MmSwRegisterContext;
  EFI_HANDLE                            MmSwHandle;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiMmSwDispatchProtocolGuid,
                    NULL,
                    &MmSwProtocol
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MmSwRegisterContext.SwMmiInputValue = 0x66;

  Status = MmSwProtocol->Register (
                           MmSwProtocol,
                           SwSmiCallback,
                           &MmSwRegisterContext,
                           &MmSwHandle
                           );
  if EFI_ERROR (Status) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RegisterSxSmi (
  )
{
  EFI_STATUS                            Status;
  EFI_MM_SX_DISPATCH_PROTOCOL           *MmSwProtocol;
  EFI_MM_SX_REGISTER_CONTEXT            MmSwRegisterContext;
  EFI_HANDLE                            MmSwHandle;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiMmSwDispatchProtocolGuid,
                    NULL,
                    &MmSwProtocol
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MmSwRegisterContext.Type  = SxS5;
  MmSwRegisterContext.Phase = SxEntry;

  Status = MmSwProtocol->Register (
                           MmSwProtocol,
                           SxSmiCallback,
                           &MmSwRegisterContext,
                           &MmSwHandle
                           );
  if EFI_ERROR (Status) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RegisterIoTrapSmi (
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
                               IoTrapSmiCallback,
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
  EFI_STATUS           Status;

  Status = RegisterSwSmi ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RegisterSxSmi ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RegisterIoTrapSmi ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}