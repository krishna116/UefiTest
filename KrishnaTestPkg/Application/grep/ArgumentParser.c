#include "grep.h"
#include "BufferManager.h"
#include "ArgumentParser.h"

Mapping gMappingTable[] = {
    /// all help messages.
    {L"?",                          StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},
    /// some option has alias like this.
    {L"-?",                         StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},
    {L"-h",                         StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},
    {L"help",                       StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},
    {L"-help",                      StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},
    {L"--help",                     StepFirstHelpMessage,               NULL,                   NULL,                           NULL,                  NULL},

    /// front iterator move to a number.
    {L"+",                          StepFirstNeedOneNumber,             Step2InputBuffer,       Step3FrontMoveN,                StepLastOutputBuffer,  NULL},
    {L"-move-front",                StepFirstNeedOneNumber,             Step2InputBuffer,       Step3FrontMoveN,                StepLastOutputBuffer,  NULL},

    /// reverse iterator move to a number.
    {L"-",                          StepFirstNeedOneNumber,             Step2InputBuffer,       Step3ReverseMoveN,              StepLastOutputBuffer,  NULL},
    {L"-move-reverse",              StepFirstNeedOneNumber,             Step2InputBuffer,       Step3ReverseMoveN,              StepLastOutputBuffer,  NULL},

    /// front iterator move to string(s).
    {L"+-",                         StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontNonSkipForce,         StepLastOutputBuffer,  NULL},
    {L"+-+",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontNonSkipForce,         StepLastOutputBuffer,  NULL},
    {L"-front-non-skip-force",      StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontNonSkipForce,         StepLastOutputBuffer,  NULL},

    {L"+--",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontNonSkipNonForce,      StepLastOutputBuffer,  NULL},
    {L"-front-non-skip-non-force",  StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontNonSkipNonForce,      StepLastOutputBuffer,  NULL},

    {L"++",                         StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontSkipForce,            StepLastOutputBuffer,  NULL},
    {L"+++",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontSkipForce,            StepLastOutputBuffer,  NULL},
    {L"-front-skip-force",          StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontSkipForce,            StepLastOutputBuffer,  NULL},

    {L"++-",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontSkipNonForce,         StepLastOutputBuffer,  NULL},
    {L"-front-skip-non-force",      StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3FrontSkipNonForce,         StepLastOutputBuffer,  NULL},

    /// reverse iterator move to string(s).
    {L"--",                         StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseNonSkipForce,       StepLastOutputBuffer,  NULL},
    {L"--+",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseNonSkipForce,       StepLastOutputBuffer,  NULL},
    {L"-reverse-non-skip-force",    StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseNonSkipForce,       StepLastOutputBuffer,  NULL},

    {L"---",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseNonSkipNonForce,    StepLastOutputBuffer,  NULL},
    {L"-reverse-non-skip-non-force",StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseNonSkipNonForce,    StepLastOutputBuffer,  NULL},

    {L"-+",                         StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseSkipForce,          StepLastOutputBuffer,  NULL},
    {L"-++",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseSkipForce,          StepLastOutputBuffer,  NULL},
    {L"-reverse-skip-force",        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseSkipForce,          StepLastOutputBuffer,  NULL},

    {L"-+-",                        StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseSkipNonForce,       StepLastOutputBuffer,  NULL},
    {L"-reverse-skip-non-force",    StepFirstNeedOneOrMoreStrings,      Step2InputBuffer,       Step3ReverseSkipNonForce,       StepLastOutputBuffer,  NULL},

    /// trim off invisible chars.
    {L"-trim-front-space",          StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimFrontSpace,            StepLastOutputBuffer,  NULL},
    {L"-tfs",                       StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimFrontSpace,            StepLastOutputBuffer,  NULL},

    {L"-trim-back-space",           StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimBackSpace,             StepLastOutputBuffer,  NULL},
    {L"-tbs",                       StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimBackSpace,             StepLastOutputBuffer,  NULL},

    {L"-trim-space",                StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimSpace,                 StepLastOutputBuffer,  NULL},
    {L"-ts",                        StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TrimSpace,                 StepLastOutputBuffer,  NULL},

    /// append operations.
    {L"-push-front",                StepFirstNeedOneString,             Step2InputBuffer,       Step3PushFrontString,           StepLastOutputBuffer,  NULL},
    {L"-pf",                        StepFirstNeedOneString,             Step2InputBuffer,       Step3PushFrontString,           StepLastOutputBuffer,  NULL},

    {L"-push-back",                 StepFirstNeedOneString,             Step2InputBuffer,       Step3PushBackString,            StepLastOutputBuffer,  NULL},
    {L"-pb",                        StepFirstNeedOneString,             Step2InputBuffer,       Step3PushBackString,            StepLastOutputBuffer,  NULL},

    /// scope operations.
    {L"-char",                      StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneChar,              StepLastOutputBuffer,  NULL},
    {L"-c",                         StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneChar,              StepLastOutputBuffer,  NULL},

    {L"-word",                      StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneWord,              StepLastOutputBuffer,  NULL},
    {L"-w",                         StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneWord,              StepLastOutputBuffer,  NULL},

    {L"-line",                      StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneLine,              StepLastOutputBuffer,  NULL},
    {L"-l",                         StepFirstNeedNoneArg,               Step2InputBuffer,       Step3ScopeOneLine,              StepLastOutputBuffer,  NULL},

    /// test operations.
    {L"-empty",                     StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TestEmpty,                 StepLastOutputStatus,  NULL},
    {L"-em",                        StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TestEmpty,                 StepLastOutputStatus,  NULL},

    {L"-match",                     StepFirstNeedOneString,             Step2InputBuffer,       Step3TestMatch,                 StepLastOutputStatus,  NULL},
    {L"-equal",                     StepFirstNeedOneString,             Step2InputBuffer,       Step3TestMatch,                 StepLastOutputStatus,  NULL},
    {L"-eq",                        StepFirstNeedOneString,             Step2InputBuffer,       Step3TestMatch,                 StepLastOutputStatus,  NULL},

    {L"-find",                      StepFirstNeedOneString,             Step2InputBuffer,       Step3TestFind,                  StepLastOutputStatus,  NULL},
    {L"-fi",                        StepFirstNeedOneString,             Step2InputBuffer,       Step3TestFind,                  StepLastOutputStatus,  NULL},

    {L"-size",                      StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TestSize,                  StepLastOutputData,    NULL},
    {L"-si",                        StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TestSize,                  StepLastOutputData,    NULL},
    {L"-sz",                        StepFirstNeedNoneArg,               Step2InputBuffer,       Step3TestSize,                  StepLastOutputData,    NULL},
};

