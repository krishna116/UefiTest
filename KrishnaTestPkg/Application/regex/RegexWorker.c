#include "RegexWorker.h"

#include "TinyRegexLib.h"

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //global gST gBS gImageHandle
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>

RegexWorker* RegexWorkerCreate()
{
    EFI_STATUS Status;
    EFI_SHELL_PROTOCOL              *Shell = NULL;
    EFI_SHELL_PARAMETERS_PROTOCOL   *Param = NULL;
    RegexWorker* Worker = NULL;

    Status = gBS->LocateProtocol(&gEfiShellProtocolGuid, NULL, &Shell);
    if(Status != EFI_SUCCESS)
    {
        PrintError(L"[Cannot locate shell.\n");
        return NULL;
    }

    Status = gBS->HandleProtocol(gImageHandle, &gEfiShellParametersProtocolGuid, &Param);
    if(Status != EFI_SUCCESS)
    {
        PrintError(L"Cannot locate ParametersProtocol.\n");
        return NULL;
    }

    Worker = (RegexWorker*)AllocateZeroPool(sizeof(RegexWorker));
    if(Worker)
    {
        Worker->DoWork = RegexWorkerDoWork;
        Worker->SetPatternStr = RegexWorkerSetPatternStr;
        Worker->GetPatternStr = RegexWorkerGetPatternStr;
        Worker->SetOutputType = RegexWorkerSetOutputType;
        Worker->GetOutputType = RegexWorkerGetOutputType;
        Worker->Destroy = RegexWorkerDestroy;

        Worker->Private_.Shell = Shell;
        Worker->Private_.Param = Param;
        Worker->Private_.ThisOutputType = OutputTypeAll;

        ZeroMem(Worker->Private_.PatternStr, sizeof(Worker->Private_.PatternStr));
    }
    else
    {
        PrintError(L"Out of memory when create RegexWorker.\n");
    }

    return Worker;
}

/**
 * @brief Convert ucs2-str to ascii-str.
 * If return not NUll, it is user's responsibility to free it.
 * 
 * @param[in] Str       Input ucs2 str.
 * 
 * @return NOT NULL     Operation success.
 * @return NULL         Operation fail.
 */
CHAR8* UnicodeToAscii(CONST CHAR16* Str)
{
    CHAR8* AsciiBuffer = NULL;
    UINTN AsciiBufferSize = 0;

    if(Str == NULL) return NULL;

    AsciiBufferSize = StrLen(Str) + 1;
    AsciiBuffer = AllocateZeroPool(AsciiBufferSize);
    if(AsciiBuffer == NULL)
    {
        PrintError(L"UnicodeToAscii out of memory.\n");
        return NULL;
    }

    if (UnicodeStrToAsciiStrS(Str, AsciiBuffer, AsciiBufferSize) == RETURN_SUCCESS)
    {
        return AsciiBuffer;
    }
    else
    {
        PrintError(L"UnicodeToAscii convert fail.\n");
    }
    
    return NULL;
}

/**
 * @brief Convert ascii-str to ucs2-str.
 * If return not NUll, it is user's responsibility to free it.
 * 
 * @param[in] Str       Input ucs2 str.
 * 
 * @return NOT NULL     Operation success.
 * @return NULL         Operation fail.
 */
CHAR16* AsciiToUnicode(CONST CHAR8* InputStr)
{
    CHAR16* UnicodeBuffer = NULL;
    UINTN UnicodeBufferSize = 0;
    UINTN AsciiStrLength = 0;
    UINTN ConvertedSize = 0;

    if(InputStr == NULL) return NULL;

    AsciiStrLength = AsciiStrLen(InputStr);
    UnicodeBufferSize = (AsciiStrLength + 1) * sizeof(CHAR16);
    UnicodeBuffer = AllocateZeroPool(UnicodeBufferSize);
    if(UnicodeBuffer == NULL)
    {
        PrintError(L"AsciiToUnicode out of memory.\n");
        return NULL;
    }

    if(AsciiStrnToUnicodeStrS(InputStr,AsciiStrLength,UnicodeBuffer, AsciiStrLength + 1, &ConvertedSize)==RETURN_SUCCESS)
    {
        return UnicodeBuffer;
    }
    else
    {
        PrintError(L"AsciiToUnicode convert fail.\n");
    }

    return NULL;
}

