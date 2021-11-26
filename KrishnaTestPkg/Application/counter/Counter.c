#include "Counter.h"
#include "digits.h"

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>

#include <Library/SafeIntLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/SimpleTextOut.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>

//
// Reference from GRAPHICS CONSOLE Driver.
//
#define GRAPHICS_CONSOLE_DEV_SIGNATURE SIGNATURE_32('g', 's', 't', 'o')

//
// Reference from GRAPHICS CONSOLE Driver.
//
typedef struct
{
    UINTN Columns;
    UINTN Rows;
    INTN DeltaX;
    INTN DeltaY;
    UINT32 GopWidth;
    UINT32 GopHeight;
    UINT32 GopModeNumber;
} GRAPHICS_CONSOLE_MODE_DATA;

//
// Reference from GRAPHICS CONSOLE Driver.
//
typedef struct
{
    UINTN Signature;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
    EFI_UGA_DRAW_PROTOCOL *UgaDraw;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL SimpleTextOutput;
    EFI_SIMPLE_TEXT_OUTPUT_MODE SimpleTextOutputMode;
    GRAPHICS_CONSOLE_MODE_DATA *ModeData;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *LineBuffer;
} GRAPHICS_CONSOLE_DEV;

//
// Reference from GRAPHICS CONSOLE Driver.
//
#define GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(a) \
    CR(a, GRAPHICS_CONSOLE_DEV, SimpleTextOutput, GRAPHICS_CONSOLE_DEV_SIGNATURE)

STATIC UINTN mCounterDelayTime = 0;
STATIC BOOLEAN mExportEnv = 0;
STATIC CHAR8 mCommonBuffer[64];
STATIC CONST UINTN mCommonBufferSize = sizeof(mCommonBuffer);

/**
 * @brief There are many SimpleTextProtocol in uefi system, this function 
 * will get one with which has GOP binded.
 * 
 * @param[out] SimpleTextProtocol       A SimpleTextProtocol instance.
 * 
 * @retval EFI_SUCCESS                  Operation success.
 * @retval Others                       Operation fail.
 */
EFI_STATUS FindDesiredSimpleTextOutProtocol(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL **SimpleTextProtocol)
{
    EFI_STATUS Status = EFI_NOT_FOUND;
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer = NULL;

    ASSERT(SimpleTextProtocol != NULL);

    Status = gBS->LocateHandleBuffer(
        ByProtocol, &gEfiGraphicsOutputProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer);

    if (!EFI_ERROR(Status))
    {
        for (UINTN i = 0; i < HandleCount; i++)
        {
            Status = gBS->OpenProtocol(
                HandleBuffer[i],
                &gEfiSimpleTextOutProtocolGuid,
                SimpleTextProtocol,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL);

            if (!EFI_ERROR(Status))
            {
                // Got it.
                Status = EFI_SUCCESS;
                break;
            }
        }
    }

    if (HandleBuffer)
    {
        gBS->FreePool(HandleBuffer);
        HandleBuffer = NULL;
    }

    return Status;
}

/**
 * @brief Convert UINTN to decimal string. 
 * If convert success, it is not the the caller's responsibility to free the return string.
 * If convert fail, return NULL.
 * 
 * @param[in] DecimalNumber         A decimal number.
 * 
 * @return CHAR8 *                  A string.
 */
CHAR8 *DecimalNumberToString(UINTN DecimalNumber)
{
    ZeroMem(mCommonBuffer,mCommonBufferSize);
    UINTN BufferSize = mCommonBufferSize -1;

    if (AsciiValueToStringS(mCommonBuffer, BufferSize, 0, DecimalNumber, COUNTER_MAX_WIDTH) == RETURN_SUCCESS)
    {
        return mCommonBuffer;
    }
    else
    {
        return NULL;
    }
}

EFI_SHELL_PROTOCOL* GetEfiShellProtocol()
{
    EFI_STATUS Status;
    EFI_SHELL_PROTOCOL *shell;

    Status = gBS->LocateProtocol(
        &gEfiShellProtocolGuid,
        NULL,
        (VOID **)&shell
    );
    if (EFI_ERROR(Status))
    {
        PrintError(L"Cannot get shell protocol.\n");
        return NULL;
    }

    return shell;
}

