#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <Uefi.h>

// Max digit-chars to show, it should be no more than 10.
#define COUNTER_MAX_WIDTH       10
// Max count number, it should be no more than INT64_MAX.
#define COUNTER_MAX_COUNT       9999999999
// Counter log file name.
#define COUNTER_LOG_FILE_NAME   L".\\counter.efi.log"
// Counter version.
#define COUNTER_VERSION_STR     L"counter.efi version 0.1.0"
// Max delay time in millisecond.
#define COUNTER_MAX_DELAY_TIME  60000
// Export uefi environment variable.
#define COUNTER_LAST_VALUE_STR  L"CounterLastValue"

#ifndef NO_VERBOSE
#define PrintError(message, ...) Print(L"[%a]: " message, "counter", ##__VA_ARGS__)
#else
#define PrintError(message, ...)
#endif

typedef struct Counter_ Counter;
typedef EFI_STATUS (*COUNTER_GET_COUNT)(Counter* This, UINTN* Count);
typedef EFI_STATUS (*COUNTER_ADD_TO_COUNT)(Counter* This, UINTN i);
typedef EFI_STATUS (*COUNTER_CLEAR_COUNT)(Counter* This);
typedef EFI_STATUS (*COUNTER_SHOW_COUNT)(Counter* This);
typedef EFI_STATUS (*COUNTER_SET_DELAY_TIME)(Counter* This, UINTN Milliseconds);
typedef UINTN (*COUNTER_GET_DELAY_TIME)(Counter* This);
typedef EFI_STATUS (*COUNTER_SET_EXPORT_ENV)(Counter* This, BOOLEAN value);
typedef BOOLEAN (*COUNTER_GET_EXPORT_ENV)(Counter* This);
typedef EFI_STATUS (*COUNTER_EXPORT_ENV)(Counter* This, UINTN Count, BOOLEAN IsUsingDelete);
typedef EFI_STATUS (*COUNTER_SHOW_VERSION)(Counter* This);
typedef EFI_STATUS (*COUNTER_SHOW_HELP)(Counter* This);
typedef EFI_STATUS (*COUNTER_DESTROY)(Counter** This);

struct Counter_
{
    COUNTER_GET_COUNT       GetCount;
    COUNTER_ADD_TO_COUNT    AddToCount;
    COUNTER_CLEAR_COUNT     ClearCount;
    COUNTER_SHOW_COUNT      ShowCount;
    COUNTER_SET_DELAY_TIME  SetDelayTime;
    COUNTER_GET_DELAY_TIME  GetDelayTime;
    COUNTER_SET_EXPORT_ENV  SetExportEnv;
    COUNTER_GET_EXPORT_ENV  GetExportEnv;
    COUNTER_EXPORT_ENV      ExportEnv;
    COUNTER_SHOW_VERSION    ShowVersion;
    COUNTER_SHOW_HELP       ShowHelp;
    COUNTER_DESTROY         Destroy;
};

/**
 * @brief Counter constructor.
 * 
 * @return Counter*     A Counter instance.
 */
Counter* CounterCreate();

/**
 * @brief Get count.
 * 
 * @param[in] This          Counter instance.
 * @param[out] Count        Current count.
 * 
 * @retval EFI_SUCCESS      Operation success.
 * @retval Others           Operation fail.
 */
EFI_STATUS CounterGetCount(Counter* This, UINTN* Count);

/**
 * @brief Add a number i to count.
 * 
 * @param[in] This          Counter instance.
 * @param[in] i             A number
 * 
 * @retval EFI_SUCCESS      Operation success.
 * @retval Others           Operation fail.
 */
EFI_STATUS CounterAddToCount(Counter* This, UINTN i);

/**
 * @brief Clear and reset count to zero.
 * 
 * @param[in] This          Counter instance.
 * 
 * @retval EFI_SUCCESS      Operation success.
 * @retval Others           Operation fail.
 */
EFI_STATUS CounterClearCount(Counter* This);

/**
 * @brief Show count.
 * 
 * @param[in] This          Counter instance.
 * 
 * @retval EFI_SUCCESS      Operation success.
 * @retval Others           Operation fail.
 */
EFI_STATUS CounterShowCount(Counter* This);

/**
 * @brief Set delay time when show the count.
 * 
 * @param[in] This          Counter instance.
 * @param[in] Milliseconds  Delay time to be set.
 * 
 * @retval EFI_SUCCESS      Operation success.
 */
EFI_STATUS CounterSetDelayTime(Counter* This, UINTN Milliseconds);

/**
 * @brief Get the delay time.
 * 
 * @param[in] This      Counter instance.
 *    
 * @return UINTN        Get delay time in millisecond.
 */
UINTN CounterGetDelayTime(Counter* This);

/**
 * @brief Set switch for Export environment variable.
 * 
 * @param[in] This      Counter instance.
 * @param[in] value     Value to be set.
 * @return EFI_SUCCESS 
 */
EFI_STATUS CounterSetExportEnv(Counter* This, BOOLEAN value);

/**
 * @brief Get switch for Export environment variable.
 * 
 * @param[in] This      Counter instance.
 * @return BOOLEAN 
 */
BOOLEAN CounterGetExportEnv(Counter* This);

/**
 * @brief Export environment variable.
 * 
 * @param[in] This              Counter instance.
 * @param[in] Count             The count number to be set.
 * @param[in] IsUsingDelete     Delete the environment variable if TRUE.
 * 
 * @return EFI_STATUS 
 */
EFI_STATUS CounterExportEnv(Counter* This, UINTN Count, BOOLEAN IsUsingDelete);

/**
 * @brief Show version.
 * 
 * @param[in] This          Counter instance.
 * 
 * @retval EFI_SUCCESS      Operation success.
 */
EFI_STATUS CounterShowVersion(Counter* This);

/**
 * @brief Show Help.
 * 
 * @param[in] This          Counter instance.
 * 
 * @retval EFI_SUCCESS      Operation success.
 */
EFI_STATUS CounterShowHelp(Counter* This);

/**
 * @brief Counter destructor.
 * 
 * @param[in] This          Counter instance address.
 * 
 * @retval EFI_SUCCESS      Operation success.
 */
EFI_STATUS CounterDestroy(Counter** This);

#endif //_COUNTER_H_
