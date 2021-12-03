#include "Xdb.h"

#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
 * @brief Parse args and do the work.
 * 
 * This tool is used to collect information/logs when run many tests.
 * 
 * @param xdb               A Xdb instance.
 * @param Param             A shell parameters protocol instance.
 * 
 * @return EFI_SUCCESS      Operation pass.
 * @return Others           Operation fail.
 */
EFI_STATUS DoWork(Xdb *xdb, EFI_SHELL_PARAMETERS_PROTOCOL *Param);

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status = EFI_ABORTED;
    EFI_SHELL_PARAMETERS_PROTOCOL *Param = NULL;

    Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiShellParametersProtocolGuid, &Param);
    if (Status != EFI_SUCCESS)
    {
        PrintError(L"Cannot locate shell parameters protocol.\n");
        return Status;
    }

    Xdb *xdb = XdbCreate();
    if (xdb)
    {
        Status = DoWork(xdb, Param);
        XdbDestroy(&xdb);
    }

    return Status;
}

/**
 * @brief Show help.
 */
VOID ShowHelp()
{
    Print(L"xdb.efi [<file>] <option>\n");
    Print(L"\n");
    Print(L"It is used to get and set data in a local key-value database.\n");
    Print(L"\n");
    Print(L"Options:\n");
    Print(L"  --set <key> <value>           //create or update the key-value.\n");
    Print(L"  --get <key>                   //get the key-value.\n");
    Print(L"  --push-front <key> <value>    //push a key-value at front.\n");
    Print(L"  --push-back <key> <value>     //push a key-value at back.\n");
    Print(L"  --pop-front                   //pop a key-value at front.\n");
    Print(L"  --pop-back                    //pop a key-value at back.\n");
    Print(L"  --append <key> <value>        //append the key-value(permit duplicate keys).\n");
    Print(L"  --dump                        //dump all key-values in the database.\n");
    Print(L"  --size                        //get database size(sum of key-values).\n");
    Print(L"  --erase <key>                 //erase the key-value.\n");
    Print(L"  --clear                       //clear all data of the file.\n");
    Print(L"  --help                        //show help.\n");
    Print(L"  --version                     //show version.\n");
    Print(L"Note:\n");
    Print(L"  Output to shell variable: %a, %a, %a\n", XDB_ENV_VARIABLE_KEY, XDB_ENV_VARIABLE_VALUE, XDB_ENV_VARIABLE_SIZE);
    Print(L"\n");
}

/**
 * @brief Show version.
 */
VOID ShowVersion()
{
    Print(L"%s\n", XDB_VERSION_STR);
}

EFI_STATUS DoWork(Xdb *xdb, EFI_SHELL_PARAMETERS_PROTOCOL *Param)
{
    ASSERT(xdb != NULL);
    ASSERT(Param != NULL);

    EFI_STATUS Status = EFI_SUCCESS;
    BOOLEAN hasSetFileName = 0;
    UINTN Argc = Param->Argc;
    CHAR16 **Argv = Param->Argv;

    if (Argc <= 1)
    {
        ShowHelp();
        return EFI_SUCCESS;
    }

    UINTN i = 1;
    while (i < Argc)
    {
        if (StrLen(Argv[i]) >= 1 && *(Argv[i]) != '-' && !hasSetFileName)
        {
            xdb->SetFileName(xdb, Argv[i]);
            hasSetFileName = TRUE;
            i += 1;
            continue;
        }
        else if(StrCmp(Argv[i], L"--verbose") == 0)
        {
            xdb->SetVerboseMode(xdb, TRUE);
            i += 1;
            continue;
        }
        else if (StrCmp(Argv[i], L"--set") == 0 && (i + 3) == Argc)
        {
            return xdb->Set(xdb, Argv[i+1], Argv[i+2]);
        }
        else if (StrCmp(Argv[i], L"--get") == 0 && (i + 2) == Argc)
        {
            return xdb->Get(xdb, Argv[i + 1]);
        }
        else if (StrCmp(Argv[i], L"--append") == 0 && (i + 3) == Argc)
        {
            return xdb->Append(xdb, Argv[i + 1], Argv[i + 2]);
        }
        else if (StrCmp(Argv[i], L"--push-front") == 0 && (i + 3) == Argc)
        {
            return xdb->Push(xdb, Argv[i + 1], Argv[i + 2], TRUE);
        }
        else if (StrCmp(Argv[i], L"--push-back") == 0 && (i + 3) == Argc)
        {
            return xdb->Push(xdb, Argv[i + 1], Argv[i + 2], FALSE);
        }
        else if (StrCmp(Argv[i], L"--pop-front") == 0 && (i + 1) == Argc)
        {
            return xdb->Pop(xdb, TRUE);
        }
        else if (StrCmp(Argv[i], L"--pop-back") == 0 && (i + 1) == Argc)
        {
            return xdb->Pop(xdb, FALSE);
        }
        else if (StrCmp(Argv[i], L"--dump") == 0)
        {
            return xdb->Dump(xdb);
        }
        else if (StrCmp(Argv[i], L"--size") == 0)
        {
            return xdb->GetDatabaseSize(xdb);
        }
        else if (StrCmp(Argv[i], L"--erase") == 0 && (i + 2) == Argc)
        {
            return xdb->Clear(xdb, Argv[i + 1]);
        }
        else if (StrCmp(Argv[i], L"--clear") == 0)
        {
            return xdb->Clear(xdb, L"");
        }
        else if (StrCmp(Argv[i], L"--help") == 0)
        {
            ShowHelp();
            break;
        }
        else if (StrCmp(Argv[i], L"--version") == 0)
        {
            ShowVersion();
            break;
        }
        else
        {
            PrintError(L"Invalid parameter.\n");
            Status = EFI_ABORTED;
            break;
        }
    }

    return Status;
}