/**
 * @brief Draw decimal number using GOP.
 * 
 * @param[in] DecimalNumber     A decimal number.
 */
EFI_STATUS DrawDecimalNumber(UINTN DecimalNumber)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *pSimpleTextOut = NULL;
    GRAPHICS_CONSOLE_DEV *Private = NULL;
    UINTN DeltaX = 0;
    UINTN DeltaY = 0;

    CHAR8 *DecimalNumberStr = NULL;
    UINTN DecimalNumberStrLength = 0;

    EFI_STATUS status = FindDesiredSimpleTextOutProtocol(&pSimpleTextOut);

    if (status != EFI_SUCCESS)
    {
        PrintError(L"Cannot find desired SimpleTextOutProtocol.\n");
        return EFI_ABORTED;
    }

    DecimalNumberStr = DecimalNumberToString(DecimalNumber);
    if (DecimalNumberStr)
    {
        DecimalNumberStrLength = AsciiStrnLenS(DecimalNumberStr, COUNTER_MAX_WIDTH);
        if (DecimalNumberStrLength == 0)
        {
            PrintError(L"Cannot convert decimal number to string.\n");
            return EFI_ABORTED;
        }

        Private = GRAPHICS_CONSOLE_CON_OUT_DEV_FROM_THIS(pSimpleTextOut);
        EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = Private->GraphicsOutput;

        UINTN RowCount = 6;
        for (UINTN i = 0; i < RowCount; i++)
        {
            Print(L"\n"); //Get some space to draw.
        }

        // Calculate start position for drawing.
        DeltaX = Private->ModeData[pSimpleTextOut->Mode->Mode].DeltaX;
        DeltaY = (pSimpleTextOut->Mode->CursorRow - RowCount) * EFI_GLYPH_HEIGHT + EFI_GLYPH_HEIGHT / 2 + Private->ModeData[pSimpleTextOut->Mode->Mode].DeltaY;

        UINTN offset = 0;
        DigitInfo *digit;

        for (INTN i = -1; i <= (INTN)DecimalNumberStrLength; i++)
        {
            if(i==-1)
            {
                digit = GetDigitInfo(DIGIT_DECORATE_L);
            }
            else if(i == (INTN)DecimalNumberStrLength)
            {
                digit = GetDigitInfo(DIGIT_DECORATE_R);
            }
            else
            {
                digit = GetDigitInfo(DecimalNumberStr[i]-0x30);
            }

            GraphicsOutput->Blt(
                GraphicsOutput,
                digit->buf,
                EfiUgaBltBufferToVideo,
                0,
                0,
                DeltaX + offset,
                DeltaY,
                *(digit->width),
                *(digit->height),
                *(digit->width) * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

            offset += *(digit->width);
        }

        return EFI_SUCCESS;
    }

    return EFI_ABORTED;
}

Counter *CounterCreate()
{
    Counter *C = (Counter *)AllocateZeroPool(sizeof(Counter));
    ASSERT(C != NULL);

    C->GetCount = CounterGetCount;
    C->AddToCount = CounterAddToCount;
    C->ClearCount = CounterClearCount;
    C->ShowCount = CounterShowCount;

    C->SetDelayTime = CounterSetDelayTime;
    C->GetDelayTime = CounterGetDelayTime;

    C->SetExportEnv = CounterSetExportEnv;
    C->GetExportEnv = CounterGetExportEnv;
    C->ExportEnv    = CounterExportEnv;

    C->ShowVersion = CounterShowVersion;
    C->ShowHelp = CounterShowHelp;
    C->Destroy = CounterDestroy;

    return C;
}