Parser gParser = {
    &gArgumentManager,
    &gBufferManager,

    gMappingTable,
    sizeof(gMappingTable) / sizeof(Mapping),

    0,                              /// temp return-code.
    INVALID_MODE,                   /// temp mode.
    0,                              /// temp data.

    /// public functions
    ParserConstructor,
    ParserDestructor,
    Work,
    
    /// protected functions
    /// Step first
    StepFirstHelpMessage,
    StepFirstNeedOneNumber,
    StepFirstNeedOneOrMoreStrings,
    StepFirstNeedNoneArg,
    StepFirstNeedOneString,

    /// Step 2
    Step2InputBuffer,

    /// Step 3
    Step3FrontMoveN,
    Step3ReverseMoveN,
    Step3FrontNonSkipForce,
    Step3FrontNonSkipNonForce,
    Step3FrontSkipForce,
    Step3FrontSkipNonForce,
    Step3ReverseNonSkipForce,
    Step3ReverseNonSkipNonForce,
    Step3ReverseSkipForce,
    Step3ReverseSkipNonForce,
    Step3TrimSpace,

    Step3PushFrontString,
    Step3PushBackString,
    Step3ScopeOneChar,
    Step3ScopeOneWord,
    Step3ScopeOneLine,
    Step3TestEmpty,
    Step3TestMatch,
    Step3TestFind,

    /// Step last
    StepLastOutputBuffer,
    StepLastOutputStatus,
    StepLastOutputData,
};

