#ifndef _XDB_H_
#define _XDB_H_

#include <Uefi.h>

#include <Library/JsonLib.h>
#include <Protocol/Shell.h>

#define XDB_VERSION_STR             L"xdb.efi version 0.1.0"
#define XDB_DEFAULT_INPUT_FILE      L".\\xdb.efi.log"
#define XDB_INPUT_FILE_SIZE_MAX     (1024 * 1024 * 2)
#define XDB_ENV_VARIABLE_KEY        "XdbLastKey"
#define XDB_ENV_VARIABLE_VALUE      "XdbLastValue"
#define XDB_DEFAULT_NULL_STR        "null"
#define XDB_TOKEN_SIZE_MAX          256

#ifndef NO_VERBOSE
#define PrintError(message, ...) Print(L"[%a]: " message, "xdb-error", ##__VA_ARGS__)
#else
#define PrintError(message, ...)
#endif

typedef struct Xdb_ Xdb;
// Public part.
typedef EFI_STATUS (*XDB_SET)(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value);
typedef EFI_STATUS (*XDB_GET)(Xdb *This, CONST CHAR16 *Key);
typedef EFI_STATUS (*XDB_PUSH)(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value, BOOLEAN isPushFront);
typedef EFI_STATUS (*XDB_POP)(Xdb *This, BOOLEAN isPopFront);
typedef EFI_STATUS (*XDB_APPEND)(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value);
typedef EFI_STATUS (*XDB_CLEAR)(Xdb *This, CONST CHAR16 *Key);
typedef EFI_STATUS (*XDB_SET_FILE_NAME)(Xdb *This, CONST CHAR16 *FileName);
typedef CHAR16 *(*XDB_GET_FILE_NAME)(Xdb *This);
typedef EFI_STATUS (*XDB_SET_VERBOSE_MODE)(Xdb *This, BOOLEAN Value);
typedef BOOLEAN (*XDB_GET_VERBOSE_MODE)(Xdb *This);
typedef VOID (*XDB_DESTROY)(Xdb **This);

// Private part.
typedef EDKII_JSON_VALUE (*XDB_LOAD_JSON)(Xdb *This);
typedef BOOLEAN (*XDB_CHECK_JSON)(Xdb *This, EDKII_JSON_VALUE Root);
typedef EFI_STATUS (*XDB_SAVE_JSON)(Xdb *This, EDKII_JSON_VALUE Root);
typedef VOID (*XDB_CREATE_DEFATLT_JSON_FILE)(Xdb *This, BOOLEAN IsForce);
typedef EFI_STATUS (*XDB_UPDATE_ENV_VARIABLE)(Xdb *This, CONST CHAR8 *EnvName, CONST CHAR8 *EnvValue);
typedef CHAR8* (*XDB_UNICODE_TO_ASCII)(Xdb* This, CONST CHAR16* Str);
typedef CHAR16* (*XDB_ASCII_TO_UNICODE)(Xdb* This, CONST CHAR8* Str);
typedef EFI_STATUS (*XDB_COPY_STRING)(Xdb* This, CONST CHAR8* InString, CHAR8**OutString);

typedef EFI_STATUS (*XDB_INTERNAL_SET)(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value);
typedef EFI_STATUS (*XDB_INTERNAL_GET)(Xdb *This, CONST CHAR8 *Key);
typedef EFI_STATUS (*XDB_INTERNAL_PUSH)(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value, BOOLEAN isPushFront);
typedef EFI_STATUS (*XDB_INTERNAL_CLEAR)(Xdb *This, CONST CHAR8 *Key);
typedef VOID (*XDB_DUMP_KEY_VALUE)(Xdb *This, CONST CHAR8 *Prefix, CONST CHAR8 *Key, CONST CHAR8 *Value);
typedef BOOLEAN (*XDB_FIND_KEY)(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *Key, UINTN *Index);

typedef EFI_STATUS (*XDB_ROOT_ARRAY_REMOVE_BY_KEY)(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey);
typedef EFI_STATUS (*XDB_ROOT_ARRAY_REMOVE_ITEM_BY_INDEX)(Xdb *This, EDKII_JSON_VALUE Root, UINTN Index);
typedef EFI_STATUS (*XDB_ROOT_ARRAY_GET_KEY_VALUE_BY_KEY)(Xdb* This, EDKII_JSON_VALUE Root,  CONST CHAR8 *InputKey, CHAR8 **OutKey, CHAR8 **OutValue);
typedef EFI_STATUS (*XDB_ROOT_ARRAY_GET_KEY_VALUE_BY_INDEX)(Xdb* This, EDKII_JSON_VALUE Root,  UINTN Index, CHAR8 **OutKey, CHAR8 **OutValue);
typedef EFI_STATUS (*XDB_ROOT_ARRAY_UPDATE)(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue);
typedef EFI_STATUS (*XDB_ROOT_ARRAY_INSERT)(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue, UINTN Index);
typedef EFI_STATUS (*XDB_EXTRACT_KEY_AND_VALUE_FROM_JSON_KEY_VALUE)(Xdb *This, EDKII_JSON_VALUE JsonKeyValue, CHAR8 **OutKey, CHAR8 **OutValue);