EFI_STATUS CounterGetCount(Counter *This, UINTN *Count)
{
    EFI_STATUS Status = EFI_SUCCESS;
    SHELL_FILE_HANDLE FileHandle = NULL;
    CHAR8 Buffer[64];
    INTN BufferSize = sizeof(Buffer);
    ZeroMem(Buffer, BufferSize);
    BufferSize -= 1; //Reserve a NULL Terminator for char8 string.

    ASSERT(This != NULL);
    ASSERT(Count != NULL);

    Status = ShellOpenFileByName(
        COUNTER_LOG_FILE_NAME,
        &FileHandle,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
        0);
    if (EFI_ERROR(Status))
    {
        PrintError(L"Cannot open file: %s\n", COUNTER_LOG_FILE_NAME);
        return Status;
    }

    Status = ShellReadFile(FileHandle, (UINTN *)&BufferSize, &Buffer);
    ShellCloseFile(&FileHandle);
    if (EFI_ERROR(Status))
    {
        PrintError(L"Cannot read file: %s\n", COUNTER_LOG_FILE_NAME);
        return Status;
    }

    // Remove possible CR/LF at the line end.
    while(BufferSize - 1 >= 0 
    && (Buffer[BufferSize - 1] == '\r' || Buffer[BufferSize - 1] == '\n' || Buffer[BufferSize - 1] == ' '))
    {
        Buffer[BufferSize - 1] = '\0';
        --BufferSize;
    }

    if (BufferSize <= 0)
    {
        *Count = 0;
    }
    else
    {
        if (RETURN_SUCCESS != AsciiStrDecimalToUintnS(Buffer, NULL, Count))
        {
            PrintError(L"Convert string to UINTN failed.\n");
            return EFI_INVALID_PARAMETER;
        }

        if (*Count > COUNTER_MAX_COUNT)
        {
            PrintError(L"Count overflow, max count allowed is: %lld.\n", COUNTER_MAX_COUNT);
            return EFI_INVALID_PARAMETER;
        }
    }

    if(This->GetExportEnv(This))
    {
        This->ExportEnv(This, *Count, 0);
    }

    return EFI_SUCCESS;
}

EFI_STATUS CounterAddToCount(Counter *This, UINTN i)
{
    UINTN Count = 0;
    SHELL_FILE_HANDLE FileHandle = NULL;
    CHAR8 Buffer[64];
    UINTN BufferSize = sizeof(Buffer);
    ZeroMem(Buffer, BufferSize);
    BufferSize -= 2; //Reserve for \r \n.

    ASSERT(This != NULL);
    if (i == 0) return EFI_SUCCESS;

    if (This->GetCount(This, &Count) == EFI_SUCCESS)
    {
        if (SafeUintnAdd(Count, i, &Count) == RETURN_SUCCESS)
        {
            if (Count > COUNTER_MAX_COUNT)
            {
                PrintError(L"Count overflow, max count allowed is: %lld.\n", COUNTER_MAX_COUNT);
                return EFI_ABORTED;
            }

            if (AsciiValueToStringS(Buffer, BufferSize, 0, Count, COUNTER_MAX_WIDTH) == RETURN_SUCCESS)
            {
                UINTN StrLength = AsciiStrnLenS(Buffer, BufferSize);
                EFI_SHELL_PROTOCOL*shell = NULL;
                Buffer[StrLength++] = '\r';
                Buffer[StrLength++] = '\n';

                if((shell = GetEfiShellProtocol())==NULL) return EFI_ABORTED; 

                if (shell->CreateFile(COUNTER_LOG_FILE_NAME, 0, &FileHandle) == EFI_SUCCESS)
                {
                    if (EFI_SUCCESS == ShellWriteFile(FileHandle, &StrLength, Buffer))
                    {
                        ShellFlushFile(FileHandle);
                    }
                    else
                    {
                        PrintError(L"Write to this file failed: %s\n", COUNTER_LOG_FILE_NAME);
                    }

                    ShellCloseFile(&FileHandle);

                    if(This->GetExportEnv(This))
                    {
                        This->ExportEnv(This, Count, 0);
                    }

                    return EFI_SUCCESS;
                }
                else
                {
                    PrintError(L"Cannot write file: %s\n", COUNTER_LOG_FILE_NAME);
                }
            }
            else
            {
                PrintError(L"Convert to string failed.\n");
            }
        }
        else
        {
            PrintError(L"Count add operation failed.\n");
        }
    }

    return EFI_ABORTED;
}

