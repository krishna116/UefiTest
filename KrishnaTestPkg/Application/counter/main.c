#include "Counter.h"

#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

EFI_STATUS DoWork(Counter *counter);

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status = EFI_SUCCESS;

    Counter *counter = CounterCreate();
    Status = DoWork(counter);
    counter->Destroy(&counter);

    return Status;
}

/**
 * @brief Parse args and go to count.
 * 
 * Some usage:
 *   counter.efi -a
 *   counter.efi -s
 *   counter.efi -a -s
 *   counter.efi -a -a -s
 *   counter.efi -s -a -a
 *   counter.efi -a -s -d 1000 -e
 *   counter.efi -c
 * 
 * @param[in] counter   A Counter instance.
 * 
 * @return EFI_STATUS 
 */
EFI_STATUS DoWork(Counter *counter)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN Argc = 0;
    CHAR16 **Argv = NULL;
    UINTN CountToAdd = 0;
    UINTN CountToShow = 0;
    BOOLEAN ExportEnv = 0;
    BOOLEAN CounterClear = 0;
    UINTN IsFirstActionAdd = 0;
    UINTN IsFirstActionShow = 0;

    ASSERT(counter != NULL);

    if (gEfiShellParametersProtocol != NULL)
    {
        Argc = gEfiShellParametersProtocol->Argc;
        Argv = gEfiShellParametersProtocol->Argv;
    }
    else
    {
        PrintError(L"Shell parameter protocol is empty.\n");
        return EFI_ABORTED;
    }

    if (Argc <= 1)
    {
        return counter->ShowHelp(counter);
    }

    for (UINTN i = 1; i < Argc; i++)
    {
        if (StrCmp(Argv[i], L"-a") == 0 || StrCmp(Argv[i], L"--add") == 0)
        {
            ++CountToAdd;
            if (IsFirstActionAdd == 0 && IsFirstActionShow == 0)
            {
                IsFirstActionAdd = 1;
            }
        }
        else if (StrCmp(Argv[i], L"-s") == 0 || StrCmp(Argv[i], L"--show") == 0)
        {
            ++CountToShow;
            if (IsFirstActionAdd == 0 && IsFirstActionShow == 0)
            {
                IsFirstActionShow = 1;
            }
        }
        else if (StrCmp(Argv[i], L"-d") == 0 || StrCmp(Argv[i], L"--delay") == 0)
        {
            UINTN isOk = 0;

            if(i+1<Argc)
            {
                UINTN milliseconds=0;
                if(StrDecimalToUintnS(Argv[i+1],NULL,&milliseconds)==RETURN_SUCCESS)
                {
                    counter->SetDelayTime(counter, milliseconds);
                    isOk=1;
                }
                ++i;
            }

            if(!isOk)
            {
                PrintError(L"Invalid parameter.\n");
                return EFI_ABORTED;
            }
        }
        else if (StrCmp(Argv[i], L"-e") == 0 || StrCmp(Argv[i], L"--env") == 0)
        {
            if(ExportEnv == 0)
            {
                ExportEnv = 1;
                counter->SetExportEnv(counter, ExportEnv);
            }
            else
            {
                PrintError(L"Invalid parameter.\n");
                return EFI_ABORTED;
            }
        }
        else if (StrCmp(Argv[i], L"-c") == 0 || StrCmp(Argv[i], L"--clear") == 0)
        {
            if(CounterClear == 0)
            {
                CounterClear = 1;
            }
            else
            {
                PrintError(L"Invalid parameter.\n");
                return EFI_ABORTED;
            }
        }
        else if (StrCmp(Argv[i], L"-h") == 0 || StrCmp(Argv[i], L"--help") == 0)
        {
            return counter->ShowHelp(counter);
        }
        else if (StrCmp(Argv[i], L"-v") == 0 || StrCmp(Argv[i], L"--version") == 0)
        {
            return counter->ShowVersion(counter);
        }
        else
        {
            PrintError(L"Invalid parameter.\n");
            return EFI_ABORTED;
        }
    }

    Status = EFI_SUCCESS;

    if(CounterClear)
    {
        counter->ClearCount(counter);
        return Status;
    }

    if (IsFirstActionAdd)
    {
        if (CountToAdd > 0)
            Status = counter->AddToCount(counter, CountToAdd);
        if (CountToShow > 0 && Status == EFI_SUCCESS)
            Status = counter->ShowCount(counter);
    }
    else
    {
        if (CountToShow > 0)
            Status = counter->ShowCount(counter);
        if (CountToAdd > 0 && Status == EFI_SUCCESS)
            Status = counter->AddToCount(counter, CountToAdd);
    }

    return Status;
}