#include "history.h"
#include "Historian.h"

#define WCHAR_LF                    0x000A
#define WCHAR_CR                    0x000D
#define WCHAR_F                     0x0046
#define WCHAR_S                     0x0053
#define WCHAR_SPACE                 0x0020
#define WCHAR_BS                    0x0008

EFI_STATUS HistorianInitParam(Historian *This)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    if(gBS->HandleProtocol(gImageHandle, &gEfiShellParametersProtocolGuid, &(This->Private_.Param)) == EFI_SUCCESS)
    {
        Status = EFI_SUCCESS;
    }
    else
    {
        PrintError(L"Cannot locate ParametersProtocol.\n");
    }

    return Status;
}

EFI_STATUS HistorianInitScreenBuffer(Historian *This)
{
    ASSERT(This != NULL);
    ASSERT(This->Private_.ScreenBuffer == NULL);

    EFI_STATUS Status = EFI_ABORTED;
    EFI_HANDLE* ShellHandleBuffer=NULL;
    UINTN ShellHandleBufferLength = 0;
    CONSOLE_LOGGER_PRIVATE_DATA *ConsoleInfo = NULL;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleTextProtocol = NULL;

    if(gBS->LocateHandleBuffer(ByProtocol, &gEfiShellProtocolGuid, NULL, &ShellHandleBufferLength, &ShellHandleBuffer) == EFI_SUCCESS)
    {
        // Just try to get simple text protocol from first shell handle.
        if(gBS->HandleProtocol(ShellHandleBuffer[0], &gEfiSimpleTextOutProtocolGuid, &SimpleTextProtocol) == EFI_SUCCESS)
        {
            ConsoleInfo = CONSOLE_LOGGER_PRIVATE_DATA_FROM_THIS(SimpleTextProtocol);
            This->Private_.ScreenBuffer = AllocateZeroPool(ConsoleInfo->BufferSize);
            if (This->Private_.ScreenBuffer != NULL)
            {
                CopyMem(This->Private_.ScreenBuffer, ConsoleInfo->Buffer, ConsoleInfo->BufferSize);
                This->Private_.ScreenBufferLength = ConsoleInfo->BufferSize / sizeof(CHAR16);
                This->Private_.RowsPerScreen = ConsoleInfo->RowsPerScreen;
                This->Private_.ColsPerScreen = ConsoleInfo->ColsPerScreen;
                Status = EFI_SUCCESS;
            }
            else
            {
                PrintError(L"Out of memory.\n");
            }
        }
        else
        {
            PrintError(L"Cannot locate SimpleTextOutProtocol.\n");
        }
        FreePool(ShellHandleBuffer);
    }
    else
    {
        PrintError(L"Cannot locate all shell handles.\n");
    }

    return Status;
}

Historian* HistorianCreate()
{
    EFI_STATUS Status = EFI_ABORTED;
    Historian* ThisHistorian = NULL;

    if((ThisHistorian = AllocateZeroPool(sizeof(Historian))) != NULL)
    {
        ThisHistorian->Private_.Param = NULL;
        if(HistorianInitParam(ThisHistorian) == EFI_SUCCESS)
        {
            ThisHistorian->Private_.ScreenBuffer = NULL;
            if(HistorianInitScreenBuffer(ThisHistorian) == EFI_SUCCESS)
            {
                ThisHistorian->TellStory = HistorianTellStory;
                ThisHistorian->Destroy = HistorianDestroy;

                Status = EFI_SUCCESS;
            }
        }
    }

    if(Status != EFI_SUCCESS) HistorianDestroy(&ThisHistorian);

    return ThisHistorian;
}

