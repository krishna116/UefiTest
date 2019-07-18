#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

///
/// Front iterator point to the buffer's first char default.
///
#define FRONT_ITERATOR                          0

///
/// Reverse Iterator Point to the buffer's last char default.
///
#define REVERSE_ITERATOR                        1

///
/// Force & non-force
///
/// Given this [string] for example:"name = Li Lei //description;" 
/// Run [string] | grep -line | grep --reverse-move-skip-non-force "//"
/// The result is "name = Li Lei "
///
/// Run [string] | grep -line | grep --reverse-move-skip-force "//"
/// The result is "name = Li Lei "
/// 
/// But given this [string] for example:"name = Li Lei" 
/// Run [string] | grep -line | grep --reverse-move-skip-non-force "//"
/// The result is "name = Li Lei", it will trim off "//" as far as possible,but not necessarily.
///
/// Run [string] | grep -line | grep --reverse-move-skip-force "//"
/// The result is empty, if cannot find the string in force mode, the result is always empty.
/// 
#define NON_FORCE_MODE                          2
#define FORCE_MODE                              3

///
/// The iterator point to the string or skip the string.
///
#define NON_SKIP_MODE                           4
#define SKIP_MODE                               5

#define INVALID_MODE                            0xFFFF

/// Whitespace char
#define WCHAR_SPACE                             0x0020

#define WCHAR_LF                                0x000A
#define WCHAR_CR                                0x000D
#define WCHAR_END                               0x0000

#define WCHAR_VISIBLE_BEGIN                     (WCHAR_SPACE+1)

/// May be we could ignore this to get chinese chars work.
///#define WCHAR_VISIBLE_END                       0x007E

#define WCHAR_DIGIT_BEGIN                       0x0030
#define WCHAR_DIGIT_END                         0x0039

#define WCHAR_UCS_BOM                           0xFEFF

#define MAX_STRING_SZ                           128
#define MAX_BUFFER_SZ                           (1024*1024*4)

typedef struct BufferManager_ BufferManager;

typedef UINTN (*BUFFER_MANAGER_CONSTRUCTOR)(BufferManager *This, EFI_SHELL_PARAMETERS_PROTOCOL *param);
typedef UINTN (*BUFFER_MANAGER_DESTRUCTOR)(BufferManager *This);

typedef UINTN (*SET_ITERATOR_MODE)(BufferManager *This, UINTN WhichIterator);
typedef UINTN (*IS_FRONT_ITERATOR_MODE)(BufferManager *This);
typedef UINTN (*SET_FORCE_MODE)(BufferManager *This, UINTN WhichMode);
typedef UINTN (*IS_FORCE_MODE)(BufferManager *This);
typedef UINTN (*SET_SKIP_MODE)(BufferManager *This, UINTN WhichMode);
typedef UINTN (*IS_SKIP_MODE)(BufferManager *This);

typedef UINTN (*MOVE)(BufferManager *This, UINTN Number);
typedef UINTN (*MOVE_TO_STRING)(BufferManager *This, CHAR16 *String, UINTN StringLen);

typedef UINTN (*TRIM_SPACE)(BufferManager *This);

typedef UINTN (*SCOPE_TO_CHAR)(BufferManager *This);
typedef UINTN (*SCOPE_TO_WORD)(BufferManager *This);
typedef UINTN (*SCOPE_TO_LINE)(BufferManager *This);

typedef UINTN (*PUSH_STRING)(BufferManager *This, CHAR16 *String, UINTN StringLen);

typedef UINTN (*MATCH_STRING)(BufferManager *This, CHAR16 *String, UINTN StringLen);
typedef UINTN (*FIND_STRING)(BufferManager *This, CHAR16 *String, UINTN StringLen);
typedef UINTN (*IS_BUFFER_EMPTY)(BufferManager *This);

typedef UINTN (*OUTPUT_BUFFER)(BufferManager *This);

typedef UINTN (*IS_ITERATOR_EXCEED)(BufferManager *This);
typedef UINTN (*GET_SCOPE_SIZE)(BufferManager *This);
typedef UINTN (*IS_VISIBLE_CHAR)(CHAR16 *C);
typedef UINTN (*LOCATE_STRING)(BufferManager *This, UINTN WhichIterator, CHAR16 *String, UINTN StringLen, UINTN *Offset);