EFI_STATUS OutputToShellEnv(RegexWorker* This, CONST CHAR16 *EnvName, CONST CHAR16 *EnvValue)
{
    ASSERT(This != NULL);
    ASSERT(EnvName != NULL);
    ASSERT(EnvValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    Status = This->Private_.Shell->SetEnv(EnvName, EnvValue, TRUE);

    return Status;
}

EFI_STATUS OutputToShellEnv2(RegexWorker* This, CONST CHAR16 *EnvName, UINTN EnvValue)
{
    ASSERT(This != NULL);
    ASSERT(EnvName != NULL);

    CHAR16 ValueStrBuffer[16];
    UINTN ValueStrBufferSize = sizeof(ValueStrBuffer);
    ZeroMem(ValueStrBuffer,ValueStrBufferSize);

    if(UnicodeValueToStringS(ValueStrBuffer,ValueStrBufferSize,0,(INT64)EnvValue,10)==RETURN_SUCCESS)
    {
        return OutputToShellEnv(This, EnvName, ValueStrBuffer);
    }
    else
    {
        PrintError(L"OutputToShellEnv2 -> AsciiValueToStringS fail.\n");
    }

    return EFI_ABORTED;
}

EFI_STATUS OutputToStdout(RegexWorker* This, CONST CHAR16* Str, UINTN StrByteSize)
{
    ASSERT(This != NULL);
    ASSERT(Str != NULL);
    ASSERT(StrByteSize > 0);

    EFI_STATUS Status = EFI_ABORTED;
    EFI_SHELL_PROTOCOL *Shell = This->Private_.Shell;
    EFI_SHELL_PARAMETERS_PROTOCOL *Param = This->Private_.Param;

    Shell->WriteFile(Param->StdOut, &StrByteSize, (VOID*)Str);
    Shell->FlushFile(Param->StdOut);

    return EFI_SUCCESS;
}

/**
 * @brief Output result to stdout and uefi-shell-environment-variables.
 * 
 * @param[in] This      A RegexWorker instance.
 * @param[in] Str       The str to be output.
 * @param[in] Offset    Output begins with this offset to the str.
 * @param[in] Length    Specify how many chars to be output.
 * 
 * @return EFI_STATUS   Operation success.
 * @return Others       Operation fail.
 */
EFI_STATUS Output(RegexWorker* This, CONST CHAR16* Str, UINTN Offset, UINTN Length)
{
    ASSERT(This != NULL);
    ASSERT(Str != NULL);
    ASSERT(Length > 0);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR16* OutputBuffer = NULL;
    UINTN OutputBufferSize = 0;

    //Allocate = Length + L'\r' + L'\n' + string_terminator.
    OutputBufferSize = (Length + 2 + 1) * sizeof(CHAR16);
    OutputBuffer = AllocateZeroPool(OutputBufferSize);

    if(OutputBuffer != NULL)
    {
        CopyMem(OutputBuffer, &(Str[Offset]), Length * sizeof(CHAR16));
        if((Status = OutputToShellEnv(This, REGEX_LAST_MATCH_STR, OutputBuffer)) == EFI_SUCCESS &&
           (Status = OutputToShellEnv2(This, REGEX_LAST_MATCH_STR_Offset, Offset)) == EFI_SUCCESS &&
           (Status = OutputToShellEnv2(This, REGEX_LAST_MATCH_STR_LENGTH, Length)) == EFI_SUCCESS
        )
        {
            OutputBuffer[Length] = L'\r';
            OutputBuffer[Length+1] = L'\n';
            Status = OutputToStdout(This, OutputBuffer, (Length+2)*sizeof(CHAR16));
        }
        FreePool(OutputBuffer);
    }
    else
    {
        PrintError(L"Out of memory when output.\n");
    }

    return Status;
}

/**
 * @brief Convert input PatternString and Text to Ascii-string and do the match work.
 * 
 * @param[in] This              A RegexWorker instance.
 * @param[in] PatternString     A pattern str.
 * @param[in] Text              A text to be search by the pattern str.
 * @param[out] MatchedLength    Return matched str-length if operation success.
 * 
 * @return INTN >= 0            Operation success and it is the matched-start-position of the text.
 * @return INTN < 0             Operation fail.
 */
INTN EfiRegexCompileAndMatchUcs2(RegexWorker* This, CONST CHAR16* PatternString, CONST CHAR16* Text, INTN* MatchedLength)
{
    ASSERT(This != NULL);
    ASSERT(PatternString != NULL);
    ASSERT(Text != NULL);
    ASSERT(MatchedLength != NULL);

    CHAR8* AsciiPatternStr = NULL;
    CHAR8* AsciiText = NULL;
    INTN StartPos = -1;

    AsciiPatternStr = UnicodeToAscii(PatternString);
    AsciiText = UnicodeToAscii(Text);

    if(AsciiPatternStr != NULL && AsciiText != NULL)
    {
        *MatchedLength = 0;
        StartPos = EfiRegexCompileAndMatch(AsciiPatternStr,AsciiText, MatchedLength);
    }

    if(AsciiPatternStr)FreePool(AsciiPatternStr);
    if(AsciiText)FreePool(AsciiText);

    return StartPos;
}

/**
 * @brief Read buffer from stdin.
 * Return buffer is managed by the RegexWorker instance, user cannot free it.
 * 
 * @param[in] This      A RegexWorker instance.
 * 
 * @return NOT NULL     Operation success.
 * @return NULL         Operation fail.
 */
CHAR16* ReadInputFromStdin(RegexWorker* This)
{
    ASSERT(This != NULL);

    CHAR16* BufferRef = NULL;
    UINTN BufferByteSize = sizeof(This->Private_.Text);
    ZeroMem(This->Private_.Text,BufferByteSize);
    BufferByteSize -= sizeof(CHAR16);   // Reserve a string-terminator.

    This->Private_.Shell->ReadFile(This->Private_.Param->StdIn, &BufferByteSize, This->Private_.Text);
    if(BufferByteSize > 0)
    {
        if(This->Private_.Text[0] == 0xFEFF) // Skip UCS2-BOM in front of the buffer.
        {
            BufferRef = &(This->Private_.Text[1]);
        }
        else
        {
            BufferRef = &(This->Private_.Text[0]);
        }
    }

    return BufferRef;
}

EFI_STATUS RegexWorkerDoWork(RegexWorker* This)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR16* PatternStr = NULL;
    CHAR16* Buffer= NULL;
    CHAR16* BufferRef= NULL;
    INTN StartPos = 0;
    INTN MatchedLength = 0;

    // First we should check user has input the pattern.
    PatternStr = This->GetPatternStr(This);
    ASSERT(PatternStr != NULL);
    if(PatternStr[0] == L'\0')
    {
        PrintError(L"Input pattern str is empty.\n");
    }
    else
    {
        // Read input text.
        if((Buffer = ReadInputFromStdin(This)) != NULL)
        {
            // Do match work.
            BufferRef = Buffer; //Backup buffer start position.
            BOOLEAN HasLastMatchOk = FALSE;
            INTN LastMatchOffset = 0;
            INTN LastMatchLength = 0;
            OutputType ThisOutputType = This->GetOutputType(This);
            Status = EFI_SUCCESS;

            StartPos = EfiRegexCompileAndMatchUcs2(This, PatternStr, Buffer, &MatchedLength);
            while(StartPos >=0 && MatchedLength>0 && Status == EFI_SUCCESS)
            {
                LastMatchOffset = (INTN)(Buffer - BufferRef + StartPos);
                LastMatchLength = MatchedLength;

                if(ThisOutputType == OutputTypeFirst)
                {
                    // Only output first matched item.
                    Status = Output(This, BufferRef, LastMatchOffset, LastMatchLength);
                    break;
                }
                else if(ThisOutputType == OutputTypeLast)
                {
                    // Only output last matched item.
                    HasLastMatchOk = TRUE;
                }
                else
                {
                    // Default is output all matched items.
                    Status = Output(This, BufferRef, LastMatchOffset, LastMatchLength);
                }

                // Relative offset to Buffer.
                Buffer += (StartPos + MatchedLength);
                // Try next match.
                StartPos = EfiRegexCompileAndMatchUcs2(This, PatternStr, Buffer, &MatchedLength);
            }

            if(HasLastMatchOk)
            {
                Status = Output(This, BufferRef, LastMatchOffset, LastMatchLength);
            }
        }
    }

    if(Status != EFI_SUCCESS)
    {
        //Reset our envronment variable to default value.
        OutputToShellEnv(This, REGEX_LAST_MATCH_STR, L"null");
        OutputToShellEnv2(This, REGEX_LAST_MATCH_STR_Offset, 0);
        OutputToShellEnv2(This, REGEX_LAST_MATCH_STR_LENGTH, 0);
    }

    return Status;
}