typedef struct
{
    XDB_CREATE_DEFATLT_JSON_FILE                    CreateDefaultJsonFile;
    XDB_LOAD_JSON                                   LoadJson;
    XDB_CHECK_JSON                                  CheckJson;
    XDB_SAVE_JSON                                   SaveJson;
    XDB_UPDATE_ENV_VARIABLE                         UpdateEnvVariable;
    XDB_UNICODE_TO_ASCII                            UnicodeToAscii;
    XDB_ASCII_TO_UNICODE                            AsciiToUnicode;
    XDB_COPY_STRING                                 CopyString;

    XDB_INTERNAL_SET                                InternalSet;
    XDB_INTERNAL_GET                                InternalGet;
    XDB_INTERNAL_PUSH                               InternalPush;
    XDB_INTERNAL_CLEAR                              InternalClear;
    XDB_DUMP_KEY_VALUE                              DumpKeyValue;
    XDB_FIND_KEY                                    FindKey;

    XDB_ROOT_ARRAY_REMOVE_BY_KEY                    RootArrayRemoveByKey;
    XDB_ROOT_ARRAY_REMOVE_ITEM_BY_INDEX             RootArrayRemoveItemByIndex;
    XDB_ROOT_ARRAY_GET_KEY_VALUE_BY_KEY             RootArrayGetKeyValueByKey;
    XDB_ROOT_ARRAY_GET_KEY_VALUE_BY_INDEX           RootArrayGetKeyValueByIndex;
    XDB_ROOT_ARRAY_UPDATE                           RootArrayUpdate;
    XDB_ROOT_ARRAY_INSERT                           RootArrayInsert;
    XDB_EXTRACT_KEY_AND_VALUE_FROM_JSON_KEY_VALUE   ExtractKeyAndValueFromJsonKeyValue;

    EFI_SHELL_PROTOCOL *                            Shell;
    CHAR16 *                                        FileName;
    BOOLEAN                                         IsVerboseMode;
} XdbPrivateInfo;

struct Xdb_
{
    //public:
    XDB_SET                 Set;
    XDB_GET                 Get;
    XDB_PUSH                Push;
    XDB_POP                 Pop;
    XDB_APPEND              Append;
    XDB_CLEAR               Clear;
    XDB_SET_FILE_NAME       SetFileName;
    XDB_GET_FILE_NAME       GetFileName;
    XDB_SET_VERBOSE_MODE    SetVerboseMode;
    XDB_GET_VERBOSE_MODE    GetVerboseMode;
    XDB_DESTROY             Destroy;

    //private:
    XdbPrivateInfo          private_;
};

/**
 * @brief Create a xdb instance.
 * 
 * @return NOT NULL     A xdb instance.
 * @return NULL         Create xdb instance failed.
 */
Xdb *XdbCreate();

/**
 * @brief Set key-value pair.
 * If the key is not exist in the database, the key will be created and
 * it will push-back to the database.
 * If the key is exist in the database, the key's value will be updated.
 * 
 * If set key-value success, in default mode, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = value
 * key-value will also output to stdout.
 * 
 * If set key-value fail, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * Error message will output to stdout.
 * 
 * @param[in] This      A xdb instance.
 * @param[in] Key       Input key.
 * @param[in] Value     Input value.
 * 
 * @return EFI_SUCCESS  Operation success.
 * @return Others       Operation fail.
 */
EFI_STATUS XdbSet(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value);

/**
 * @brief Get key-value pair by key.
 * If get key-value success, in default mode, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = value
 * key-value will also output to stdout.
 * 
 * If get key-value fail, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * Error message will output to stdout.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Key           Input key.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbGet(Xdb *This, CONST CHAR16 *Key);

/**
 * @brief Push key-value pair.
 * If push key-value success, in default mode,
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = value
 * key-value will also output to stdout.
 * 
 * If push key-value fail, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * Error message will output to stdout.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Key           Input key.
 * @param[in] Value         Input value.
 * @param[in] isPushFront   push front or push back.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbPush(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value, BOOLEAN isPushFront);

/**
 * @brief Pop a key-value pair.
 * If Pop key-value success, in default mode, 
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = value
 * key-value will also output to stdout.
 * 
 * If Pop key-value fail,
 * XDB_ENV_VARIABLE_KEY = XDB_DEFAULT_NULL_STR
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * Error message will output to stdout.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] isPopFront    pop front or pop back.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbPop(Xdb *This, BOOLEAN isPopFront);

/**
 * @brief Set input file name.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] FileName      File name.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */

