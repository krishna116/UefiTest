#ifndef _REGEX_WORKER_H_
#define _REGEX_WORKER_H_

#include <Uefi.h>

#include <Protocol/Shell.h>
#include <Protocol/ShellParameters.h>

/// Buffer limits.
#define EFI_REGEX_PATTERN_STR_MAX_COUNT     (256)
#define EFI_REGEX_TEXT_STR_MAX_COUNT        (80*25*4)
/// Environment variable names.
#define REGEX_LAST_MATCH_STR                L"RegexLastMatchStr"
#define REGEX_LAST_MATCH_STR_Offset         L"RegexLastMatchStrOffset"
#define REGEX_LAST_MATCH_STR_LENGTH         L"RegexLastMatchStrLength"
/// App version.
#define EFI_REGEX_VERSION_STR               L"regex.efi version 0.1.0"

#ifndef NO_VERBOSE
#define PrintError(message, ...) Print(L"[%a]: " message, "regex", ##__VA_ARGS__)
#else
#define PrintError(message, ...)
#endif

typedef enum {
    OutputTypeAll = 0,      ///< Output all matched items.
    OutputTypeFirst,        ///< Output the first matched item.
    OutputTypeLast          ///< Output the last matched item.
}OutputType;

typedef struct RegexWorker_ RegexWorker;
typedef EFI_STATUS (*REGEX_WORKER_DO_WORK)(RegexWorker* This);
typedef EFI_STATUS (*REGEX_WORKER_SET_PATTERN_STR)(RegexWorker* This, CONST CHAR16* Str);
typedef CHAR16* (*REGEX_WORKER_GET_PATTERN_STR)(RegexWorker* This);
typedef EFI_STATUS (*REGEX_WORKER_SET_OUTPUT_TYPE)(RegexWorker* This, OutputType type);
typedef OutputType (*REGEX_WORKER_GET_OUTPUT_TYPE)(RegexWorker* This);
typedef VOID (*REGEX_WORKER_DESTROY)(RegexWorker** This);

typedef struct{
    EFI_SHELL_PROTOCOL *Shell;
    EFI_SHELL_PARAMETERS_PROTOCOL   *Param;
    CHAR16 PatternStr[EFI_REGEX_PATTERN_STR_MAX_COUNT+1];
    CHAR16 Text[EFI_REGEX_TEXT_STR_MAX_COUNT+1];
    OutputType ThisOutputType;
}RegexWorkerPrivateInfo;

struct RegexWorker_
{
    //public:
    REGEX_WORKER_DO_WORK                DoWork;
    REGEX_WORKER_SET_PATTERN_STR        SetPatternStr;
    REGEX_WORKER_GET_PATTERN_STR        GetPatternStr;
    REGEX_WORKER_SET_OUTPUT_TYPE        SetOutputType;
    REGEX_WORKER_GET_OUTPUT_TYPE        GetOutputType;
    REGEX_WORKER_DESTROY                Destroy;
    //private:
    RegexWorkerPrivateInfo              Private_;
};

/**
 * @brief RegexWorker constructor.
 * 
 * @return NOT NULL     Operation success.
 * @return NULL         Operation fail.
 */
RegexWorker* RegexWorkerCreate();

/**
 * @brief Do the match work.
 * It should be used after RegexWorker->SetPatternStr.
 * 
 * @param[in] This          A RegexWorker instance.
 * 
 * @return EFI_STATUS       Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS RegexWorkerDoWork(RegexWorker* This);

/**
 * @brief Set pattern str.
 * 
 * @param[in] This          A RegexWorker instance.
 * @param[in] Str           Input pattern str.
 *
 * @return EFI_STATUS       Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS RegexWorkerSetPatternStr(RegexWorker* This, CONST CHAR16* Str);

/**
 * @brief Get pattern str.
 * 
 * @param[in] This          A RegexWorker instance.
 * 
 * @return CHAR16*          The pattern str.
 */
CHAR16* RegexWorkerGetPatternStr(RegexWorker* This);

/**
 * @brief Set output type.
 * 
 * @param[in] This      A RegexWorker instance.
 * @param[in] type      Output type to be set.
 * 
 * @return EFI_SUCCESS 
 */
EFI_STATUS RegexWorkerSetOutputType(RegexWorker* This, OutputType type);

/**
 * @brief Get output type.
 * 
 * @param[in] This      A RegexWorker instance.
 * 
 * @return OutputType   Current output type.
 */
OutputType RegexWorkerGetOutputType(RegexWorker* This);

/**
 * @brief RegexWorker destructor.
 * 
 * @param[in] This      A RegexWorker instance's address.
 */
VOID RegexWorkerDestroy(RegexWorker** This);

#endif //_REGEX_WORKER_H_