UINTN StepFirstHelpMessage(Parser *This)
{
    ///_____--------------------80 char width-----------------------------------------------
    Print(L"+-----------------------------------------------------------------------------+\n");
    Print(L"|  grep.efi v2.1 for stream feature extraction.          --Krishna,2019-04-17 |\n");
    Print(L"+-----------------------------------------------------------------------------+\n");
    Print(L"Usage:\n");
    Print(L"  grep.efi [option]\n");
    Print(L"Options:\n");
    Print(L"  [+|-] [n]              //move a iterator by a number.\n");
    Print(L"                           1,[+|-]:front or reverse (iterator).\n");
    Print(L"                           2,[n]:a number to move.\n");

    Print(L"  ");ColorPrint(L"[+|-]",COLOR_ATTR_RED);ColorPrint(L"[+|-]",COLOR_ATTR_GREEN);ColorPrint(L"{+|-}",COLOR_ATTR_PURPLE);Print(L" [s]+   //move a iterator by string(s).\n");
    Print(L"                           1,");ColorPrint(L"[+|-]",COLOR_ATTR_RED);Print(L":front or reverse;  2,");ColorPrint(L"[+|-]",COLOR_ATTR_GREEN);Print(L":skip or non-skip.\n");
    Print(L"                           3,");ColorPrint(L"{+|-}",COLOR_ATTR_PURPLE);Print(L":force or non-force;4,");Print(L"[s]+:string(s) input.\n\n");

    Print(L"  -char                  //get first visible char at front-iterator.\n");
    Print(L"  -word                  //get first visible word at front-iterator.\n");
    Print(L"  -line                  //get one line at front-iterator.\n");
    Print(L"  -trim-space            //trim off scope's front and back invisible chars.\n\n");

    Print(L"  -push-front [s]        //add a string at front of scope.\n");
    Print(L"  -push-back  [s]        //add a string at back of scope.\n\n");

    Print(L"  -empty                 //test scope is empty or not.\n");
    Print(L"  -equal [s]             //test scope if it equals a string or not.\n");
    Print(L"  -find  [s]             //test scope if it contains a string or not.\n");

    return 0;
};

UINTN ParserConstructor(Parser *This)
{
    ASSERT(This != NULL);

    /* 
        Force to using global protocols,as there has strange result if using ArgumentManager->param and BufferManager->shell:
        if the command used in script file(xxx.nsh),and script with "echo off", the pipe message will also be "turn off".(it may be a shell's bug).
     */
    if(gEfiShellProtocol==NULL || gEfiShellParametersProtocol==NULL)
    {
        PrintError(L"Pipe function dependencies are not satisfied.\n");
        return 0;
    }

    return This->argumentManager->constructor(This->argumentManager);
};

UINTN ParserDestructor(Parser *This)
{
    ASSERT(This != NULL);

    This->bufferManager->destructor(This->bufferManager);

    return 1;
};

UINTN Work(Parser *This)
{
    UINTN returnCode = 0;
    UINTN find = 0;
    CHAR16 *arg = NULL;
    UINTN index;

    ASSERT(This != NULL);

    if(This->argumentManager->size(This->argumentManager)==1)
    {
        return This->stepFirstHelpMessage(This);
    }
    else if (This->argumentManager->peekArg(This->argumentManager) == 1)
    {
        /// It is the option-string.
        arg = This->argumentManager->getNextArg(This->argumentManager);

        for (UINTN i = 0; i < This->mappingTableSize; i++)
        {
            /// Try to find a option-mapping to process this option.
            if (StrCmp(arg, This->mappingTable[i].arg) == 0)
            {
                find = 1;

                index=0;
                /// Process the option by a serial of steps.
                while(This->mappingTable[i].stepArray[index]!=NULL)
                {
                    returnCode=This->mappingTable[i].stepArray[index](This);
                    if(returnCode!=1)
                    {
                        break;
                    }

                    index++;
                }

                break;
            }
        } /// for loop end;

        /// The above for loop can process "grep + number" and "grep - number",
        /// but cannot process "grep +number" and "grep -number".
        /// this patch can make the later work.
        if (find == 0 && This->argumentManager->size(This->argumentManager) == 2)
        {
            if (arg[0]==L'+' && OptionBeginWithCharNumber(This, arg))
            {
                find = 1;

                /// process "+number"
                if ((returnCode = This->step2InputBuffer(This)) == 1)
                {
                    if ((returnCode = This->step3FrontMoveN(This)) == 1)
                    {
                        returnCode = This->stepLastOutputBuffer(This);
                    }
                }
            }
            else if (arg[0]==L'-' && OptionBeginWithCharNumber(This, arg))
            {
                find = 1;

                /// process "-number"
                if ((returnCode = This->step2InputBuffer(This)) == 1)
                {
                    if ((returnCode = This->step3ReverseMoveN(This)) == 1)
                    {
                        returnCode = This->stepLastOutputBuffer(This);
                    }
                }
            }
        } /// Patch end
    }

    if (!find)
    {
        PrintError(L"Invalid-parameter,please type [ grep.efi -h ] to get help.\n");
    }

    return returnCode;
};

UINTN StepFirstNeedNoneArg(Parser *This)
{
    ASSERT(This != NULL);

    if (This->argumentManager->peekArg(This->argumentManager) == 1)
    {
        PrintError(L"Invalid-parameter.\n");
        return 0;
    }

    return 1;
};

