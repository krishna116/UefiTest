#ifndef __READ_HISTORY__H__
#define __READ_HISTORY__H__

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //global gST gBS gImageHandle
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>

#include <Protocol/Shell.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/SimpleTextOut.h>

/// Reference from Shell->ConsoleLogger.h
#define CONSOLE_LOGGER_PRIVATE_DATA_SIGNATURE SIGNATURE_32('c', 'o', 'P', 'D')

/// Reference from Shell->ConsoleLogger.h
#pragma pack(1)
typedef struct _CONSOLE_LOGGER_PRIVATE_DATA
{
    UINTN Signature;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     OurConOut;          ///< the protocol we installed onto the system table
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *OldConOut;         ///< old protocol to reinstall upon exiting
    EFI_HANDLE                          OldConHandle;       ///< old protocol handle
    UINTN                               ScreenCount;        ///< How many screens worth of data to save
    CHAR16                              *Buffer;            ///< Buffer to save data
    UINTN                               BufferSize;         ///< size of buffer in bytes

    UINTN                               OriginalStartRow;   ///< What the originally visible start row was
    UINTN                               CurrentStartRow;    ///< what the currently visible start row is

    UINTN                               RowsPerScreen;      ///< how many rows the screen can display
    UINTN                               ColsPerScreen;      ///< how many columns the screen can display

    INT32                               *Attributes;        ///< Buffer for Attribute to be saved for each character
    UINTN                               AttribSize;         ///< Size of Attributes in bytes

    EFI_SIMPLE_TEXT_OUTPUT_MODE         HistoryMode;        ///< mode of the history log
    BOOLEAN                             Enabled;            ///< Set to FALSE when a break is requested.
    UINTN                               RowCounter;         ///< Initial row of each print job.
} CONSOLE_LOGGER_PRIVATE_DATA;
#pragma pack()

/// Reference from Shell->ConsoleLogger.h
#define CONSOLE_LOGGER_PRIVATE_DATA_FROM_THIS(a) CR(a, CONSOLE_LOGGER_PRIVATE_DATA, OurConOut, CONSOLE_LOGGER_PRIVATE_DATA_SIGNATURE)

/**
 * This application's name.
 */
#define APP_NAME "ReadHistory"

/**
 * Using it to print verbose informations.
 */
#ifndef NO_VERBOSE
#define PrintError(message, ...) Print(L"[%a][%d][%a]:" message, APP_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define PrintError(message, ...)
#endif

/**
 * This application's help message.
 */
VOID PrintUsageMsg();

/**
 * Read some history messages from console.
 * 
 * @retval 1        Operation pass.
 * @retval 0        Operation failed.
 */
UINTN ReadHistory();

#endif //__READ_HISTORY__H__
