#include "Xdb.h"

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

STATIC CHAR16 mDefaultInputFileName[] = XDB_DEFAULT_INPUT_FILE;
STATIC CONST CHAR8 mDefaultNullAsciiString[] = XDB_DEFAULT_NULL_STR;

EFI_SHELL_PROTOCOL *GetEfiShellProtocol()
{
    EFI_STATUS Status;
    EFI_SHELL_PROTOCOL *Shell = NULL;

    Status = gBS->LocateProtocol(
        &gEfiShellProtocolGuid,
        NULL,
        (VOID **)&Shell);
    if (EFI_ERROR(Status))
    {
        PrintError(L"Cannot get shell protocol.\n");
        Shell = NULL;
    }

    return Shell;
}

Xdb *XdbCreate()
{
    EFI_SHELL_PROTOCOL *CurrentShell = GetEfiShellProtocol();
    if (CurrentShell == NULL)
    {
        PrintError("cannot find Efi shell protocol.\n");
        return NULL;
    }

    Xdb *X = (Xdb *)AllocateZeroPool(sizeof(Xdb));
    if (X != NULL)
    {
        X->Set =            XdbSet;
        X->Get =            XdbGet;
        X->Push =           XdbPush;
        X->Pop =            XdbPop;
        X->Append =         XdbAppend;
        X->Clear =          XdbClear;
        X->SetFileName =    XdbSetFileName;
        X->GetFileName =    XdbGetFileName;
        X->SetVerboseMode = XdbSetVerboseMode;
        X->GetVerboseMode = XdbGetVerboseMode;
        X->Destroy =        XdbDestroy;

        X->private_.CreateDefaultJsonFile = XdbCreateDefaultJsonFile;
        X->private_.LoadJson =              XdbLoadJson;
        X->private_.CheckJson =             XdbCheckJson;
        X->private_.SaveJson =              XdbSaveJson;
        X->private_.UpdateEnvVariable =     XdbUpdateEnvVariable;
        X->private_.UnicodeToAscii =        XdbUnicodeToAscii;
        X->private_.AsciiToUnicode =        XdbAsciiToUnicode;
        X->private_.CopyString =            XdbCopyString;

        X->private_.InternalSet =           XdbInternalSet;
        X->private_.InternalGet =           XdbInternalGet;
        X->private_.InternalPush =          XdbInternalPush;
        X->private_.InternalClear =         XdbInternalClear;
        X->private_.DumpKeyValue =          XdbDumpKeyValue;
        X->private_.FindKey =               XdbFindKey;

        X->private_.RootArrayRemoveByKey =                  XdbRootArrayRemoveByKey;
        X->private_.RootArrayRemoveItemByIndex =            XdbRootArrayRemoveItemByIndex;
        X->private_.RootArrayGetKeyValueByKey =             XdbRootArrayGetKeyValueByKey;
        X->private_.RootArrayGetKeyValueByIndex =           XdbRootArrayGetKeyValueByIndex;
        X->private_.RootArrayUpdate =                       XdbRootArrayUpdate;
        X->private_.RootArrayInsert =                       XdbRootArrayInsert;
        X->private_.ExtractKeyAndValueFromJsonKeyValue =    XdbExtractKeyAndValueFromJsonKeyValue;

        X->private_.FileName =      NULL;
        X->private_.Shell =         CurrentShell;
        X->private_.IsVerboseMode = 1;
    }
    else
    {
        PrintError("Out of memory when create xdb.\n");
    }

    return X;
}

EFI_STATUS XdbSet(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* InputKey = This->private_.UnicodeToAscii(This, Key);
    CHAR8* InputValue = This->private_.UnicodeToAscii(This, Value);

    if(InputKey && InputValue)
    {
        Status = This->private_.InternalSet(This,InputKey,InputValue);
    }

    if(InputKey) FreePool(InputKey);
    if(InputValue) FreePool(InputValue);

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_KEY, InputKey);
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }

    return Status;
}