#pragma pack(1)
struct BufferManager_{
    CHAR16                              *buffer;                    ///< Managed input buffer.
    UINTN                               bufferLength;               ///< Unit is sizeof(CHAR16),not sizeof(BYTE)
    EFI_SHELL_PROTOCOL                  *shell;                     ///< A shell reference.

    CHAR16                              *frontIterator;             ///< Pointer to buffer's begin default.
    CHAR16                              *reverseIterator;           ///< Pointer to buffer's end default.

    UINTN                               mIteratorMode;              ///< Iterator switch.
    UINTN                               mForceMode;                 ///< Force mode switch.
    UINTN                               mSkipMode;                  ///< Skip mode switch.

    /// Public functions.
    BUFFER_MANAGER_CONSTRUCTOR          constructor;
    BUFFER_MANAGER_DESTRUCTOR           destructor;

    /// Mode settings.
    SET_ITERATOR_MODE                   setIteratorMode;
    IS_FRONT_ITERATOR_MODE              isFrontIteratorMode;
    SET_FORCE_MODE                      setForceMode;
    IS_FORCE_MODE                       isForceMode;
    SET_SKIP_MODE                       setSkipMode;
    IS_SKIP_MODE                        isSkipMode;

    /// Move iterators.
    MOVE                                move;
    MOVE_TO_STRING                      moveToString;

    /// Trim off spaces.
    TRIM_SPACE                          trimFrontSpace;
    TRIM_SPACE                          trimBackSpace;
    TRIM_SPACE                          trimSpace;

    /// Get sub scope functions.
    SCOPE_TO_CHAR                       scopeToChar;
    SCOPE_TO_WORD                       scopeToWord;
    SCOPE_TO_LINE                       scopeToLine;

    /// Push string.
    PUSH_STRING                         pushString;

    /// Test functions.
    MATCH_STRING                        matchString;
    FIND_STRING                         findString;
    IS_BUFFER_EMPTY                     isBufferEmpty;
    GET_SCOPE_SIZE                      getScopeSize;

    /// Output buffer.
    OUTPUT_BUFFER                       outputBuffer;

    /// Private functions.
    IS_ITERATOR_EXCEED                  isIteratorExceed;
    IS_VISIBLE_CHAR                     isVisibleChar;
    LOCATE_STRING                       locateString;
};
#pragma pack()

extern BufferManager gBufferManager;

/**
 * Construct BufferManager.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] param                             The instance of EFI_SHELL_PARAMETERS_PROTOCOL binding to this application.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN BufferManagerConstructor(BufferManager *This, EFI_SHELL_PARAMETERS_PROTOCOL *param);

/**
 * Destruct BufferManager.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN BufferManagerDestructor(BufferManager *This);

/**
 * Set active iterator which will used to do some movement.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] WhichIterator                     FRONT_ITERATOR or REVERSE_ITERATOR
 * 
 * @retval 1                                    Operation success.
 */
UINTN SetIteratorMode(BufferManager *This, UINTN WhichIterator);

/**
 * Check active iterator.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Front iterator actived.
 * @retval 0                                    Reverse iterator actived.
 */
UINTN IsFrontIteratorMode(BufferManager *This);

/**
 * Set force mode.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] WhichMode                         FORCE_MODE or NON_FORCE_MODE.
 * 
 * @retval 1                                    Operation success.
 */
UINTN SetForceMode(BufferManager *This, UINTN WhichMode);

/**
 * Check force mode.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Force mode actived.
 * @retval 0                                    Non Force mode actived.
 */
UINTN IsForceMode(BufferManager *This);

/**
 * Set skip mode.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] WhichMode                         SKIP_MODE or NON_SKIP_MODE.
 * 
 * @retval 1                                    Operation success.
 */
UINTN SetSkipMode(BufferManager *This, UINTN WhichMode);

/**
 * Check skip mode.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Skip mode actived.
 * @retval 0                                    Non skip mode actived.
 */
