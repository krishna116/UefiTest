#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //global gST gBS gImageHandle
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h >
#include <Library/ShellLib.h>
#include <Library/ArgumentManagerLib.h>

#include <Protocol/Shell.h>
#include <Protocol/ShellParameters.h>

ArgumentManager gArgumentManager = {
    NULL,
    1,    /// offset 0 is this application's path string,offset 1 is our first argument.
    1,    /// backup currentIndex.

    ArgumentManagerConstructor,
    Size,
    PeekArg,
    GetNextArg,
    GetNextArgAndCompare,
    BackupIndex,
    RestoreIndex,
};

UINTN ArgumentManagerConstructor(ArgumentManager *This)
{
    EFI_STATUS status;

    ASSERT(This != NULL);
    ASSERT(This->param==NULL);

    status = gBS->HandleProtocol(gImageHandle, &gEfiShellParametersProtocolGuid, &(This->param));
    if (status != EFI_SUCCESS)
    {
        DEBUG ((EFI_D_ERROR, "Construct ArgumentManager failed.\n"));
        return 0;
    }

    return 1;
};

UINTN Size(ArgumentManager *This)
{
    ASSERT(This != NULL);
    ASSERT(This->param!=NULL);

    return This->param->Argc;
};

UINTN PeekArg(ArgumentManager *This)
{
    ASSERT(This != NULL);

    if (This->size(This) <= 1)
    {
        return 0;
    }

    /// we will using currentIndex to locate a arg,
    /// currentIndex can be not out of the max-args-offset;
    if (This->currentIndex > (This->size(This) - 1))
    {
        return 0;
    }

    return 1;
};

CHAR16 *GetNextArg(ArgumentManager *This)
{
    ASSERT(This != NULL);

    if (This->peekArg(This) == 1)
    {
        return This->param->Argv[This->currentIndex++];
    }

    return NULL;
};

UINTN GetNextArgAndCompare(ArgumentManager *This, CHAR16 *String)
{
    CHAR16 *arg = NULL;

    ASSERT(This != NULL);
    ASSERT(String != NULL);

    if ((arg = This->getNextArg(This)) != NULL)
    {
        if (StrCmp(arg, String) == 0)
        {
            return 1;
        }
    }

    return 0;
}

VOID BackupIndex(ArgumentManager *This)
{
    ASSERT(This != NULL);

    This->backup = This->currentIndex;
};

VOID RestoreIndex(ArgumentManager *This)
{
    ASSERT(This != NULL);

    This->currentIndex = This->backup;
};