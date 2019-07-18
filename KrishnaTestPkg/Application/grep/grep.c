#include "grep.h"
#include "BufferManager.h"
#include "ArgumentParser.h"

UINTN Grep()
{
    UINTN returnCode = 0;

    if (gParser.constructor(&gParser) == 1)
    {
        returnCode = gParser.work(&gParser);
    }
    gParser.destructor(&gParser);

    return returnCode;
}

/**
 * Entry point for the application.
 *
 * @param[in] ImageHandle    This image.
 * @param[in] SystemTable    EFI System Table.
 *
 * @retval any               Some working status.
 */
EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    return Grep();
}