UINTN StepFirstNeedOneString(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,backup index,we will use it again in later steps.
    This->argumentManager->backupIndex(This->argumentManager);

    /// 002,check args.
    {
        if (This->argumentManager->peekArg(This->argumentManager) != 1)
        {
            /// Next arg not exist;
            PrintError(L"Invalid-parameter.\n");
            return 0;
        }

        if (StrLen(This->argumentManager->getNextArg(This->argumentManager)) > MAX_STRING_SZ)
        {
            PrintError(L"Invalid-parameter->input string size overflow.\n");
            return 0;
        }

        if (This->argumentManager->peekArg(This->argumentManager) == 1)
        {
            /// Next next arg should not exist,as we need only one arg.
            PrintError(L"Invalid-parameter.\n");
            return 0;
        }
    }

    /// 003,restore arg index.
    This->argumentManager->restoreIndex(This->argumentManager);

    return 1;
};

UINTN StepFirstNeedOneNumber(Parser *This)
{
    CHAR16 *begin = NULL;
    UINTN size = 0;
    UINTN number = 0;

    ASSERT(This != NULL);

    /// 001,backup index,we will use it again in later steps.
    This->argumentManager->backupIndex(This->argumentManager);

    /// 002,check args.
    {
        if (This->argumentManager->peekArg(This->argumentManager) != 1)
        {
            PrintError(L"Invalid-parameter.\n");
            return 0;
        };

        begin = This->argumentManager->getNextArg(This->argumentManager);
        size = StrLen(begin);

        /// It should be a number;
        if (StrDecimalToUintnS(begin, NULL, &number) != RETURN_SUCCESS)
        {
            PrintError(L"Invalid-parameter->input number is not valid.\n");
            return 0;
        }

        /// Strong check,make sure the string do not contain invalid digits;
        for (UINTN i = 0; i < size; i++)
        {
            if (*begin < WCHAR_DIGIT_BEGIN || *begin > WCHAR_DIGIT_END)
            {
                PrintError(L"Number is not a vaild decimal digit-number.\n");
                return 0;
            }

            begin++;
        }
    }

    /// 003,restore arg index.
    This->argumentManager->restoreIndex(This->argumentManager);

    return 1;
};

UINTN StepFirstNeedOneOrMoreStrings(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,backup index,we will use it again.
    This->argumentManager->backupIndex(This->argumentManager);

    /// 002,check args.
    {
        if (This->argumentManager->peekArg(This->argumentManager) != 1)
        {
            PrintError(L"Invalid-parameter.\n");
            return 0;
        }

        while (This->argumentManager->peekArg(This->argumentManager) == 1)
        {
            if (StrLen(This->argumentManager->getNextArg(This->argumentManager)) > MAX_STRING_SZ)
            {
                PrintError(L"Input string size overflow.\n");
                return 0;
            }
        }
    }

    /// 003,restore arg index.
    This->argumentManager->restoreIndex(This->argumentManager);

    return 1;
};

UINTN Step2InputBuffer(Parser *This)
{
    ASSERT(This != NULL);
    ASSERT(This->bufferManager != NULL);

    return This->bufferManager->constructor(This->bufferManager, This->argumentManager->param);
};

UINTN Step3FrontMoveN(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);

    /// 002,get a number and moving.
    return Step3InternalMoveNumber(This);
};

UINTN Step3ReverseMoveN(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);

    /// 002,get a number and moving.
    return Step3InternalMoveNumber(This);
};

UINTN Step3FrontNonSkipForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, NON_SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3FrontNonSkipNonForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, NON_SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, NON_FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3FrontSkipForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3FrontSkipNonForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, NON_FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3ReverseNonSkipForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, NON_SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3ReverseNonSkipNonForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, NON_SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, NON_FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3ReverseSkipForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode;
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3ReverseSkipNonForce(Parser *This)
{
    ASSERT(This != NULL);

    /// 001,set correct mode.
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);
    This->bufferManager->setSkipMode(This->bufferManager, SKIP_MODE);
    This->bufferManager->setForceMode(This->bufferManager, NON_FORCE_MODE);

    /// 002,working.
    return Step3InternalMoveStrings(This);
};

UINTN Step3TrimFrontSpace(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->trimFrontSpace(This->bufferManager);
};

UINTN Step3TrimBackSpace(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->trimBackSpace(This->bufferManager);
};