/**
 * @brief Append key-value pair.
 * This function do not verify input file format and
 * do not check duplicated keys in order to do fast 
 * append key-value to the database.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] Key               Input key.
 * @param[in] Value             Input value.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbAppend(Xdb *This, CONST CHAR16 *Key, CONST CHAR16 *Value);

/**
 * @brief Set input file.
 * 
 * @param[in] This                  A xdb instance.
 * @param[in] FileName              Filename.
 * @return EFI_STATUS 
 */
EFI_STATUS XdbSetFileName(Xdb *This, CONST CHAR16 *FileName);

/**
 * @brief Get input file name.
 * 
 * @param[in] This      A xdb instance.
 * 
 * @return NOT NULL     The fileName str.
 * @return NULL         The filename is empty.
 */
CHAR16 *XdbGetFileName(Xdb *This);

/**
 * @brief Clear a key-value or clear the input file.
 * If input key is empty string "", the input file will be cleared,
 * If input key is not empty string, it will try to find the key and
 * delete the key-value pair.
 * 
 * if clear a key-value pass:
 * XDB_ENV_VARIABLE_KEY = key
 * XDB_ENV_VARIABLE_VALUE = value
 * key-value will also output to stdout.
 * 
 * if clear a key-value fail:
 * XDB_ENV_VARIABLE_KEY = XDB_DEFAULT_NULL_STR
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * 
 * if clear a json file:
 * XDB_ENV_VARIABLE_KEY = XDB_DEFAULT_NULL_STR
 * XDB_ENV_VARIABLE_VALUE = XDB_DEFAULT_NULL_STR
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Key           Input key.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbClear(Xdb *This, CONST CHAR16 *Key);

/**
 * @brief Set verbose mode and show more information.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Value         TRUE or FALSE
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbSetVerboseMode(Xdb *This, BOOLEAN Value);

/**
 * @brief Get verbose mode.
 * 
 * @param[in] This      A xdb instance.
 * 
 * @return BOOLEAN      Current verbose mode.
 */
BOOLEAN XdbGetVerboseMode(Xdb *This);

/**
 * @brief Xdb destructor.
 * 
 * @param[in] This      A xdb instance's address.
 */
VOID XdbDestroy(Xdb **This);

/**
 * @brief The database internal is a json file, this function will save it to a json file.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbSaveJson(Xdb *This, EDKII_JSON_VALUE Root);

/**
 * @brief The database internal is a json file, this function will load it from a json file.
 * 
 * @param[in] This          A xdb instance.
 * If return not null, return is the json's root.
 * If return not null, it is user's responsibility to free it(using JsonDecreaseReference).
 * 
 * @return NOT NULL         Operation success.
 * @return NULL             Operation fail.
 */
EDKII_JSON_VALUE XdbLoadJson(Xdb *This);

/**
 * @brief Check json if it is valid or not.
 * The json should match follow grammar:
 *  JsonRoot        -> Array
 *  Array           -> "[" {[JsonObject] {"," [JsonObject]}*} "]"
 *  JsonObject      -> "{" key "=" value "}"
 *  key             -> string
 *  value           -> string
 *  string          -> Ascii json string, the string length must <= XDB_TOKEN_SIZE_MAX
 * 
 * @param[in] This      A xdb instance.
 * @param[in] Root      A json's root.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
BOOLEAN XdbCheckJson(Xdb *This, EDKII_JSON_VALUE Root);

/**
 * @brief Try to create default json file if input file is not exist.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] IsForce       Always create new file.
 */
VOID XdbCreateDefaultJsonFile(Xdb *This, BOOLEAN IsForce);

/**
 * @brief Update uefi shell environment variable.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] EnvName           Environment variable name.
 * @param[in] EnvValue          Environment variable value.
 * 
 * @return EFI_SUCCESS          Operation success.
 * @return Others               Operation fail.
 */
EFI_STATUS XdbUpdateEnvVariable(Xdb *This, CONST CHAR8 *EnvName, CONST CHAR8 *EnvValue);

/**
 * @brief Convert ucs2 string to Ascii string.
 * 
 * @param[in] This      A xdb instance.
 * @param[in] Str       Input ucs2 string.
 * @return CHAR8*       Output ascii string.
 */
CHAR8* XdbUnicodeToAscii(Xdb* This, CONST CHAR16* Str);

/**
 * @brief Convert Ascii string to ucs2 string.
 * 
 * @param[in] This      A xdb instance.
 * @param[in] Str       Input ascii string.
 * @return CHAR8*       Output ucs2 string.
 */
CHAR16* XdbAsciiToUnicode(Xdb* This, CONST CHAR8* Str);