EFI_STATUS XdbGet(Xdb *This, CONST CHAR16 *Key)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* InputKey = This->private_.UnicodeToAscii(This, Key);

    if(InputKey != NULL)
    {
        Status = This->private_.InternalGet(This, InputKey);
        FreePool(InputKey);
    }

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_KEY, InputKey);
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }

    return Status;
}

EFI_STATUS XdbPush(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value, BOOLEAN isPushFront)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* InputKey = This->private_.UnicodeToAscii(This, Key);
    CHAR8* InputValue = This->private_.UnicodeToAscii(This, Value);

    if(InputKey && InputValue)
    {
        Status = This->private_.InternalPush(This,InputKey,InputValue, isPushFront);
    }

    if(InputKey) FreePool(InputKey);
    if(InputValue) FreePool(InputValue);

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_KEY, mDefaultNullAsciiString);
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }
    return Status;
}

EFI_STATUS XdbPop(Xdb *This, BOOLEAN isPopFront)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    EDKII_JSON_VALUE Root = This->private_.LoadJson(This);

    if (Root)
    {
        UINTN ArraySize = JsonArrayCount(Root);
        if (ArraySize > 0)
        {
            Status =XdbRootArrayRemoveItemByIndex(This, Root, (isPopFront ? (0) : (ArraySize - 1)));
        }
        else
        {
            PrintError(L"Database is empty and nothing can be poped.\n");
        }

        JsonDecreaseReference(Root);
    }

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_KEY, mDefaultNullAsciiString);
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }

    return Status;
}

EFI_STATUS XdbAppend(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value)
{
    ASSERT(This != NULL);
    ASSERT(Key != NULL);
    ASSERT(Value != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* InputKey = NULL;
    CHAR8* InputValue = NULL;

    This->private_.CreateDefaultJsonFile(This, 0);
    EDKII_JSON_VALUE Root = XdbOpenFileAndReadJson(This);
    if(Root)
    {
        InputKey = This->private_.UnicodeToAscii(This, Key);
        InputValue = This->private_.UnicodeToAscii(This, Value);
        
        if(InputKey && InputValue)
        {
            EDKII_JSON_VALUE NewJsonKeyValue = JsonValueInitObject();
            if (NewJsonKeyValue)
            {
                EDKII_JSON_VALUE NewValue = JsonValueInitAsciiString(InputValue);
                if (NewValue)
                {
                    if (JsonObjectSetValue(NewJsonKeyValue, InputKey, NewValue) == EFI_SUCCESS)
                    {
                        Status = JsonArrayAppendValue(Root,NewJsonKeyValue);
                        if(Status != EFI_SUCCESS)
                        {
                            PrintError(L"Append key-value failed.\n");
                        }
                        else
                        {
                            Status = XdbSaveJson(This,Root);
                            if(Status == EFI_SUCCESS)
                            {
                                XdbDumpKeyValue(This,"+",InputKey,InputValue );
                            }
                        }
                    }
                    else
                    {
                        PrintError(L"Update object->kv failed.\n");
                    }
                    JsonDecreaseReference(NewValue);
                }
                else
                {
                    PrintError(L"Create object->kv->value failed.\n");
                }
                JsonDecreaseReference(NewJsonKeyValue);
            }
            else
            {
                PrintError(L"Create object->kv failed.\n");
            }
        }

        JsonDecreaseReference(Root);
        if(InputKey) FreePool(InputKey);
        if(InputValue) FreePool(InputValue);
    }

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_KEY, InputKey);
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }

    return Status;
}

EFI_STATUS XdbSetFileName(Xdb *This, CONST CHAR16 *FileName)
{
    ASSERT(This != NULL);
    ASSERT(FileName != NULL);

    // Remove old filename.
    if (This->private_.FileName != NULL)
    {
        FreePool(This->private_.FileName);
        This->private_.FileName = NULL;
    }

    // Copy new filename.
    UINTN FileNameSize = StrSize(FileName);
    This->private_.FileName = AllocateCopyPool(FileNameSize, FileName);
    if (This->private_.FileName == NULL)
    {
        PrintError("Out of memory when set input filename.\n");
        return EFI_ABORTED;
    }

    return EFI_SUCCESS;
}