EFI_STATUS HistorianTellStory(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine)
{
    EFI_STATUS Status = EFI_ABORTED;

    UINTN OneColumnLineSize = 0;
    CHAR16 *OutputPosition = NULL;

    CHAR16 *OutputBuffer = NULL;
    UINTN OutputBufferLength = 0;
    UINTN OutputBufferByteSize = 0;
    UINTN OutputLineCount = 0;

    CHAR16 *Start = NULL;
    CHAR16 *Final = NULL;
    CHAR16 *Dest = NULL;
    CHAR16 *Source = NULL;

    ASSERT(This != NULL);

    if(This->Private_.ScreenBuffer == NULL || 
       This->Private_.ScreenBufferLength == 0 ||
       DesiredRowCount == 0
    )
    {
        return Status;
    }

    // Every line size is [ColsPerScreen + 2] used in uefi shell.
    OneColumnLineSize = This->Private_.ColsPerScreen + 2;
    Start = This->Private_.ScreenBuffer;
    while (Start < (This->Private_.ScreenBuffer +This->Private_.ScreenBufferLength))
    {
        // Try to locate current position.
        if (*Start == WCHAR_F && *(Start + 1) == WCHAR_S)
        {
            Final = Start;
        }
        Start += OneColumnLineSize;
    }

    // Check current position.
    if (Final == NULL)
    {
        PrintError(L"Cannot locate app's current position.\n");
        return Status;
    }

    // This->buffer is the screen's first-line's begin.
    // Final is the current line where we execute this.efi.
    if ((This->Private_.ScreenBuffer + (DesiredRowCount * OneColumnLineSize)) > Final)
    {
        /// DesiredRowCount is out of range,just output the present screen buffer.
        OutputPosition = This->Private_.ScreenBuffer;
        OutputLineCount = (Final - This->Private_.ScreenBuffer) / OneColumnLineSize;
    }
    else
    {
        // Move up to desired line begin;
        OutputPosition = Final - DesiredRowCount * OneColumnLineSize;
        OutputLineCount = DesiredRowCount;
    }

    if (IsOnlyGetLastLine) OutputLineCount = 1;

    OutputBufferLength = OutputLineCount * This->Private_.ColsPerScreen;
    // Allocate one more CHAR16 is used for string-terminator,but do not count it.
    OutputBuffer = AllocateZeroPool((OutputBufferLength + 1) * sizeof(CHAR16));
    if (OutputBuffer != NULL)
    {
        Dest = OutputBuffer;
        Source = OutputPosition;
        OutputBufferByteSize = OutputBufferLength * sizeof(CHAR16);

        // Every line size is OneColumnLineSize(ColsPerScreen + 2) used in uefi shell,
        // We should exclude the last two char.
        for (UINTN line = 0; line < OutputLineCount; line++)
        {
            CopyMem(Dest, Source, This->Private_.ColsPerScreen * sizeof(CHAR16));
            Dest += This->Private_.ColsPerScreen;
            Source += OneColumnLineSize;
        }

        // In Emulator(QEMU) the buffer may contain zero-value chars, 
        // it is not valid for output, convert them to space.
        for (UINTN i = 0; i < OutputBufferLength; i++)
        {
            if (OutputBuffer[i] == 0) OutputBuffer[i] = WCHAR_SPACE;
        }

        if (gEfiShellProtocol != NULL)
        {
            gEfiShellProtocol->WriteFile(This->Private_.Param->StdOut, &OutputBufferByteSize, (VOID *)OutputBuffer);
            gEfiShellProtocol->FlushFile(This->Private_.Param->StdOut);
            Status = EFI_SUCCESS;
        }
        else
        {
            PrintError(L"Need a shell instance to output.\n");
        }

        FreePool(OutputBuffer);
    }
    else
    {
        PrintError(L"Out of memory\n");
    }

    return Status;
}

VOID HistorianDestroy(Historian **This)
{
    if(This && *This)
    {
        if((*This)->Private_.ScreenBuffer)
        {
            FreePool((*This)->Private_.ScreenBuffer);
            (*This)->Private_.ScreenBuffer = NULL;
        }

        FreePool(*This);
        *This = NULL;
    }
}