/**
 * @brief Copy string buffer.
 * If *OutString is not null, it is user's responsibility to free it.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] InString      Input string.
 * @param[out] OutString    Output string address.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbCopyString(Xdb* This, CONST CHAR8* InString, CHAR8**OutString);

/**
 * @brief Internal set key-value.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] Key               Input key.
 * @param[in] Value             Input value.
 * 
 * @return EFI_SUCCESS          Operation success.
 * @return Others               Operation fail.
 */
EFI_STATUS XdbInternalSet(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value);

/**
 * @brief Internal get key-value by key.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] Key               Input key.
 * 
 * @return EFI_SUCCESS          Operation success.
 * @return Others               Operation fail.
 */
EFI_STATUS XdbInternalGet(Xdb *This, CONST CHAR8 *Key);

/**
 * @brief Internal push key-value.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] Key               Input key.
 * @param[in] Value             Input value.
 * @param[in] isPushFront       Push front or push back.
 * 
 * @return EFI_SUCCESS          Operation success.
 * @return Others               Operation fail.
 */
EFI_STATUS XdbInternalPush(Xdb *This, CONST CHAR8 *Key, CONST CHAR8 *Value, BOOLEAN isPushFront);

/**
 * @brief Internal clear key-value or input json file.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Key           Input key.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbInternalClear(Xdb *This, CONST CHAR8 *Key);

/**
 * @brief Dump key-value and export them to environment variable.
 * This function usually used when get/set/push/pop/clear key-value success.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Prefix        A prefix string.
 * @param[in] Key           Input key.
 * @param[in] Value         Input value.
 */
VOID XdbDumpKeyValue(Xdb *This, CONST CHAR8 *Prefix, CONST CHAR8 *Key, CONST CHAR8 *Value);

/**
 * @brief Find the key in the json-root-tree and output the key's index if success.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] Key           Input key.
 * @param[out] Index        Output the key's index in the json-array(the json's root is also a array).
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
BOOLEAN XdbFindKey(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *Key, UINTN *Index);

/**
 * @brief Remove key-value item by key.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] InputKey      Input key.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayRemoveByKey(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey);

/**
 * @brief Remove key-value item by index.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] Index         Input key's index.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayRemoveItemByIndex(Xdb *This, EDKII_JSON_VALUE Root, UINTN Index);

/**
 * @brief Get key-value by key.
 * if return EFI_SUCCESS, the *OutKey and *OutValue are not null and
 * it is user's responsibility to free it.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] InputKey      Input key.
 * @param[out] OutKey       Output key.
 * @param[out] OutValue     Output value.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayGetKeyValueByKey(Xdb* This, EDKII_JSON_VALUE Root,  CONST CHAR8 *InputKey, CHAR8 **OutKey, CHAR8 **OutValue);

/**
 * @brief Get key-value by index.
 * if return EFI_SUCCESS, the *OutKey and *OutValue are not null and
 * it is user's responsibility to free it.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] Index         Input index.
 * @param[out] OutKey       Output key.
 * @param[out] OutValue     Output value.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayGetKeyValueByIndex(Xdb* This, EDKII_JSON_VALUE Root,  UINTN Index, CHAR8 **OutKey, CHAR8 **OutValue);

/**
 * @brief Update a key's value.
 * Before using this function, the key should exist.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] InputKey      Input key.
 * @param[in] InputValue    Input value.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayUpdate(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue);

/**
 * @brief Insert a key-value.
 * Before using this function, the key should not exist in the json,
 * because we don't support duplicated keys in one json file in design.
 * 
 * @param[in] This          A xdb instance.
 * @param[in] Root          The json's root.
 * @param[in] InputKey      Input key.
 * @param[in] InputValue    Input value.
 * @param[in] Index         A position in the json-array to insert.
 * 
 * @return EFI_SUCCESS      Operation success.
 * @return Others           Operation fail.
 */
EFI_STATUS XdbRootArrayInsert(Xdb *This, EDKII_JSON_VALUE Root, CONST CHAR8 *InputKey, CONST CHAR8 *InputValue, UINTN Index);

/**
 * @brief Extract key-string and value-string from json-key-value-object.
 * If return success, the *OutKey and *OutValue are not null and
 * it is user's responsibility to free them.
 * 
 * @param[in] This              A xdb instance.
 * @param[in] JsonKeyValue      A json object.
 * @param[out] OutKey           Output key.
 * @param[out] OutValue         Output value.
 * 
 * @return EFI_SUCCESS          Operation success.
 * @return Others               Operation fail.
 */
EFI_STATUS XdbExtractKeyAndValueFromJsonKeyValue(Xdb *This, EDKII_JSON_VALUE JsonKeyValue, CHAR8 **OutKey, CHAR8 **OutValue);

EDKII_JSON_VALUE XdbOpenFileAndReadJson(Xdb *This);
#endif //_XDB_H_
