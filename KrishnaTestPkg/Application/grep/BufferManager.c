#include "grep.h"
#include "BufferManager.h"

BufferManager gBufferManager = {
    NULL,
    0,
    NULL,

    NULL,
    NULL,

    FRONT_ITERATOR,
    FORCE_MODE,
    NON_SKIP_MODE,

    BufferManagerConstructor,
    BufferManagerDestructor,

    SetIteratorMode,
    IsFrontIteratorMode,
    SetForceMode,
    IsForceMode,
    SetSkipMode,
    IsSkipMode,

    Move,
    MoveToString,

    TrimFrontSpace,
    TrimBackSpace,
    TrimSpace,

    ScopeToChar,
    ScopeToWord,
    ScopeToLine,

    PushString,

    MatchString,
    FindString,
    IsBufferEmpty,
    GetScopeSize,

    OutPutBuffer,

    IsIteratorExceed,
    IsVisibleChar,
    LocateString
};

UINTN BufferManagerConstructor(BufferManager *This, EFI_SHELL_PARAMETERS_PROTOCOL *param)
{
    EFI_STATUS status;
    EFI_FILE_INFO *fileInfo = NULL;
    UINTN readByteSize;

    /// make sure do not construct this more than once.
    ASSERT(param != NULL);
    ASSERT(This->buffer == NULL);
    ASSERT(This->shell == NULL);

    /// 001,get ready for shell.
    status = gBS->LocateProtocol(&gEfiShellProtocolGuid, NULL, &(This->shell));
    if (status != EFI_SUCCESS)
    {
        PrintError(L"Cannot locate a shell.\n");
        return 0;
    }

    /// 001-2,file valid check.
    if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
    {
        fileInfo = gEfiShellProtocol->GetFileInfo(gEfiShellParametersProtocol->StdIn);
        if (fileInfo != NULL)
        {
            if (fileInfo->FileSize > MAX_BUFFER_SZ * sizeof(CHAR16))
            {
                PrintError(L"Input stream out of MAX_BUFFER_SZ:%d\n", MAX_BUFFER_SZ);
                return 0;
            }
        }
    }

    /// 002,Allocate extra 2+2 is reversed for output-using,usually it is reversed for CR+LF+L'\0'.
    This->bufferLength = MAX_BUFFER_SZ;
    This->buffer = AllocateZeroPool((This->bufferLength + 2 + 2) * sizeof(CHAR16));
    if (This->buffer == NULL)
    {
        PrintError(L"Out of memory.\n");
        return 0;
    }

    /// 003,pipe in the data.
    readByteSize = (This->bufferLength) * sizeof(CHAR16);
    
    /// Using global variable if possible.
    if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
    {
        status = gEfiShellProtocol->ReadFile(gEfiShellParametersProtocol->StdIn, &readByteSize, (VOID *)(This->buffer));
    }
    else
    {
        status = This->shell->ReadFile(param->StdIn, &readByteSize, (VOID *)(This->buffer));
    }
    
    if (status != EFI_SUCCESS)
    {
        if (status == EFI_BUFFER_TOO_SMALL)
        {
            PrintError(L"Input stream out of MAX_BUFFER_SZ:%d\n", MAX_BUFFER_SZ);
        }
        else
        {
            PrintError(L"Input stream failed\n");
        }

        return 0;
    }

    /// 004-1,front iterator init,we should remove the UCS_BOM in front of the buffer if exist.
    if (This->buffer[0] == WCHAR_UCS_BOM)
    {
        This->frontIterator = &(This->buffer[1]);
    }
    else
    {
        This->frontIterator = &(This->buffer[0]);
    }

    /// 004-2-1,reverse iterator init.
    This->reverseIterator = &(This->buffer[readByteSize / sizeof(CHAR16) - 1]);

    /// 004-2-2,just trim off back NewLine and Invalid chars if possible.
    while (This->frontIterator <= This->reverseIterator)
    {
        if (*(This->reverseIterator) == WCHAR_CR || *(This->reverseIterator) == WCHAR_LF || *(This->reverseIterator) == WCHAR_END)
        {
            This->reverseIterator--;
        }
        else
        {
            break;
        }
    }

    if (This->isIteratorExceed(This))
    {
        //PrintError(L"Input stream is empty.\n");
        return 0;
    }

    return 1;
}

