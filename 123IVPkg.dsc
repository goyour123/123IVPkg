
[Defines]
  PLATFORM_NAME           = 123IVPkg
  PLATFORM_GUID           = 12341234-1234-1234-1234-000000000000
  PLATFORM_VERSION        = 0.01
  DSC_SPECIFICATION       = 0x00010006
  OUTPUT_DIRECTORY        = Build/123IVPkg
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER        = DEFAULT

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf

  #
  # Common Libraries
  #
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf

  #
  # Basic Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf

  #
  # Generic Module
  #
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[Components]
  #
  # Application Samples
  #
  123IVPkg/Samples/Hello/Hello.inf
  123IVPkg/Samples/Hello/Hello2.inf
  123IVPkg/Samples/PciScan/PciScan.inf

  #
  # Application
  #
  123IVPkg/App/XhciViewer/XhciViewer.inf
