
[Defines]
  PLATFORM_NAME           = 123IVPkg
  PLATFORM_GUID           = fe3dd29d-1234-4843-9381-e5aeed74979f
  PLATFORM_VERSION        = 0.01
  DSC_SPECIFICATION       = 0x00010006
  OUTPUT_DIRECTORY        = Build/123IVPkg
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER        = DEFAULT

  DEFINE REDFISH_ENABLE   = TRUE

!if $(REDFISH_ENABLE) == TRUE
  DEFINE REDFISH_DEF_DSC  = RedfishPkg/RedfishDefines.dsc.inc
  DEFINE REDFISH_LIB_DSC  = RedfishPkg/RedfishLibs.dsc.inc
!else
  DEFINE REDFISH_DEF_DSC  =
  DEFINE REDFISH_LIB_DSC  =
!endif

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
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
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf

  #
  # Basic Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf

  #
  # Generic Module
  #
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  #
  # 123IVPkg Libraries
  #
  UiLib|123IVPkg/Library/UserInterfaceLib/UserInterfaceLib.inf
  RtcLib|123IVPkg/Library/RtcLib/RtcLib.inf

!if $(REDFISH_ENABLE) == TRUE
  !include $(REDFISH_DEF_DSC)
  !include $(REDFISH_LIB_DSC)
!endif

[Components]
  #
  # Application Samples
  #
  123IVPkg/Samples/Hello/Hello.inf
  123IVPkg/Samples/Hello/Hello2.inf
  123IVPkg/Samples/PciScan/PciScan.inf
  123IVPkg/Samples/MpServiceProtocol/MpServiceProtocol.inf
  123IVPkg/Samples/SerialIo/SerialIo.inf

  #
  # Application
  #
  123IVPkg/Application/XhciViewer/XhciViewer.inf
!if $(REDFISH_ENABLE) == TRUE
  123IVPkg/Application/Ec/EcJson.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = -D REDFISH_ENABLE
  }
!else
  123IVPkg/Application/Ec/Ec.inf
!endif
  123IVPkg/Application/PowerTest/PowerTest.inf
  
  #
  # Smm Driver
  #
  123IVPkg/Smm/MmChildDispatcher/MmChildDispatcher.inf
