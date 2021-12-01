#ifndef __ARGUMENT_PARSER_H__
#define __ARGUMENT_PARSER_H__

#define COLOR_ATTR_RED      0x0C
#define COLOR_ATTR_GREEN    0x0A
#define COLOR_ATTR_PURPLE   0x0D

#define MAX_STEP_SIZE       5

typedef struct Parser_ Parser;

typedef UINTN (*PARSER_CONSTRUCTOR)(Parser *This);
typedef UINTN (*PARSER_DESTRUCTOR)(Parser *This);
typedef UINTN (*WORK)(Parser *This);
typedef UINTN (*STEP)(Parser *This);

typedef struct Mapping_ Mapping;

struct Parser_{
    ArgumentManager         *argumentManager;           ///< Store a ArgumentManager pointer to this.
    BufferManager           *bufferManager;             ///< Store a BufferManager pointer to this.

    Mapping                 *mappingTable;              ///< Store a mappingTable pointer to this.
    UINTN                   mappingTableSize;           ///< Store mappingTableSize.

    UINTN                   status;                     ///< Used to store temp return-value when do work->steps.
    UINTN                   mode;                       ///< Used to store temp mode when do work->steps.
    UINTN                   data;                       ///< Used to store temp data when do work->steps.

    /// Public functions
    PARSER_CONSTRUCTOR      constructor;                ///< This object's constructor.
    PARSER_DESTRUCTOR       destructor;                 ///< This object's destructor.
    WORK                    work;                       ///< The main work function.

    ///
    /// Privte functions.
    /// Step 1
    ///
    STEP                    stepFirstHelpMessage;
    STEP                    stepFirstNeedOneNumber;
    STEP                    stepFirstNeedOneOrMoreStrings;
    STEP                    stepFirstNeedNoneArg;
    STEP                    stepFirstNeedOneString;

    /// Step 2
    STEP                    step2InputBuffer;

    /// Step 3
    STEP                    step3FrontMoveN;
    STEP                    step3ReverseMoveN;
    STEP                    step3FrontNonSkipForce;
    STEP                    step3FrontNonSkipNonForce;
    STEP                    step3FrontSkipForce;
    STEP                    step3FrontSkipNonForce;
    STEP                    step3ReverseNonSkipForce;
    STEP                    step3ReverseNonSkipNonForce;
    STEP                    step3ReverseSkipForce;
    STEP                    step3ReverseSkipNonForce;
    STEP                    step3TrimSpace;

    STEP                    step3PushFrontString;
    STEP                    step3PushBackString;
    STEP                    step3ScopeOneChar;
    STEP                    step3ScopeOneWord;
    STEP                    step3ScopeOneLine;
    STEP                    step3TestEmpty;
    STEP                    step3TestMatch;
    STEP                    Step3TestFind;

    /// Step last
    STEP                    stepLastOutputBuffer;   
    STEP                    stepLastOutputStatus;
    STEP                    stepLastOutputData;
};

/**
 * Every Option's work flow.
 */
struct Mapping_{
    CHAR16                      arg[MAX_STRING_SZ+1];       ///< Option-string at command line.
    STEP                        stepArray[MAX_STEP_SIZE];   ///< All steps,it should end with a NULL-step.
};

extern Parser gParser;

extern Mapping gMappingTable[];

/**
 * Construct parser.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success;
 * @retval 0                                Operation failed;
 */
UINTN ParserConstructor(Parser *This);

/**
 * Destruct parser.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success;
 * @retval 0                                Operation failed;
 */
UINTN ParserDestructor(Parser *This);

/**
 * Parsing args and do buffer operation steps.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success;
 * @retval 0                                Operation failed;
 */
UINTN Work(Parser *This);

/**
 * Show help message.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 0                                default value;
 */
UINTN StepFirstHelpMessage(Parser *This);

/**
 * Show version message.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 0                                default value;
 */
UINTN StepFirstHelpVersion(Parser *This);

/**
 * Test and make sure the reset args is empty.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                The reset args is empty.
 * @retval 0                                invalid parameter,The reset args is not empty.
 */
UINTN StepFirstNeedNoneArg(Parser *This);

/**
 * Test and make sure there is only one vaild string.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                test one string ok;
 * @retval 0                                invalid parameter.
 */
UINTN StepFirstNeedOneString(Parser *This);