EFI_STATUS RegexWorkerSetPatternStr(RegexWorker* This, CONST CHAR16* Str)
{
    ASSERT(This != NULL);
    ASSERT(Str != NULL);

    UINTN size= StrSize(Str);
    if(size > sizeof(This->Private_.PatternStr))
    {
        PrintError(L"Input pattern size out of range, max size allowed = %ld\n", EFI_REGEX_PATTERN_STR_MAX_COUNT);
        return EFI_ABORTED;
    }

    CopyMem(This->Private_.PatternStr, Str, size);

    return EFI_SUCCESS;
}

CHAR16* RegexWorkerGetPatternStr(RegexWorker* This)
{
    ASSERT(This != NULL);
    return This->Private_.PatternStr;
}

EFI_STATUS RegexWorkerSetOutputType(RegexWorker* This, OutputType type)
{
    ASSERT(This != NULL);
    This->Private_.ThisOutputType = type;
    return EFI_SUCCESS;
}

OutputType RegexWorkerGetOutputType(RegexWorker* This)
{
    ASSERT(This != NULL);
    return This->Private_.ThisOutputType;
}

VOID RegexWorkerDestroy(RegexWorker** This)
{
    if(This && *This)
    {
        FreePool(*This);
        (*This) = NULL;
    }
}
