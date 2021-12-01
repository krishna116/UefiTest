[Defines]
  PLATFORM_NAME                  = KrishnaTest
  PLATFORM_GUID                  = F6D2FCF2-9550-4049-ABBC-A79792B8308A
  PLATFORM_VERSION               = 1.02
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/KrishnaTest
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses.common]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLibOptionalDevicePathProtocol.inf
!if $(TARGET) == RELEASE
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!else
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
!endif
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  !include NetworkPkg/NetworkLibs.dsc.inc

  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
  AcpiViewCommandLib|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf

  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  ArgumentManagerLib|KrishnaTestPkg/Library/ArgumentManagerLib/ArgumentManagerLib.inf
  
  JsonLib|RedfishPkg/Library/JsonLib/JsonLib.inf
  Ucs2Utf8Lib|RedfishPkg/Library/BaseUcs2Utf8Lib/BaseUcs2Utf8Lib.inf
  RedfishCrtLib|RedfishPkg/PrivateLibrary/RedfishCrtLib/RedfishCrtLib.inf
  BaseSortLib|MdeModulePkg/Library/BaseSortLib/BaseSortLib.inf

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000
!ifdef $(NO_SHELL_PROFILES)
  gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x00
!endif #$(NO_SHELL_PROFILES)

[Components]

[Components.X64]

  ## build -p KrishnaTestPkg\KrishnaTestPkg.dsc -a X64
  KrishnaTestPkg/Application/grep/grep.inf
  KrishnaTestPkg/Application/history/history.inf
  KrishnaTestPkg/Application/counter/counter.inf

  ##  
  ## This regex.efi has tiny-regex-c dependency(https://github.com/kokke/tiny-regex-c)
  ## 1, download tiny-regex-c library.
  ## 2, copy tiny-regex-c/re.c to KrishnaTestPkg/Application/regex/tiny-regex-c
  ## 3, copy tiny-regex-c/re.h to KrishnaTestPkg/Application/regex/tiny-regex-c
  ## 4, modify KrishnaTestPkg/Application/regex/tiny-regex-c/re.c as follow description:
  ##    modify value(#define MAX_REGEXP_OBJECTS) to value(#define EFI_REGEX_PATTERN_STR_MAX_COUNT) in RegexWorker.h
  ##    modify value(#define MAX_CHAR_CLASS_LEN) to value(#define EFI_REGEX_TEXT_STR_MAX_COUNT) in RegexWorker.h
  ## 5, Now you can build [KrishnaTestPkg/Application/regex/regex.inf].
  ##
  KrishnaTestPkg/Application/regex/regex.inf

  ##  
  ## To build this xdb.efi successfully, you should modify edk2 source code(edk2-stable202108) as follow description.
  ##
  ## 1, In "edk2\RedfishPkg\Include\Library\JsonLib.h", add follow declaration:
  ##      EFI_STATUS
  ##      EFIAPI
  ##      JsonArrayInsertValue (
  ##      IN    EDKII_JSON_ARRAY    JsonArray,
  ##      IN    UINTN               Index,
  ##      IN    EDKII_JSON_VALUE    Json
  ##      );
  ## 
  ## 2, In "edk2\RedfishPkg\Library\JsonLib\JsonLib.c", add follow code:
  ##    EFI_STATUS
  ##    EFIAPI
  ##    JsonArrayInsertValue (
  ##      IN    EDKII_JSON_ARRAY    JsonArray,
  ##      IN    UINTN               Index,
  ##      IN    EDKII_JSON_VALUE    Json
  ##      )
  ##    {
  ##      if(json_array_insert((json_t*) JsonArray, Index, (json_t*) Json)!=0)
  ##      {
  ##        return EFI_ABORTED;
  ##      }else
  ##      {
  ##        return EFI_SUCCESS;
  ##      }
  ##    }
  ##
  ## 3, Now you can build [KrishnaTestPkg/Application/xdb/xdb.inf].
  ##
  KrishnaTestPkg/Application/xdb/xdb.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