UINTN IsSkipMode(BufferManager *This);

/**
 * Move a iterator by a number.
 * You should set correct modes such as (active-iterator/force-mode/skip-mode) before using this function;
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] Number                            Number of chars to move.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN Move(BufferManager *This, UINTN Number);

/**
 * Locate and move to a string in BufferManager.
 * You should set correct modes such as (active-iterator/force-mode/skip-mode) before using this function;
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] String                            The pointer of input string.
 * @param[IN] StringLen                         The size of the String,do not contain the NULL terminator.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN MoveToString(BufferManager *This, CHAR16 *String, UINTN StringLen);

/**
 * Trim off buffer's front spaces(contain invisible chars).
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN TrimFrontSpace(BufferManager *This);

/**
 * Trim off buffer's back spaces(contain invisible chars).
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN TrimBackSpace(BufferManager *This);

/**
 * Trim off buffer's front and back spaces(contain invisible chars).
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN TrimSpace(BufferManager *This);

/**
 * Get a char of buffer at front iterator,the char is a visible char.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN ScopeToChar(BufferManager *This);

/**
 * Get a word of buffer at front iterator,
 * the word is a set of visible chars.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN ScopeToWord(BufferManager *This);

/**
 * Get a line of buffer at front iterator.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN ScopeToLine(BufferManager *This);

/**
 * Push a string to the scope,
 * if isFrontIteratorMode == 1,the string will pushed at front of the scope,
 * else the string will pushed at end of the scope.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN PushString(BufferManager *This, CHAR16 *String, UINTN StringLen);

/**
 * Test the buffer and a string if they are match or not;
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] String                            The pointer of input string.
 * @param[IN] StringLen                         The length of the String,do not contain the NULL terminator.
 * 
 * @retval 1                                    Match.
 * @retval 0                                    Not match.
 */
UINTN MatchString(BufferManager *This, CHAR16 *String, UINTN StringLen);

/**
 * Test the buffer if it contains a string or not;
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] String                            The pointer of input string.
 * @param[IN] StringLen                         The length of the String,do not contain the NULL terminator.
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN FindString(BufferManager *This, CHAR16 *String, UINTN StringLen);

/**
 * Test the buffer is empty or not.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Empty.
 * @retval 0                                    Not empty.
 */
UINTN IsBufferEmpty(BufferManager *This);

/**
 * Pipe out the buffer.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * @param[IN] param                             The instance of EFI_SHELL_PARAMETERS_PROTOCOL binding on this application.
 * 
 * @retval 1                                    Operation pass.
 * @retval 0                                    Operation failed.
 */
UINTN OutPutBuffer(BufferManager *This);

/**
 * Check front-iterator and reverse-iterator if they exceeds each other or not.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval 1                                    Yes.
 * @retval 0                                    No.
 */
UINTN IsIteratorExceed(BufferManager *This);

/**
 * Get the scope-buffer's size,which begin at front-iterator and end at reverse-iterator.
 * 
 * @param[IN] This                              The instance of BufferManager.
 * 
 * @retval ANY                                  Size of the scope.
 */
UINTN GetScopeSize(BufferManager *This);

/**
 * Check input char if it is visible or not.
 * 
 * @param[IN] C                                 A pointer of a char.
 * 
 * @retval 1                                    It is visible.
 * @retval 0                                    It is invisible.
 */
UINTN IsVisibleChar(CHAR16 *C);

/**
 * try to locate a string in the scope.
 * 
 * @param[IN]    This                           The instance of BufferManager.
 * @param[IN]    WhichIterator                  Start position(start iterator) to locate.
 * @param[IN]    String                         Input string address.
 * @param[IN]    StringLen                      Input string length,do not contain the NULL terminator.
 * @param[OUT]   *offset  OPTIONAL              The string's offset in the scope,this offset is relative to [WhichIterator].
 * 
 * @retval 1                                    Operation success.
 * @retval 0                                    Operation failed.
 */
UINTN LocateString(BufferManager *This, UINTN WhichIterator, CHAR16 *String, UINTN StringLen, UINTN *Offset);

#endif //__BUFFER_MANAGER_H__