CHAR16 *XdbGetFileName(Xdb *This)
{
    ASSERT(This != NULL);

    return This->private_.FileName;
}

EFI_STATUS XdbClear(Xdb *This, CONST CHAR16 *Key)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* InputKey = This->private_.UnicodeToAscii(This, Key);

    if(InputKey != NULL)
    {
        Status = This->private_.InternalClear(This, InputKey);
        FreePool(InputKey);
    }

    return Status;
}

EFI_STATUS XdbSetVerboseMode(Xdb *This, BOOLEAN Value)
{
    ASSERT(This != NULL);
    This->private_.IsVerboseMode = Value;
    return EFI_SUCCESS;
}

BOOLEAN XdbGetVerboseMode(Xdb *This)
{
    ASSERT(This != NULL);
    return This->private_.IsVerboseMode;
}

VOID XdbDestroy(Xdb **This)
{
    if (This && *This)
    {
        if ((*This)->private_.FileName)
        {
            FreePool((*This)->private_.FileName);
            (*This)->private_.FileName = NULL;
        }
        FreePool(*This);
        (*This) = NULL;
    }
}

VOID XdbCreateDefaultJsonFile(Xdb *This, BOOLEAN IsForce)
{
    STATIC CHAR8 JsonBuffer[] = "[] \n";

    EFI_STATUS Status;
    SHELL_FILE_HANDLE FileHandle;
    CONST CHAR16 *FileName = NULL;
    EFI_SHELL_PROTOCOL *Shell;
    BOOLEAN needCreateNewJson = FALSE;

    ASSERT(This != NULL);

    Shell = This->private_.Shell;
    ASSERT(Shell != NULL);

    FileName = This->GetFileName(This);
    if (FileName == NULL)
    {
        FileName = mDefaultInputFileName;
    }

    if (IsForce == 0)
    {
        Status = Shell->OpenFileByName(FileName, &FileHandle, EFI_FILE_MODE_READ);
        if (Status == EFI_NOT_FOUND)
        {
            needCreateNewJson = TRUE;
        }
        else if (Status == EFI_SUCCESS)
        {
            UINT64 FileSize = 0;
            Shell->GetFileSize(FileHandle, &FileSize);
            if (FileSize < 2)
            {
                needCreateNewJson = TRUE;
            }
            Shell->CloseFile(FileHandle);
        }
    }

    if (needCreateNewJson || IsForce)
    {
        if ((Status = Shell->CreateFile(FileName, 0, &FileHandle)) == EFI_SUCCESS)
        {
            UINTN JsonBufferSize = AsciiStrLen(JsonBuffer);
            Shell->WriteFile(FileHandle, &JsonBufferSize, JsonBuffer);
            Shell->FlushFile(FileHandle);
            Shell->CloseFile(FileHandle);
        }
        else
        {
            PrintError(L"Create new file[%s] failed..\n", FileName);
        }
    }
}

EDKII_JSON_VALUE XdbOpenFileAndReadJson(Xdb *This)
{
    ASSERT(This != NULL);

    EDKII_JSON_ERROR Error;
    EDKII_JSON_VALUE root = NULL;
    SHELL_FILE_HANDLE FileHandle;
    CONST CHAR16 *FileName = NULL;

    if ((FileName =This->GetFileName(This)) == NULL)
    {
        FileName = mDefaultInputFileName;
    }

    if (This->private_.Shell->OpenFileByName(FileName, &FileHandle, EFI_FILE_MODE_READ) == EFI_SUCCESS)
    {
        CHAR8 *FileBuffer = NULL;
        UINT64 FileSize = 0;
        This->private_.Shell->GetFileSize(FileHandle, &FileSize);

        if (0 < FileSize && FileSize <= XDB_INPUT_FILE_SIZE_MAX)
        {
            FileBuffer = (CHAR8 *)AllocateZeroPool(XDB_INPUT_FILE_SIZE_MAX);
            if (FileBuffer)
            {
                if (This->private_.Shell->ReadFile(FileHandle, &FileSize, FileBuffer) == EFI_SUCCESS)
                {
                    root = JsonLoadBuffer(FileBuffer, FileSize, 0, &Error);
                    if (root == NULL)
                    {
                        PrintError(L"Error load and parse Json from file: %s\n", FileName);
                    }
                }
                else
                {
                    PrintError(L"Error happened when read file: %s.\n", FileName);
                }

                FreePool(FileBuffer);
            }
            else
            {
                PrintError(L"Out of memory when read file: %s.\n", FileName);
            }
        }
        else
        {
            if (FileSize == 0)
            {
                PrintError(L"Input file is empty.\n");
            }
            else
            {
                PrintError(L"Input file is too big, max size allowed = %ld.\n", XDB_INPUT_FILE_SIZE_MAX);
            }
        }

        This->private_.Shell->CloseFile(FileHandle);
    }
    else
    {
        PrintError(L"Cannot open file to read: %s\n", FileName);
    }

    return root;
}