EFI_STATUS CounterClearCount(Counter *This)
{
    ASSERT(This != NULL);

    if(This->GetExportEnv(This))
    {
        This->ExportEnv(This, 0, 0);
    }
    else
    {
        This->ExportEnv(This, 0, 1);
    }

    if (ShellFileExists(COUNTER_LOG_FILE_NAME) == EFI_SUCCESS)
    {
        return ShellDeleteFileByName(COUNTER_LOG_FILE_NAME);
    }

    return EFI_SUCCESS;
}

EFI_STATUS CounterShowCount(Counter *This)
{
    ASSERT(This != NULL);

    UINTN Count = 0;
    UINTN DelayTime = 0;
    EFI_STATUS Status = EFI_SUCCESS;

    if (This->GetCount(This, &Count) == EFI_SUCCESS)
    {
        Status = DrawDecimalNumber(Count);

        if(Status == EFI_SUCCESS)
        {
            DelayTime = This->GetDelayTime(This);
            if(DelayTime>0)
            {
                gBS->Stall(DelayTime*1000);
            }
        }

        return Status;
    }

    return EFI_ABORTED;
}

EFI_STATUS CounterSetDelayTime(Counter* This, UINTN Milliseconds)
{
    ASSERT(This != NULL);

    if(Milliseconds < COUNTER_MAX_DELAY_TIME)
    {
        mCounterDelayTime = Milliseconds;
    }
    else
    {
        mCounterDelayTime = COUNTER_MAX_DELAY_TIME;
    }

    return EFI_SUCCESS;
}

UINTN CounterGetDelayTime(Counter* This)
{
    ASSERT(This != NULL);

    return mCounterDelayTime;
}

EFI_STATUS CounterSetExportEnv(Counter* This, BOOLEAN value)
{
    mExportEnv = value;
    return EFI_SUCCESS;
}

BOOLEAN CounterGetExportEnv(Counter* This)
{
    return mExportEnv;
}

EFI_STATUS CounterExportEnv(Counter* This, UINTN Count, BOOLEAN IsUsingDelete)
{
    EFI_SHELL_PROTOCOL*shell=NULL;
    CHAR16 Buffer[64];
    UINTN BufferSize = sizeof(Buffer);
    ZeroMem(Buffer,BufferSize);

    if((shell = GetEfiShellProtocol()) != NULL)
    {
        if(IsUsingDelete)
        {
            shell->SetEnv(COUNTER_LAST_VALUE_STR, NULL, TRUE);
            return EFI_SUCCESS;
        }
        else    
        {
            if(UnicodeValueToStringS(Buffer,BufferSize,0,Count,COUNTER_MAX_WIDTH) == RETURN_SUCCESS)
            {
                return shell->SetEnv(COUNTER_LAST_VALUE_STR, Buffer, TRUE);
            }
        }
    }

    return EFI_ABORTED;
}

EFI_STATUS CounterShowVersion(Counter *This)
{
    ASSERT(This != NULL);

    Print(L"%s\n", COUNTER_VERSION_STR);

    return EFI_SUCCESS;
}

EFI_STATUS CounterShowHelp(Counter *This)
{
    ASSERT(This != NULL);

    Print(L"counter.efi [<options>]\n");
    Print(L"\n");
    Print(L"It is used to do count under uefi shell.\n");
    Print(L"\n");
    Print(L"Options:\n");
    Print(L"  -a,--add              count add 1.\n");
    Print(L"  -s,--show             show the count.\n");
    Print(L"  -d,--delay <ms>       set delay time when show the count.\n");
    Print(L"  -e,--env              environment variable export: %s.\n", COUNTER_LAST_VALUE_STR);
    Print(L"  -c,--clear            clear and reset count to zero.\n");
    Print(L"  -h,--help             show help.\n");
    Print(L"  -v,--version          show version.\n");
    Print(L"\n");

    return EFI_SUCCESS;
}

EFI_STATUS CounterDestroy(Counter **This)
{
    if (This && *This)
    {
        FreePool(*This);
        (*This) = NULL;
    }

    return EFI_SUCCESS;
}
