#ifndef __ARGUMENT_MANAGER_H__
#define __ARGUMENT_MANAGER_H__

typedef struct ArgumentManager_ ArgumentManager;

typedef UINTN (*ARGUMENT_MANAGER_CONSTRUCTOR)(ArgumentManager *This);
typedef UINTN (*PEEK_ARG)(ArgumentManager *This);
typedef UINTN (*SIZE)(ArgumentManager *This);
typedef CHAR16 *(*GET_NEXT_ARG)(ArgumentManager *This);
typedef UINTN (*GET_NEXT_ARG_AND_COMPARE)(ArgumentManager *This, CHAR16 *String);
typedef VOID (*BACK_UP_INDEX)(ArgumentManager *This);
typedef VOID (*RESTORE_INDEX)(ArgumentManager *This);

struct ArgumentManager_
{
    EFI_SHELL_PARAMETERS_PROTOCOL *param; ///< a ParameterProtocol reference.
    UINTN currentIndex;                   ///< Current index of argument.
    UINTN backup;                         ///< used for backup currentIndex.

    /// public functions.
    ARGUMENT_MANAGER_CONSTRUCTOR    constructor;
    SIZE                            size;
    PEEK_ARG                        peekArg;
    GET_NEXT_ARG                    getNextArg;
    GET_NEXT_ARG_AND_COMPARE        getNextArgAndCompare;

    BACK_UP_INDEX                   backupIndex;
    RESTORE_INDEX                   restoreIndex;
};

extern ArgumentManager gArgumentManager;

/**
 * Construct ArgumentManager.
 * 
 * @param[IN] This                  The instance of ArgumentManager.
 * 
 * @retval 1                        Operation success.
 * @retval 0                        Operation failed.
 * 
 */
UINTN ArgumentManagerConstructor(ArgumentManager *This);

/**
 * Get the size of the argument list.
 * 
 * @param[IN] This                  The instance of ArgumentManager.
 * 
 * @retval size of the argument list.
 */
UINTN Size(ArgumentManager *This);

/**
 * Check next argument exist or not.
 * 
 * @param[IN] This                  Pointer to the ArgumentManager instance.
 * 
 * @retval 1                        Peek ok.
 * @retval 0                        Peek failed.
 */
UINTN PeekArg(ArgumentManager *This);

/**
 * Get next argument from the argument list.
 * 
 * @param[IN] This                  Pointer to the ArgumentManager instance.
 * 
 * @retval NOT NULL                 Operation success and return an argument.
 * @retval NULL                     Operation failed.
 */
CHAR16 *GetNextArg(ArgumentManager *This);

/**
 * Get next argument from the argument list,and compare with the string if they are match or not.
 * 
 * @param[in] This                  Pointer to the ArgumentManager instance.
 * @param[IN] String                A string to compare.   
 * 
 * @retval 1                        Operation pass.
 * @retval 0                        Operation failed.
 */
UINTN GetNextArgAndCompare(ArgumentManager *This, CHAR16 *String);

/**
 * Backup currentIndex in order to get this argument again.
 * 
 * @param[IN] This                  Pointer to the ArgumentManager instance.
 * 
 */
VOID BackupIndex(ArgumentManager *This);

/**
 * Restore currentIndex.
 * 
 * @param[IN] This                  Pointer to the ArgumentManager instance.
 * 
 */
VOID RestoreIndex(ArgumentManager *This);

#endif //__ARGUMENT_MANAGER_H__
