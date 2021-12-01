#include "history.h"
#include "Historian.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    return ReadHistory();
}

EFI_STATUS PrintHelp()
{
    Print(L"history.efi [<option>]\n");
    Print(L"\n");
    Print(L"It is used to read uefi shell history message.\n");
    Print(L"\n");
    Print(L"Options:\n");
    Print(L"  -b,--block <n>    //read recent n-line-block of history(n>0).\n");
    Print(L"  -l,--line <n>     //read recent one line of history(n>0).\n");
    Print(L"  -h,--help         //show help.\n");
    Print(L"  -v,--version      //show version.\n");

    return EFI_SUCCESS;
};

EFI_STATUS PrintVersion()
{
    Print(L"%s\n", HISTORY_APP_VERSION_STR);
    return EFI_SUCCESS;
}

EFI_STATUS ReadHistory()
{
    UINTN Argc = 0;
    CHAR16** Argv = NULL;
    UINTN Number = 0;
    UINTN i = 1;
    EFI_STATUS Status = EFI_ABORTED;

    if (gHistorian.constructor(&gHistorian) != 1) return Status;

    Argc = gHistorian.param->Argc;
    Argv = gHistorian.param->Argv;

    if(Argc <= 1) return PrintHelp();

    while(i < Argc)
    {
        if(StrCmp(Argv[i], L"-h") == 0 || StrCmp(Argv[i], L"--help") == 0)
        {
            Status = PrintHelp();
            break;
        }
        else if(StrCmp(Argv[i], L"-v") == 0 || StrCmp(Argv[i], L"--version") == 0)
        {
            Status = PrintVersion();
            break;
        }
        else if((StrCmp(Argv[i], L"-b") == 0 || StrCmp(Argv[i], L"--block") == 0) && (i+1)<Argc)
        {
            if (StrDecimalToUintnS(Argv[i+1], NULL, &Number) == RETURN_SUCCESS)
            {
                Status = gHistorian.tellStory(&gHistorian, Number, 0);
            }
            else
            {
                PrintError(L"Invalid number.\n");
            }
            break;
        }
        else if((StrCmp(Argv[i], L"-l") == 0 || StrCmp(Argv[i], L"--line") == 0) && (i+1)<Argc)
        {
            if (StrDecimalToUintnS(Argv[i+1], NULL, &Number) == RETURN_SUCCESS)
            {
                Status = gHistorian.tellStory(&gHistorian, Number, 1);
            }
            else
            {
                PrintError(L"Invalid number.\n");
            }
            break;
        }
        else
        {
            PrintError(L"Invalid parameter.\n");
            break;
        }
    }

    gHistorian.destructor(&gHistorian);
    return Status;
}
