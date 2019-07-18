#ifndef __GREP_H__
#define __GREP_H__

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //global gST gBS gImageHandle
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h >
#include <Library/ShellLib.h>
#include <Library/ArgumentManagerLib.h>  //gArgumentManager

#include <Protocol/Shell.h>
#include <Protocol/ShellParameters.h>

/**
 * This application's name.
 */
#define APP_NAME "grep"

/**
 * Using it to print verbose informations.
 */
#ifndef NO_VERBOSE
#define PrintError(message, ...) Print(L"[%a][%d][%a]:" message, APP_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define PrintError(message, ...)
#endif

#endif //__GREP_H__