UINTN BufferManagerDestructor(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->buffer)
    {
        FreePool(This->buffer);
        This->buffer = NULL;
        This->bufferLength = 0;
        This->reverseIterator = NULL;
        This->frontIterator = NULL;
    }

    return 1;
}

UINTN SetIteratorMode(BufferManager *This, UINTN WhichIterator)
{
    ASSERT(This != NULL);
    ASSERT(WhichIterator == FRONT_ITERATOR || WhichIterator == REVERSE_ITERATOR);

    This->mIteratorMode = WhichIterator;

    return 1;
}

UINTN IsFrontIteratorMode(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->mIteratorMode == FRONT_ITERATOR)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINTN SetForceMode(BufferManager *This, UINTN WhichMode)
{
    ASSERT(This != NULL);
    ASSERT(WhichMode == FORCE_MODE || WhichMode == NON_FORCE_MODE);

    This->mForceMode = WhichMode;

    return 1;
}

UINTN IsForceMode(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->mForceMode == FORCE_MODE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINTN SetSkipMode(BufferManager *This, UINTN WhichMode)
{
    ASSERT(This != NULL);
    ASSERT(WhichMode == SKIP_MODE || WhichMode == NON_SKIP_MODE);

    This->mSkipMode = WhichMode;

    return 1;
}

UINTN IsSkipMode(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->mSkipMode == SKIP_MODE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINTN Move(BufferManager *This, UINTN Number)
{
    ASSERT(This != NULL);

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    /// Movement is out of range,
    if (Number >= This->getScopeSize(This))
    {
        This->frontIterator = This->reverseIterator + 1;

        return 0;
    }
    else
    {
        if (This->isFrontIteratorMode(This))
        {
            This->frontIterator += Number;
        }
        else
        {
            This->reverseIterator -= Number;
        }
    }

    return 1;
}

UINTN MoveToString(BufferManager *This, CHAR16 *String, UINTN StringLen)
{
    UINTN offset = 0;
    UINTN find = 0;

    ASSERT(This != NULL);
    ASSERT(String != NULL);
    ASSERT(StringLen != 0);

    if (This->isFrontIteratorMode(This))
    {
        find = This->locateString(This, FRONT_ITERATOR, String, StringLen, &offset);
    }
    else
    {
        find = This->locateString(This, REVERSE_ITERATOR, String, StringLen, &offset);
    }

    if (find == 1)
    {
        if (This->isFrontIteratorMode(This))
        {
            This->frontIterator += offset;

            if (This->isSkipMode(This))
            {
                This->frontIterator += StringLen;
            }
        }
        else
        {
            This->reverseIterator -= offset;

            if (This->isSkipMode(This))
            {
                This->reverseIterator -= StringLen;
            }
        }
    }
    else
    {
        //PrintError(L"Cannot find this string:[%s]\n",String);
        if (This->isForceMode(This))
        {
            /// make iterators invalid;
            This->frontIterator = This->reverseIterator + 1;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

UINTN TrimFrontSpace(BufferManager *This)
{
    ASSERT(This != NULL);

    while (!(This->isIteratorExceed(This)) && !(This->isVisibleChar(This->frontIterator)))
    {
        This->frontIterator++;
    }

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    return 1;
}

UINTN TrimBackSpace(BufferManager *This)
{
    ASSERT(This != NULL);

    while (!(This->isIteratorExceed(This)) && !(This->isVisibleChar(This->reverseIterator)))
    {
        This->reverseIterator--;
    }

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    return 1;
}

UINTN TrimSpace(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->trimFrontSpace(This) == 1)
    {
        return This->trimBackSpace(This);
    }

    return 1;
}

UINTN ScopeToChar(BufferManager *This)
{
    ASSERT(This != NULL);

    /// We only need visible chars,trim off invisible chars.
    if (This->trimSpace(This) == 0)
    {
        return 0;
    }

    This->reverseIterator = This->frontIterator;

    return 1;
}

UINTN ScopeToWord(BufferManager *This)
{
    CHAR16 *tempIterator = NULL;

    ASSERT(This != NULL);

    /// We only need visible words,trim off invisible chars.
    if (This->trimSpace(This) == 0)
    {
        return 0;
    }

    tempIterator = This->frontIterator;

    while (This->isVisibleChar(tempIterator) && tempIterator <= This->reverseIterator)
    {
        tempIterator++;
    }
    This->reverseIterator = (--tempIterator);

    return 1;
}

UINTN ScopeToLine(BufferManager *This)
{
    CHAR16 *tempIterator = NULL;

    ASSERT(This != NULL);

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    tempIterator = This->frontIterator;
    while (*tempIterator != WCHAR_LF && tempIterator <= This->reverseIterator)
    {
        tempIterator++;
    }

    /// There is no WCHAR_LF,make tempIterator is stay at the scope end.
    if (tempIterator > This->reverseIterator)
    {
        tempIterator--;
    }

    /// There is a WCHAR_LF,trim off it,as we will add it back in output buffer.
    if (*tempIterator == WCHAR_LF && tempIterator >= This->frontIterator)
    {
        tempIterator--;
    }

    /// There is a WCHAR_CR,trim off it,as we will add it back in output buffer.
    if (*tempIterator == WCHAR_CR && tempIterator >= This->frontIterator)
    {
        tempIterator--;
    }

    This->reverseIterator = tempIterator;

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    return 1;
}

UINTN PushString(BufferManager *This, CHAR16 *String, UINTN StringLen)
{
    CHAR16 *newBuffer = NULL;
    CHAR16 *tempPosition = NULL;
    UINTN oldScopeSize = 0;

    ASSERT(This != NULL);
    ASSERT(This->buffer != NULL);
    ASSERT(String != NULL);
    ASSERT(StringLen != 0);

    oldScopeSize = This->getScopeSize(This);

    if ((oldScopeSize + StringLen) > This->bufferLength)
    {
        PrintError(L"Cannot append the string as buffer is out of defined-max-range:[0x%x].\n", MAX_BUFFER_SZ);

        /// make buffer invalid,so there will be nothing for output in future.
        This->frontIterator = This->reverseIterator + 1;
        return 0;
    }

    /// make sure allocate extra 2+2 CHAR16 space for output-using.
    newBuffer = AllocateZeroPool((MAX_BUFFER_SZ + 2 + 2) * sizeof(CHAR16));
    if (newBuffer == NULL)
    {
        PrintError(L"Out of memory\n");

        /// make buffer invalid,so there will be nothing for output in future.
        This->frontIterator = This->reverseIterator + 1;
        return 0;
    }

    if (This->isFrontIteratorMode(This))
    {
        /// The string will push at front of our new buffer.
        CopyMem(newBuffer, String, StringLen * sizeof(CHAR16));
        tempPosition = newBuffer + StringLen;
        CopyMem(tempPosition, This->frontIterator, oldScopeSize * sizeof(CHAR16));
    }
    else
    {
        /// The string will push at back of our new buffer.
        CopyMem(newBuffer, This->frontIterator, oldScopeSize * sizeof(CHAR16));
        tempPosition = newBuffer + oldScopeSize;
        CopyMem(tempPosition, String, StringLen * sizeof(CHAR16));
    }

    FreePool(This->buffer);
    This->buffer = newBuffer;
    This->bufferLength = MAX_BUFFER_SZ;
    This->frontIterator = This->buffer;
    This->reverseIterator = This->buffer + oldScopeSize + StringLen - 1;

    return 1;
}

UINTN MatchString(BufferManager *This, CHAR16 *String, UINTN StringLen)
{
    ASSERT(This != NULL);
    ASSERT(String != NULL);
    ASSERT(StringLen != 0);

    if (StringLen != This->getScopeSize(This))
    {
        return 0;
    }

    return LocateString(This, FRONT_ITERATOR, String, StringLen, NULL);
}

UINTN FindString(BufferManager *This, CHAR16 *String, UINTN StringLen)
{
    ASSERT(This != NULL);
    ASSERT(String != NULL);
    ASSERT(StringLen != 0);

    if (StringLen > This->getScopeSize(This))
    {
        return 0;
    }

    return This->locateString(This, FRONT_ITERATOR, String, StringLen, NULL);
}

UINTN IsBufferEmpty(BufferManager *This)
{
    if (This->trimSpace(This) == 0)
    {
        return 1;
    }

    return 0;
}

UINTN OutPutBuffer(BufferManager *This)
{
    UINTN scopeSize = 0;
    CHAR16 *buffer = NULL;
    UINTN bufferByteSize = 0;
    UINTN returnCode = 0;

    ASSERT(This != NULL);
    ASSERT(This->buffer != NULL);

    if ((scopeSize = This->getScopeSize(This)) > 0)
    {
        if (*(This->reverseIterator) != WCHAR_LF)
        {
            *(++This->reverseIterator) = WCHAR_CR;
            *(++This->reverseIterator) = WCHAR_LF;
        }

        bufferByteSize = This->getScopeSize(This) * sizeof(CHAR16);
        /// Allocate another sizeof(CHAR16) is used for string terminator.
        buffer = AllocateZeroPool(bufferByteSize + sizeof(CHAR16));
        if (buffer != NULL)
        {
            CopyMem(buffer, This->frontIterator, bufferByteSize);
            if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
            {
                /// Pipe out result.
                gEfiShellProtocol->WriteFile(gEfiShellParametersProtocol->StdOut, &bufferByteSize, (void *)(buffer));
                gEfiShellProtocol->FlushFile(gEfiShellParametersProtocol->StdOut);
            }
            else
            {
                /// Normal print.
                Print(L"%s\n", buffer);
            }

            FreePool(buffer);
            buffer = NULL;
            returnCode = 1;
        }
    }

    return returnCode;
}

UINTN IsIteratorExceed(BufferManager *This)
{
    ASSERT(This != NULL);
    ASSERT(This->frontIterator != NULL);
    ASSERT(This->reverseIterator != NULL);

    if (This->frontIterator < This->buffer || This->reverseIterator > (This->buffer + This->bufferLength - 1))
    {
        return 1;
    }

    if (This->frontIterator > This->reverseIterator)
    {
        return 1;
    }

    return 0;
}

UINTN GetScopeSize(BufferManager *This)
{
    ASSERT(This != NULL);

    if (This->isIteratorExceed(This))
    {
        return 0;
    }

    return (This->reverseIterator - This->frontIterator + 1);
}

UINTN IsVisibleChar(CHAR16 *C)
{
    UINTN returnCode = 0;

    ASSERT(C != NULL);

    if (*C >= WCHAR_VISIBLE_BEGIN)
    {
        returnCode = 1;

#ifdef WCHAR_VISIBLE_END
        if(!(*C<=WCHAR_VISIBLE_END)
        {
            returnCode = 0;
        }
#endif
    }

    return returnCode;
}

UINTN LocateString(BufferManager *This, UINTN WhichIterator, CHAR16 *String, UINTN StringLen, UINTN *Offset)
{
    UINTN index = 0;
    UINTN find = 0;
    CHAR16 *tempIterator = NULL;

    ASSERT(This != NULL);
    ASSERT(String != NULL);
    ASSERT(StringLen != 0);

    if (WhichIterator == FRONT_ITERATOR)
    {
        tempIterator = This->frontIterator;
        while (!This->isIteratorExceed(This) && (This->reverseIterator + 1) >= (StringLen + tempIterator))
        {
            if (CompareMem(tempIterator, String, StringLen * sizeof(CHAR16)) == 0)
            {
                find = 1;
                break;
            }
            tempIterator++;
            index++;
        }
    }
    else
    {
        tempIterator = This->reverseIterator;
        while (!This->isIteratorExceed(This) && (tempIterator + 1) >= (StringLen + This->frontIterator))
        {
            if (CompareMem(tempIterator - StringLen + 1, String, StringLen * sizeof(CHAR16)) == 0)
            {
                find = 1;
                break;
            }
            tempIterator--;
            index++;
        }
    }

    if (find == 1)
    {
        if (Offset != NULL)
        {
            *Offset = index;
        }
        return 1;
    }

    return 0;
}