EDKII_JSON_VALUE XdbLoadJson(Xdb *This)
{

    EDKII_JSON_VALUE root = XdbOpenFileAndReadJson(This);

    if (root)
    {
        // Full check the json.
        if (This->private_.CheckJson(This, root))
        {
            return root;
        }
        else
        {
            PrintError(L"Invald json format.\n");
            JsonDecreaseReference(root);
            root = NULL;
        }
    }

    return root;
}

BOOLEAN XdbCheckJson(Xdb *This, EDKII_JSON_VALUE Root)
{
    ASSERT(This != NULL);
    BOOLEAN IsOk = TRUE;

    if (Root == NULL)
        return FALSE;

    if (JsonValueIsArray(Root))
    {
        for (UINTN i = 0; i < JsonArrayCount(Root); i++)
        {
            EDKII_JSON_VALUE *Object = JsonArrayGetValue(Root, i);

            if (JsonValueIsObject(Object))
            {
                VOID *It = JsonObjectIterator(Object);
                if (It)
                {
                    CONST CHAR8 *Key = JsonObjectIteratorKey(It);
                    if (Key != NULL)
                    {
                        EDKII_JSON_VALUE *Value = JsonObjectIteratorValue(It);
                        if (JsonValueIsString(Value))
                        {
                            IsOk = TRUE;
                        }
                        else
                        {
                            PrintError("json->array->object[%d]->value, it is not string.\n", i);
                            IsOk = FALSE;
                            break;
                        }
                    }
                    else
                    {
                        PrintError("json->array->object[%d]->key, it is empty.\n", i);
                        IsOk = FALSE;
                        break;
                    }
                }
                else
                {
                    PrintError("json->array->object[%d], it is empty.\n", i);
                    IsOk = FALSE;
                    break;
                }
            }
            else
            {
                PrintError("json->array->object[%d], it is not an object.\n", i);
                IsOk = FALSE;
                break;
            }
        } //for each in array.
    }
    else
    {
        PrintError("json->array, it is not an array.\n");
        IsOk = FALSE;
    }

    return IsOk;
}