UINTN Step3TrimSpace(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->trimSpace(This->bufferManager);
};

UINTN Step3ScopeOneChar(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->scopeToChar(This->bufferManager);
};

UINTN Step3ScopeOneWord(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->scopeToWord(This->bufferManager);
};

UINTN Step3ScopeOneLine(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->scopeToLine(This->bufferManager);
};

UINTN Step3PushFrontString(Parser *This)
{
    CHAR16 *string = NULL;
    UINTN stringLen = 0;

    ASSERT(This != NULL);
    ASSERT(This->argumentManager->peekArg(This->argumentManager) == 1);

    string = This->argumentManager->getNextArg(This->argumentManager);
    stringLen = StrLen(string);
    This->bufferManager->setIteratorMode(This->bufferManager, FRONT_ITERATOR);

    return This->bufferManager->pushString(This->bufferManager, string, stringLen);
};

UINTN Step3PushBackString(Parser *This)
{
    CHAR16 *string = NULL;
    UINTN stringLen = 0;

    ASSERT(This != NULL);
    ASSERT(This->argumentManager->peekArg(This->argumentManager) == 1);

    string = This->argumentManager->getNextArg(This->argumentManager);
    stringLen = StrLen(string);
    This->bufferManager->setIteratorMode(This->bufferManager, REVERSE_ITERATOR);

    return This->bufferManager->pushString(This->bufferManager, string, stringLen);
};

UINTN Step3TestEmpty(Parser *This)
{
    ASSERT(This != NULL);

    /// The result is stored to This->status,we will output it in later step.
    This->status = This->bufferManager->isBufferEmpty(This->bufferManager);

    return 1;
};

UINTN Step3TestMatch(Parser *This)
{
    CHAR16 *string = NULL;
    UINTN stringLen = 0;

    ASSERT(This != NULL);

    /// An argument must be here.
    ASSERT(This->argumentManager->peekArg(This->argumentManager) == 1);

    string = This->argumentManager->getNextArg(This->argumentManager);
    stringLen = StrLen(string);

    This->status = This->bufferManager->matchString(This->bufferManager, string, stringLen);

    return 1;
};

UINTN Step3TestFind(Parser *This)
{
    CHAR16 *string = NULL;
    UINTN stringLen = 0;

    ASSERT(This != NULL);

    /// An argument must be here.
    ASSERT(This->argumentManager->peekArg(This->argumentManager) == 1);

    string = This->argumentManager->getNextArg(This->argumentManager);
    stringLen = StrLen(string);

    This->status = This->bufferManager->findString(This->bufferManager, string, stringLen);

    return 1;
};

UINTN Step3TestSize(Parser *This)
{
    ASSERT(This != NULL);

    This->data = This->bufferManager->getScopeSize(This->bufferManager);

    return 1;
};

UINTN Step3InternalMoveNumber(Parser *This)
{
    CHAR16 *string;
    UINTN number = 0;

    ASSERT(This != NULL);

    /// This->mode's default value is INVALID_MODE,
    /// It is changed in OptionBeginWithCh.
    if (This->data != 0 && This->mode != INVALID_MODE)
    {
        /// Option is "+number | -number"
        return This->bufferManager->move(This->bufferManager, This->data);
    }
    else
    {
        /// Option is "+ number | - number"
        ASSERT(This->argumentManager->peekArg(This->argumentManager) == 1);

        string = This->argumentManager->getNextArg(This->argumentManager);
        StrDecimalToUintnS(string, NULL, &number);

        return This->bufferManager->move(This->bufferManager, number);
    }
};

UINTN Step3InternalMoveStrings(Parser *This)
{
    UINTN status = 0;
    CHAR16 *string = NULL;
    UINTN stringLen = 0;

    ASSERT(This != NULL);

    while (This->argumentManager->peekArg(This->argumentManager) == 1)
    {
        string = This->argumentManager->getNextArg(This->argumentManager);
        stringLen = StrLen(string);

        status = This->bufferManager->moveToString(This->bufferManager, string, stringLen);
        if (status == 0)
        {
            break;
        }
    }

    /// Return 1 to make any change can be saved.
    return 1;
};

UINTN StepLastOutputBuffer(Parser *This)
{
    ASSERT(This != NULL);

    return This->bufferManager->outputBuffer(This->bufferManager);
};

