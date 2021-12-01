#include "history.h"
#include "Historian.h"

Historian gHistorian = {
    NULL,
    0,
    0,
    0,
    NULL,
    0,
    NULL,
    NULL,

    HistorianConstructor,
    HistorianDestructor,
    TellStory,
};

UINTN HistorianConstructor(Historian *This)
{
    EFI_STATUS status = 0;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *simpleTextProtocol = NULL;
    CONSOLE_LOGGER_PRIVATE_DATA *consoleInfo = NULL;

    /// Make sure do not construct this more than once;
    ASSERT(This != NULL);
    ASSERT(This->buffer == NULL);
    ASSERT(This->handleBuffer == NULL);

    status = gBS->LocateProtocol(&gEfiShellProtocolGuid, NULL, &(This->shell));
    if (status != EFI_SUCCESS)
    {
        PrintError(L"[Cannot locate shell.\n");
        return 0;
    }

    status = gBS->HandleProtocol(gImageHandle, &gEfiShellParametersProtocolGuid, &(This->param));
    if (status != EFI_SUCCESS)
    {
        PrintError(L"Cannot locate ParametersProtocol.\n");
        return 0;
    }

    status = gBS->LocateHandleBuffer(ByProtocol, &gEfiShellProtocolGuid, NULL, &This->handleBufferLength, &(This->handleBuffer));
    if (status == EFI_SUCCESS)
    {
        /// Assume there is only one shell instance in the system.
        status = gBS->HandleProtocol(This->handleBuffer[0], &gEfiSimpleTextOutProtocolGuid, &simpleTextProtocol);
        if (status == EFI_SUCCESS)
        {
            consoleInfo = CONSOLE_LOGGER_PRIVATE_DATA_FROM_THIS(simpleTextProtocol);
            if (consoleInfo != NULL)
            {
                This->buffer = AllocateZeroPool(consoleInfo->BufferSize);
                if (This->buffer != NULL)
                {
                    CopyMem(This->buffer, consoleInfo->Buffer, consoleInfo->BufferSize);
                    This->bufferLength = consoleInfo->BufferSize / sizeof(CHAR16);

                    This->rowsPerScreen = consoleInfo->RowsPerScreen;
                    This->colsPerScreen = consoleInfo->ColsPerScreen;

                    return 1;
                }
                else
                {
                    PrintError(L"Out of memory.\n");
                }
            }
            else
            {
                PrintError(L"Cannot locate consoleInfo.\n");
            }
        }
        else
        {
            PrintError(L"Cannot locate SimpleTextOutProtocol.\n");
        }
    }
    else
    {
        PrintError(L"Cannot get ShellHandleBuffer.\n");
    }

    return 0;
}

UINTN HistorianDestructor(Historian *This)
{
    ASSERT(This != NULL);

    if (This->buffer != NULL)
    {
        FreePool(This->buffer);
        This->buffer = NULL;
    }

    if (This->handleBuffer != NULL)
    {
        FreePool(This->handleBuffer);
        This->handleBuffer = NULL;
    }

    return 1;
}

UINTN TellStory(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine)
{
    UINTN OneColumnSize = 0;
    CHAR16 *outputPosition = NULL;

    CHAR16 *outputBuffer = NULL;
    UINTN outputBufferLength = 0;
    UINTN byteSize = 0;
    UINTN outputLineCount = 0;

    CHAR16 *start = NULL;
    CHAR16 *final = NULL;
    CHAR16 *dest = NULL;
    CHAR16 *source = NULL;

    ASSERT(This != NULL);
    ASSERT(This->buffer != NULL);
    ASSERT(This->bufferLength != 0);

    if(DesiredRowCount == 0) return 0;

    /// Every line size is [ColsPerScreen + 2] used in uefi shell.
    OneColumnSize = This->colsPerScreen + 2;
    start = This->buffer;
    while (start < (This->buffer + This->bufferLength))
    {
        if (*start == WCHAR_F && *(start + 1) == WCHAR_S)
        {
            final = start;
        }
        start += OneColumnSize;
    }

    /// The final is the last line start with 'FS',it is the position where we execute this.efi.
    if (final == NULL)
    {
        PrintError(L"Cannot locate app's current position.\n");
        return 0;
    }

    /// This->buffer is the screen's first-line's begin.
    /// final is the current line where we execute this.efi.
    if ((This->buffer + (DesiredRowCount * OneColumnSize)) > final)
    {
        /// DesiredRowCount is out of range,just output This->buffer.
        outputPosition = This->buffer;
        outputLineCount = (final - This->buffer) / OneColumnSize;
    }
    else
    {
        /// Move up to desired line begin;
        outputPosition = final - DesiredRowCount * OneColumnSize;
        outputLineCount = DesiredRowCount;
    }

    if (IsOnlyGetLastLine)
    {
        outputLineCount = 1;
    }

    outputBufferLength = outputLineCount * This->colsPerScreen;
    /// Allocate one more CHAR16 is used for string-terminator,but do not count it.
    outputBuffer = AllocateZeroPool((outputBufferLength + 1) * sizeof(CHAR16));
    if (outputBuffer != NULL)
    {
        dest = outputBuffer;
        source = outputPosition;
        byteSize = outputBufferLength * sizeof(CHAR16);

        /// Every line size is [ColsPerScreen + 2] used in uefi shell,but we exclude the last two char.
        for (UINTN line = 0; line < outputLineCount; line++)
        {
            CopyMem(dest, source, This->colsPerScreen * sizeof(CHAR16));
            dest += OneColumnSize - 2;
            source += OneColumnSize;
        }

        /// In Emulator(QEMU) the buffer may contain zero-value chars,it is not valid for output,convert them to space.
        for (UINTN i = 0; i < outputBufferLength; i++)
        {
            if (outputBuffer[i] == 0)
            {
                outputBuffer[i] = WCHAR_SPACE;
            }
        }

        if (gEfiShellParametersProtocol != NULL && gEfiShellProtocol != NULL)
        {
            gEfiShellProtocol->WriteFile(gEfiShellParametersProtocol->StdOut, &byteSize, (VOID *)outputBuffer);
            gEfiShellProtocol->FlushFile(gEfiShellParametersProtocol->StdOut);
        }
        else
        {
            Print(L"%s", outputBuffer);
        }

        if (outputBuffer != NULL)
        {
            FreePool(outputBuffer);
            outputBuffer = NULL;
        }

        return 1;
    }
    else
    {
        PrintError(L"Out of memory\n");
    }

    return 0;
}

VOID Dump(Historian *This)
{
    if (This == NULL || This->buffer == NULL)
    {
        return;
    }

    for (UINTN i = 0; i < This->bufferLength; i++)
    {
        Print(L"%02x-", This->buffer[i]);
        if ((i + 1) % (This->colsPerScreen + 2) == 0)
        {
            Print(L"z\n");
        }
    }
}
