#include "ReadHistory.h"
#include "Historian.h"

/**
 * The entry point for the application.
 * 
 * @param[in] ImageHandle    The firmware allocated handle for the EFI image.
 * @param[in] SystemTable    A pointer to the EFI System Table.
 * 
 * @retval EFI_SUCCESS       The entry point is executed successfully.
 * @retval other             Some error occurs when executing this entry point.
 */
EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    return ReadHistory();
}

VOID PrintUsageMsg()
{
    ///_____11111111112222222222333333333344444444445555555555666666666677777777778888888888
    Print(L"+-----------------------------------------------------------------------------+\n");
    Print(L"|  ReadHistory.efi v1.1 for reading uefi shell history message.               |\n");
    Print(L"|                                                        --krishna,2019.4.16  |\n");
    Print(L"+-----------------------------------------------------------------------------+\n");
    Print(L"Usage:\n");
    Print(L"  ReadHistory.efi [n]     //read recent [n] lines of history,(n>0).\n");
    Print(L"  ReadHistory.efi -l [n]  //read recent one line,[n] is line-number,(n>0).\n");
    Print(L"Note:\n");
    Print(L"  If [n] > valid screen buffer line count, [n] will stay at the top line.\n\n");
};

UINTN ReadHistory()
{
    UINTN number = 0;
    UINTN returnCode = 0;

    if (gHistorian.constructor(&gHistorian) == 1)
    {
        if (gHistorian.param->Argc == 1)
        {
            PrintUsageMsg();
            return 0;
        }
        else if (gHistorian.param->Argc == 2)
        {
            if (StrCmp(gHistorian.param->Argv[1], L"-h") == 0 ||
                StrCmp(gHistorian.param->Argv[1], L"-help") == 0 ||
                StrCmp(gHistorian.param->Argv[1], L"--help") == 0 ||
                StrCmp(gHistorian.param->Argv[1], L"-?") == 0 ||
                StrCmp(gHistorian.param->Argv[1], L"?") == 0)
            {
                PrintUsageMsg();
                return 0;
            }

            if (StrDecimalToUintnS(gHistorian.param->Argv[1], NULL, &number) == RETURN_SUCCESS)
            {
                if (number > 0)
                {
                    returnCode = gHistorian.tellStory(&gHistorian, number, 0);
                }
                else
                {
                    PrintError(L"Invalid number or number is 0.\n");
                }
            }
            else
            {
                PrintError(L"Invalid number.\n");
            }
        }
        else if (gHistorian.param->Argc == 3)
        {
            if (StrCmp(gHistorian.param->Argv[1], L"-line") == 0 || StrCmp(gHistorian.param->Argv[1], L"-l") == 0 || StrCmp(gHistorian.param->Argv[1], L"--line") == 0)
            {
                if (StrDecimalToUintnS(gHistorian.param->Argv[2], NULL, &number) == RETURN_SUCCESS)
                {
                    if (number > 0)
                    {
                        returnCode = gHistorian.tellStory(&gHistorian, number, 1);
                    }
                    else
                    {
                        PrintError(L"Invalid number or number is 0.\n");
                    }
                }
                else
                {
                    PrintError(L"Invalid number.\n");
                }
            }
            else
            {
                PrintError(L"Invalid option.\n");
            }
        }
        else
        {
            PrintError(L"Invalid parameter.\n");
        }
    }

    gHistorian.destructor(&gHistorian);

    return returnCode;
}