UINTN StepLastOutputStatus(Parser *This)
{
    /// Make sure string is UCS2 string,and do not output the string terminator(WCHAR_END).
    CHAR16 stringYes[] = {L'1', WCHAR_CR, WCHAR_LF, WCHAR_END};
    UINTN stringYesByteSize = StrLen(stringYes) * sizeof(CHAR16);

    CHAR16 stringNo[] = {L'0', WCHAR_CR, WCHAR_LF, WCHAR_END};
    UINTN stringNoByteSize = StrLen(stringNo) * sizeof(CHAR16);

    ASSERT(This != NULL);

    if (This->status == 1)
    {
        if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
        {
            gEfiShellProtocol->WriteFile(gEfiShellParametersProtocol->StdOut, &stringYesByteSize, stringYes);
            gEfiShellProtocol->FlushFile(gEfiShellParametersProtocol->StdOut);
        }
        else
        {
            Print(L"%s", stringYes);
        }
    }
    else
    {
        if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
        {
            gEfiShellProtocol->WriteFile(gEfiShellParametersProtocol->StdOut, &stringNoByteSize, stringNo);
            gEfiShellProtocol->FlushFile(gEfiShellParametersProtocol->StdOut);
        }
        else
        {
            Print(L"%s", stringNo);
        }
    }

    return This->status;
};

UINTN StepLastOutputData(Parser *This)
{
    /* NumberLength + CR_LF + StringTerminator. */
    CHAR16 buffer[32 + 2 + 1];
    UINTN bufferSize = sizeof(buffer);
    UINTN strLen = 0;
    UINTN bufferValidSize = 0;

    ASSERT(This != NULL);

    ZeroMem(buffer, bufferSize);
    if (UnicodeValueToStringS(buffer, bufferSize, 0, This->data, 32) == RETURN_SUCCESS)
    {
        strLen = StrLen(buffer);

        /// Make sure string is UCS2 string with CR+LF.
        buffer[strLen] = WCHAR_CR;
        buffer[strLen + 1] = WCHAR_LF;
        buffer[strLen + 2] = WCHAR_END;

        /// Do not output the string terminator(WCHAR_END).
        bufferValidSize = (strLen + 2) * sizeof(CHAR16);

        if (gEfiShellProtocol != NULL && gEfiShellParametersProtocol != NULL)
        {
            gEfiShellProtocol->WriteFile(gEfiShellParametersProtocol->StdOut, &bufferValidSize, (void *)buffer);
            gEfiShellProtocol->FlushFile(gEfiShellParametersProtocol->StdOut);
        }
        else
        {
            Print(L"%s", buffer);
        }
    }

    return 0;
};

UINTN OptionBeginWithCharNumber(Parser *This, CHAR16 *option)
{
    CHAR16 *begin = NULL;
    CHAR16 *temp = NULL;
    UINTN optionStrLen = 0;
    UINTN number = 0;

    ASSERT(This != NULL);
    ASSERT(option != NULL);

    optionStrLen = StrLen(option);

    if (StrLen(option) > 1)
    {
        /// Make sure the rest part is a decimal-string.
        temp = option + 1;
        for (UINTN i = 0; i < (optionStrLen - 1); i++)
        {
            if (*temp < WCHAR_DIGIT_BEGIN || *temp > WCHAR_DIGIT_END)
            {
                return 0;
            }

            temp++;
        }
    }

    if (*option== L'+')
    {
        /// option is "+number"
        if (StrLen(option) == 1)
        {
            /// number is 0;
        }
        else
        {
            begin = option + 1;
            if (StrDecimalToUintnS(begin, NULL, &number) != RETURN_SUCCESS)
            {
                PrintError(L"Intput is not a vaild digit-number.\n");
                return 0;
            }
        }

        This->mode = FRONT_ITERATOR;
        This->data = number;
        return 1;
    }
    else if (*option == L'-')
    {
        /// Option is "-number"
        if (StrLen(option) == 1)
        {
            /// number is 0;
        }
        else
        {
            begin = option + 1;
            if (StrDecimalToUintnS(begin, NULL, &number) != RETURN_SUCCESS)
            {
                PrintError(L"Intput is not a vaild digit-number.\n");
                return 0;
            }
        }

        This->mode = REVERSE_ITERATOR;
        This->data = number;
        return 1;
    }

    return 0;
};

VOID ColorPrint(CHAR16* String,UINTN Color)
{
    UINTN backup;
    ASSERT(String!=NULL);

    backup=gST->ConOut->Mode->Attribute;
    gST->ConOut->SetAttribute(gST->ConOut,Color);
    Print(L"%s",String);
    gST->ConOut->SetAttribute(gST->ConOut,backup);
}