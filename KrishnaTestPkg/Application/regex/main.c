#include "RegexWorker.h"

#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //global gST gBS gImageHandle
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>

EFI_STATUS Work(RegexWorker* Worker);

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status = EFI_ABORTED;
    RegexWorker* Worker = RegexWorkerCreate();
    if(Worker)
    {
        Status = Work(Worker);
        Worker->Destroy(&Worker);
    }
    return Status;
}

EFI_STATUS PrintVersion()
{
    Print(L"%s\n", EFI_REGEX_VERSION_STR);
    return EFI_SUCCESS;
}

EFI_STATUS PrintHelp()
{
    Print(L"regex.efi -p <pattern>\n");
    Print(L"\n");
    Print(L"It is a tiny regex tool used under uefi-shell.\n");
    Print(L"\n");
    Print(L"Options:\n");
    Print(L"  -p <pattern>          specify regex pattern.\n");
    Print(L"  -0                    only output the first matched item.\n");
    Print(L"  -1                    only output the last matched item.\n");
    Print(L"Others:\n");
    Print(L"  -h,--help             show this help.\n");
    Print(L"    ,--help-pattern     show regex pattern help.\n");
    Print(L"  -v,--version          show version.\n");

    return EFI_SUCCESS;
}


EFI_STATUS PrintHelpPattern()
{
    Print(L"Pattern:\n");
    Print(L"  .             matches any character\n");
    Print(L"  ^             matches beginning of string\n");
    Print(L"  $             matches end of string\n");
    Print(L"  *             match zero or more (greedy)\n");
    Print(L"  +             match one or more (greedy)\n");
    Print(L"  ?             match zero or one (non-greedy)\n");
    Print(L"  [abc]         match if one of {'a', 'b', 'c'}\n");
    Print(L"  [a-zA-Z]      character ranges {a-z | A-Z}\n");
    Print(L"  \\s            whitespace, \\t \\f \\r \\n \\v and spaces\n");
    Print(L"  \\S            non-whitespace\n");
    Print(L"  \\w            alphanumeric, [a-zA-Z0-9_]\n");
    Print(L"  \\W            non-alphanumeric\n");
    Print(L"  \\d            digits, [0-9]\n");
    Print(L"  \\D            non-digits\n");

    return EFI_SUCCESS;
}

EFI_STATUS Work(RegexWorker* Worker)
{
    ASSERT(Worker != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR16** Argv = Worker->Private_.Param->Argv;
    UINTN Argc = Worker->Private_.Param->Argc;
    BOOLEAN HasSetPattern = FALSE;
    BOOLEAN HasArgsOk = TRUE;
    UINTN i = 1;

    if(Argc <= 1) return PrintHelp();

    while(i < Argc)
    {
        if(StrCmp(Argv[i], L"-h") == 0 || StrCmp(Argv[i], L"--help") == 0)
        {
            return PrintHelp();
        }
        else if(StrCmp(Argv[i], L"-v") == 0 || StrCmp(Argv[i], L"--version") == 0)
        {
            return PrintVersion();
        }
        else if(StrCmp(Argv[i], L"--help-pattern") == 0)
        {
            return PrintHelpPattern();
        }
        else if(StrCmp(Argv[i], L"-0") == 0)
        {
            Worker->SetOutputType(Worker, OutputTypeFirst);
            i += 1;
            continue;
        }
        else if(StrCmp(Argv[i], L"-1") == 0)
        {
            Worker->SetOutputType(Worker, OutputTypeLast);
            i += 1;
            continue;
        }
        else if(StrCmp(Argv[i], L"-p") == 0  && (i+1) < Argc)
        {
            if(Worker->SetPatternStr(Worker, Argv[i+1]) == EFI_SUCCESS)
            {
                HasSetPattern = TRUE;
                i += 2;
                continue;
            }
            else
            {
                HasSetPattern = FALSE;
                break;
            }
        }
        else
        {
            PrintError(L"Invalid parameter.\n");
            HasArgsOk = FALSE;
            break;
        }
    }

    if(HasArgsOk && HasSetPattern)
    {
        Status = Worker->DoWork(Worker);
    }

    return Status;
}