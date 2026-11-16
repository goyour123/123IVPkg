/** @file
  UEFI driver that intentionally triggers a #PF (Page-Fault, vector 14) exception.

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/MemoryAttribute.h>

#define UNMAPPED_ADDRESS  0x00007FFFFFFFF000ULL

/**
  Read from a not-present page to trigger a #PF exception.

  @retval EFI_ABORTED  The access did not fault.
**/
EFI_STATUS
TriggerPageFault (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_MEMORY_ATTRIBUTE_PROTOCOL  *MemoryAttribute;
  EFI_PHYSICAL_ADDRESS           Page;
  volatile UINT32                *Target;
  UINT32                         Value;

  Target = (volatile UINT32 *)(UINTN)UNMAPPED_ADDRESS;

  Status = gBS->LocateProtocol (&gEfiMemoryAttributeProtocolGuid, NULL, (VOID **)&MemoryAttribute);
  if (!EFI_ERROR (Status)) {
    Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 1, &Page);
    if (!EFI_ERROR (Status)) {
      *(UINT32 *)(UINTN)Page = 0x12345678;
      Status                 = MemoryAttribute->SetMemoryAttributes (MemoryAttribute, Page, EFI_PAGE_SIZE, EFI_MEMORY_RP);
      if (!EFI_ERROR (Status)) {
        Target = (volatile UINT32 *)(UINTN)Page;
        Print (L"PageFault: page 0x%lx marked not-present (EFI_MEMORY_RP).\n", Page);
      } else {
        Print (L"PageFault: SetMemoryAttributes failed: %r, using fallback.\n", Status);
        gBS->FreePages (Page, 1);
      }
    }
  } else {
    Print (L"PageFault: EFI_MEMORY_ATTRIBUTE_PROTOCOL not found, using fallback.\n");
  }

  Print (L"PageFault: reading 0x%lx, expect #PF with CR2=0x%lx ...\n", (UINT64)(UINTN)Target, (UINT64)(UINTN)Target);

  Value = *Target;

  //
  // Only reached if the access did not fault.
  //
  Print (L"PageFault: no fault occurred, read 0x%08x.\n", Value);
  return EFI_ABORTED;
}

EFI_STATUS
EFIAPI
PageFaultEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print (L"PageFault: CR0=0x%lx CR3=0x%lx CR4=0x%lx\n", (UINT64)AsmReadCr0 (), (UINT64)AsmReadCr3 (), (UINT64)AsmReadCr4 ());

  if ((AsmReadCr0 () & BIT31) == 0) {
    Print (L"PageFault: paging is disabled (CR0.PG=0), #PF cannot occur.\n");
    return EFI_UNSUPPORTED;
  }

  return TriggerPageFault ();
}