/**
 * Test and make sure there is only one vaild number.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                test one number ok;
 * @retval 0                                invalid parameter.
 */
UINTN StepFirstNeedOneNumber(Parser *This);

/**
 * Test and make sure all string are valid;
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                test strings ok;
 * @retval 0                                invalid parameter.
 */
UINTN StepFirstNeedOneOrMoreStrings(Parser *This);

/**
 * Input buffer from stdin pipe.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step2InputBuffer(Parser *This);

/**
 * Front iterator move to a number.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3FrontMoveN(Parser *This);

/**
 * Reverse iterator move to a number.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ReverseMoveN(Parser *This);

/**
 * front-iterator && non-skip && force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3FrontNonSkipForce(Parser *This);

/**
 * front-iterator && non-skip && non-force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3FrontNonSkipNonForce(Parser *This);

/**
 * front-iterator && skip && force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3FrontSkipForce(Parser *This);

/**
 * front-iterator && skip && non-force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3FrontSkipNonForce(Parser *This);

/**
 * reverse-iterator && non-skip && force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ReverseNonSkipForce(Parser *This);

/**
 * reverse-iterator && non-skip && non-force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ReverseNonSkipNonForce(Parser *This);

/**
 * reverse-iterator && skip && force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ReverseSkipForce(Parser *This);

/**
 * reverse-iterator && skip && non-force move to string(s).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ReverseSkipNonForce(Parser *This);

/**
 * Trim off scope's front invisible chars.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TrimFrontSpace(Parser *This);

/**
 * Trim off scope's back invisible chars.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TrimBackSpace(Parser *This);

/**
 * Trim off scope's front and back invisible chars.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TrimSpace(Parser *This);

/**
 * Get a char scope at front iterator.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ScopeOneChar(Parser *This);

/**
 * Get a word scope at front iterator.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ScopeOneWord(Parser *This);

/**
 * Get a line scope at front iterator.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3ScopeOneLine(Parser *This);

/**
 * Push a string at the front of the scope.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3PushFrontString(Parser *This);

/**
 * Push a string at the back of the scope.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3PushBackString(Parser *This);

/**
 * Test scope empty or not.(if the scope only has invisible chars,it is empty too).
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TestEmpty(Parser *This);

/**
 * Test scope match(equal) a string or not.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TestMatch(Parser *This);

/**
 * Test scope contain a string or not.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TestFind(Parser *This);

/**
 * Output scope's size.
 * The step should only be used in parser->work() function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3TestSize(Parser *This);

/**
 * Internal move to a number.
 * The step should only be used in a parser->work->step3 function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3InternalMoveNumber(Parser *This);

/**
 * Internal move to string(s).
 * The step should only be used in a parser->work->step3 function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN Step3InternalMoveStrings(Parser *This);

/**
 * final output buffer.
 * The step should only be used in parser->work function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN StepLastOutputBuffer(Parser *This);

/**
 * final output any parser->work->step's status.
 * The step should only be used in parser->work function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN StepLastOutputStatus(Parser *This);

/**
 * final output any parser->work->step's data.
 * The step should only be used in parser->work function.
 * 
 * @param[IN]   This                      The instance of Parser.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN StepLastOutputData(Parser *This);

/**
 * Usually when do iterator-moving,we should use "grep.efi + number" or "grep - number",
 * It is a patch for "grep +number" and "grep.efi -number" can get work,
 * 
 * The internal work is:
 * 1,check this option format is "+number" or "-number";
 * 2,if passed, save +|- to Parser->mode,(the mode will changed from invalid-mode to valid mode).
 *              save number to Parser->data.
 * 3,when moving any iterator,the saved datas will be used in Step3InternalMoveNumber.
 * 
 * @param[IN]   This                      The instance of Parser.
 * @param[IN]   option                    The first option in command line,the first wchar in the option should be L'+' or L'-'.
 * 
 * @retval 1                                Operation success.
 * @retval 0                                Operation failed.
 */
UINTN OptionBeginWithCharNumber(Parser *This, CHAR16 *option);

/**
 * Print colored string.
 * 
 * @param[IN] String                The string to be print.
 * @param[IN] Color                 The color will be used.
 */
VOID ColorPrint(CHAR16* String,UINTN Color);

#endif //__ARGUMENT_PARSER_H__