EFI_STATUS XdbSaveJson(Xdb *This, EDKII_JSON_VALUE Root)
{
    ASSERT(This != NULL);
    ASSERT(Root != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    EFI_SHELL_PROTOCOL *shell = This->private_.Shell;
    CHAR16 *FileName = This->GetFileName(This);
    EFI_FILE_HANDLE FileHandle;
    CHAR8 *str = NULL;
    UINTN size = 0;

    if (FileName == NULL)
    {
        FileName = mDefaultInputFileName;
    }

    str = JsonDumpString(Root, EDKII_JSON_INDENT(2) | EDKII_JSON_ENSURE_ASCII);

    if (str)
    {
        size = AsciiStrLen(str);
        if (size > 0)
        {
            if (shell->CreateFile(FileName, 0, &FileHandle) == EFI_SUCCESS)
            {
                //resize file to zero.
                EFI_FILE_INFO* FileInfo=shell->GetFileInfo(FileHandle);
                if(FileInfo)
                {
                    FileInfo->FileSize=0;
                    shell->SetFileInfo(FileHandle,FileInfo);
                    FreePool(FileInfo);
                }

                //There may be bug, may be we should close and reopen it to make resize-file take effect.
                shell->CloseFile(FileHandle);
                if(shell->CreateFile(FileName, 0, &FileHandle) == EFI_SUCCESS)
                {
                    if (EFI_SUCCESS == shell->WriteFile(FileHandle, &size, str))
                    {
                        Status = shell->FlushFile(FileHandle);
                    }
                    else
                    {
                        PrintError(L"XdbSaveJson -> Write to this file failed: %s\n", FileName);
                    }
                    shell->CloseFile(FileHandle);
                }
            }
            else
            {
                PrintError(L"XdbSaveJson -> Create file[%s] failed.\n", FileName);
            }
        }
        else
        {
            PrintError(L"Dump json string is an empty string.\n");
        }

        FreePool(str);
    }
    else
    {
        PrintError(L"dump json string failed.\n");
    }

    return Status;
}

EFI_STATUS XdbUpdateEnvVariable(Xdb *This, CONST CHAR8 *EnvName, CONST CHAR8 *EnvValue)
{
    ASSERT(This != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    CHAR16* FinalEnvName = This->private_.AsciiToUnicode(This, EnvName);
    CHAR16* FinalEnvValue = This->private_.AsciiToUnicode(This, EnvValue);

    if(FinalEnvName && FinalEnvValue)
    {
        Status = This->private_.Shell->SetEnv(FinalEnvName, FinalEnvValue, TRUE);
    }

    if(FinalEnvName) FreePool(FinalEnvName);
    if(FinalEnvValue) FreePool(FinalEnvValue);

    return Status;
}

CHAR8* XdbUnicodeToAscii(Xdb* This, CONST CHAR16* Str)
{
    CHAR8* AsciiBuffer = NULL;
    UINTN AsciiBufferSize = 0;

    ASSERT(This != NULL);
    if(Str == NULL) return NULL;

    AsciiBufferSize = StrLen(Str);
    if(AsciiBufferSize>XDB_TOKEN_SIZE_MAX)
    {
        PrintError(L"Input token size out of range, max size allowed is %d\n",XDB_TOKEN_SIZE_MAX);
        return NULL;
    }

    AsciiBufferSize += 1;
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

CHAR16* XdbAsciiToUnicode(Xdb* This, CONST CHAR8* Str)
{
    CHAR16* UnicodeBuffer = NULL;
    UINTN AsciiStrLength = 0;
    UINTN UnicodeBufferSize = 0;

    ASSERT(This != NULL);
    if(Str == NULL) return NULL;

    AsciiStrLength = AsciiStrLen(Str);
    if(AsciiStrLength>XDB_TOKEN_SIZE_MAX)
    {
        PrintError(L"Input token size out of range, max size allowed is %d\n",XDB_TOKEN_SIZE_MAX);
        return NULL;
    }

    UnicodeBufferSize = (AsciiStrLength + 1) * sizeof(CHAR16);
    UnicodeBuffer = AllocateZeroPool(UnicodeBufferSize);
    if(UnicodeBuffer == NULL)
    {
        PrintError(L"AsciiToUnicode out of memory.\n");
        return NULL;
    }

    UINTN CurrentSize = 0;
    if(AsciiStrnToUnicodeStrS(Str,AsciiStrLength,UnicodeBuffer, AsciiStrLength + 1, &CurrentSize)==RETURN_SUCCESS)
    {
        return UnicodeBuffer;
    }
    else
    {
        PrintError(L"AsciiToUnicode convert fail.\n");
    }

    return NULL;
}

EFI_STATUS XdbCopyString(Xdb* This, CONST CHAR8* InString, CHAR8**OutString)
{
    ASSERT(This != NULL);
    ASSERT(InString != NULL);
    ASSERT(OutString != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    *OutString = AllocateCopyPool(AsciiStrSize(InString), InString);
    if(*OutString)
    {
        Status = EFI_SUCCESS;
    }
    else
    {
        PrintError("Out of memory when copy string.\n");
    }

    return EFI_SUCCESS;
}

EFI_STATUS XdbInternalSet(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value)
{
    ASSERT(This != NULL);
    ASSERT(Key != NULL);
    ASSERT(Value != NULL);

   // Load json.
    EFI_STATUS Status = EFI_ABORTED;
    This->private_.CreateDefaultJsonFile(This, 0);
    EDKII_JSON_VALUE Root = This->private_.LoadJson(This);

    if (Root)
    {
        UINTN Index = 0;
        if(This->private_.FindKey(This, Root, Key, &Index))
        {
            Status = XdbRootArrayUpdate(This, Root,Key,Value);
        }
        else
        {
            Status = XdbRootArrayInsert(This, Root, Key, Value, JsonArrayCount(Root));
        }
        JsonDecreaseReference(Root);
    }

    return Status;
}

EFI_STATUS XdbInternalGet(Xdb *This, CONST CHAR8 *Key)
{
    ASSERT(This != NULL);
    ASSERT(Key != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    EDKII_JSON_VALUE Root = This->private_.LoadJson(This);
    if (Root)
    {
        CHAR8* TempKey = NULL;
        CHAR8* TempValue = NULL;

        if((Status = XdbRootArrayGetKeyValueByKey(This, Root, Key, &TempKey, &TempValue)) == EFI_SUCCESS)
        {
            This->private_.DumpKeyValue(This, "g", TempKey, TempValue);
        }

        if(TempKey) FreePool(TempKey);
        if(TempValue) FreePool(TempValue);
        JsonDecreaseReference(Root);
    }

    if(Status != EFI_SUCCESS)
    {
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_KEY, Key);
        This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
    }

    return Status;
}

EFI_STATUS XdbInternalPush(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value, BOOLEAN isPushFront)
{
    ASSERT(This != NULL);
    ASSERT(Key != NULL);
    ASSERT(Value != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    UINTN Index = 0;
    This->private_.CreateDefaultJsonFile(This, 0);
    EDKII_JSON_VALUE Root = This->private_.LoadJson(This);

    if (Root)
    {
        if (!This->private_.FindKey(This, Root, Key, &Index))
        {
            UINTN ArraySize = JsonArrayCount(Root);
            if(isPushFront)
            {
                Index = 0;
            }else
            {
               Index = ArraySize;
            }
            Status = XdbRootArrayInsert(This, Root,Key,Value,Index);
        }
        else
        {
            PrintError(L"Cannot push duplicated-key!\n");
        }
        JsonDecreaseReference(Root);
    }

    return Status;
}

EFI_STATUS XdbInternalClear(Xdb *This, CONST CHAR8 *Key)
{
    ASSERT(This != NULL);
    ASSERT(Key != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    if (AsciiStrLen(Key) == 0)
    {
        This->private_.Shell->DeleteFileByName((This->GetFileName(This) != NULL) ? This->GetFileName(This) : mDefaultInputFileName);
        
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_KEY, mDefaultNullAsciiString);
        This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
        Status = EFI_SUCCESS;
    }
    else
    {
        // Clear one item.
        EDKII_JSON_VALUE Root = This->private_.LoadJson(This);
        if (Root)
        {
            UINTN Index = 0;
            if(This->private_.FindKey(This, Root,Key,&Index))
            {
                Status = XdbRootArrayRemoveItemByIndex(This, Root,Index);
            }

            JsonDecreaseReference(Root);
        }

        if(Status != EFI_SUCCESS)
        {
            This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_KEY, Key);
            This->private_.UpdateEnvVariable(This,XDB_ENV_VARIABLE_VALUE, mDefaultNullAsciiString);
        }
    }

    return Status;
}

VOID XdbDumpKeyValue(Xdb *This, CONST CHAR8 *Prefix, CONST CHAR8 *Key, CONST CHAR8 *Value)
{
    if (This->GetVerboseMode(This) && Prefix && Key && Value)
    {
        AsciiPrint("[pass][%a] %a = %a\n", Prefix, Key, Value);
    }
    This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_KEY, Key);
    This->private_.UpdateEnvVariable(This, XDB_ENV_VARIABLE_VALUE, Value);
}

BOOLEAN XdbFindKey(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *Key, UINTN *Index)
{
    ASSERT(This != NULL);
    ASSERT(Root != NULL);
    ASSERT(Key != NULL);
    ASSERT(Index != NULL);

    for (UINTN i = 0; i < JsonArrayCount(Root); i++)
    {
        EDKII_JSON_VALUE obj = JsonArrayGetValue(Root, i);
        CONST CHAR8 *ThisKey = JsonObjectIteratorKey(JsonObjectIterator(obj));
        if (AsciiStrCmp(ThisKey, Key) == 0)
        {
            *Index = i;
            return TRUE;
        }
    }

    return FALSE;
}

EFI_STATUS XdbExtractKeyAndValueFromJsonKeyValue(Xdb *This, EDKII_JSON_VALUE JsonKeyValue,CHAR8 **OutKey, CHAR8 **OutValue)
{
    ASSERT(This != NULL);
    ASSERT(JsonKeyValue != NULL);
    ASSERT(OutKey != NULL);
    ASSERT(OutValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;

    CONST CHAR8 *ThisKey = JsonObjectIteratorKey(JsonObjectIterator(JsonKeyValue));
    CONST CHAR8 *ThisValue = JsonValueGetString(JsonObjectIteratorValue(JsonObjectIterator(JsonKeyValue)));

    if(ThisKey && ThisValue)
    {
        Status = XdbCopyString(This, ThisKey, OutKey);
        if(Status == EFI_SUCCESS)
        {
            Status = XdbCopyString(This, ThisValue, OutValue);
        }
    }
    else
    {
        PrintError(L"Extract key and value failed\n");
    }

    if(Status != EFI_SUCCESS)
    {
        if(*OutKey)
        {
            FreePool(*OutKey);
            *OutKey= NULL;
        }

        if(*OutValue)
        {
            FreePool(*OutValue);
            *OutValue= NULL;
        }
    }

    return Status;
}

EFI_STATUS XdbRootArrayInsert(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue, UINTN Index)
{
    ASSERT(Root != NULL);
    ASSERT(InputKey != NULL);
    ASSERT(InputValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    EDKII_JSON_VALUE NewJsonKeyValue = JsonValueInitObject();
    
    if (NewJsonKeyValue)
    {
        EDKII_JSON_VALUE NewValue = JsonValueInitAsciiString(InputValue);
        if (NewValue)
        {
            if (JsonObjectSetValue(NewJsonKeyValue, InputKey, NewValue) == EFI_SUCCESS)
            {
                if(JsonArrayCount(Root) ==0)
                {
                    Status = JsonArrayAppendValue(Root,NewJsonKeyValue);
                }
                else
                {
                    Status = JsonArrayInsertValue(Root, Index, NewJsonKeyValue);
                }

                if(Status == EFI_SUCCESS)
                {
                    if((Status = This->private_.SaveJson(This, Root)) == EFI_SUCCESS)
                    {
                        This->private_.DumpKeyValue(This, "+", InputKey, InputValue);
                    }
                }
                else
                {
                    PrintError(L"json-array append key-value failed.\n");
                }
            }
            else
            {
                PrintError(L"Update object->kv failed.\n");
            }
            JsonDecreaseReference(NewValue);
        }
        else
        {
            PrintError(L"Create object->kv->value failed.\n");
        }
        JsonDecreaseReference(NewJsonKeyValue);
    }
    else
    {
        PrintError(L"Create object->kv failed.\n");
    }

    return Status;
}

EFI_STATUS XdbRootArrayUpdate(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue)
{
    ASSERT(Root != NULL);
    ASSERT(InputKey != NULL);
    ASSERT(InputValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    UINTN Index = 0;
    if(This->private_.FindKey(This, Root, InputKey, &Index))
    {
        EDKII_JSON_VALUE ThisKeyAndValue = JsonArrayGetValue(Root, Index);
        CONST CHAR8 *ThisKey = JsonObjectIteratorKey(JsonObjectIterator(ThisKeyAndValue));
        EDKII_JSON_VALUE JsonValue = JsonValueInitAsciiString(InputValue);
        if(JsonValue)
        {
            if((Status = JsonObjectSetValue(ThisKeyAndValue, ThisKey, JsonValue)) == EFI_SUCCESS)
            {
               if(( Status = This->private_.SaveJson(This,Root)) == EFI_SUCCESS)
               {
                    This->private_.DumpKeyValue(This, "u", InputKey, InputValue);
               }
            }
            else
            {
                PrintError(L"Update key's value failed, the key is [%a]\n", InputKey);
            }
            JsonDecreaseReference(JsonValue);
        }
        else
        {
            PrintError(L"Create new JsonValue failed.\n");
        }
    }
    else
    {
        PrintError(L"Cannot find the key to update, the key is [%a]\n", InputKey);
    }

    return Status;
}

EFI_STATUS XdbRootArrayGetKeyValueByIndex(Xdb* This, EDKII_JSON_VALUE Root,  UINTN Index, CHAR8 **OutKey, CHAR8 **OutValue)
{
    ASSERT(This != NULL);
    ASSERT(Root != NULL);
    ASSERT(OutKey != NULL);
    ASSERT(OutValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    EDKII_JSON_VALUE JsonKeyAndValue = JsonArrayGetValue(Root, Index);

    if(JsonKeyAndValue)
    {
        Status = XdbExtractKeyAndValueFromJsonKeyValue(This, JsonKeyAndValue,OutKey,OutValue);
    }
    else
    {
        PrintError(L"JsonArrayGetValue failed.\n");
    }

    return Status;
}

EFI_STATUS XdbRootArrayGetKeyValueByKey(Xdb* This, EDKII_JSON_VALUE Root,  CONST CHAR8 *InputKey, CHAR8 **OutKey, CHAR8 **OutValue)
{
    ASSERT(This != NULL);
    ASSERT(Root != NULL);
    ASSERT(InputKey != NULL);
    ASSERT(OutKey != NULL);
    ASSERT(OutValue != NULL);

    EFI_STATUS Status = EFI_ABORTED;
    UINTN Index = 0;

    if(This->private_.FindKey(This, Root,InputKey,&Index))
    {
        Status = XdbRootArrayGetKeyValueByIndex(This,Root, Index, OutKey,OutValue);
    }
    else
    {
        PrintError(L"Cannot find the key-value, input key is [%a]\n", InputKey);
    }

    return Status;
}

EFI_STATUS XdbRootArrayRemoveItemByIndex(Xdb *This, EDKII_JSON_VALUE Root, UINTN Index)
{
    ASSERT(Root != NULL);
    EFI_STATUS Status = EFI_ABORTED;
    CHAR8* TempKey = NULL;
    CHAR8* TempValue = NULL;

    if((Status = XdbRootArrayGetKeyValueByIndex(This, Root, Index, &TempKey, &TempValue)) == EFI_SUCCESS)
    {
        if((Status=JsonArrayRemoveValue(Root, Index)) == EFI_SUCCESS)
        {
            if((Status = This->private_.SaveJson(This, Root)) == EFI_SUCCESS)
            {
                This->private_.DumpKeyValue(This, "-", TempKey, TempValue);
            }
            else
            {
                PrintError(L"SaveJson failed.\n");
            }
        }
        else
        {
            PrintError(L"XdbRootArrayRemoveItemByIndex failed.\n");
        }
    }
    else
    {
        PrintError(L"XdbRootArrayGetKeyValueByIndex failed.\n");
    }

    if(TempKey) FreePool(TempKey);
    if(TempValue) FreePool(TempValue);

    return Status;
}

EFI_STATUS XdbRootArrayRemoveByKey(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey)
{
    ASSERT(Root != NULL);
    ASSERT(InputKey != NULL);

    UINTN Index = 0;

    if(This->private_.FindKey(This, Root,InputKey,&Index))
    {
        return XdbRootArrayRemoveItemByIndex(This, Root,Index);
    }

    return EFI_ABORTED;
}
