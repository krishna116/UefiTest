#ifndef __HISTORIAN__H__
#define __HISTORIAN__H__

#define WCHAR_LF                    0x000A
#define WCHAR_CR                    0x000D
#define WCHAR_F                     0x0046
#define WCHAR_S                     0x0053
#define WCHAR_SPACE                 0x0020
#define WCHAR_BS                    0x0008

typedef struct historian_ Historian;

typedef UINTN (*HISTORIAN_CONSTRUCTOR)(Historian *This);
typedef UINTN (*HISTORIAN_DESTRUCTOR)(Historian *This);
typedef UINTN (*TELL_HISTORY)(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine);
typedef UINTN (*PREPARE_OUTPUT_BUFFER)(Historian *This, CHAR16 *SourceBuffer, UINTN SourceRowCount, UINTN SourceColumnLength, CHAR16 **OutputBuffer, UINTN *OutputBufferLength);

struct historian_{
    CHAR16                          *buffer;                ///< Make a copy from UEFI Shell's internal screen buffer.
    UINTN                           bufferLength;           ///< Buffer item size,unit is CHAR16 not byte.
    UINTN                           rowsPerScreen;          ///< Record UEFI Shell's internal console's height.
    UINTN                           colsPerScreen;          ///< Record UEFI Shell's internal console's width.
    EFI_HANDLE                      *handleBuffer;          ///< Store temp Handle buffer.
    UINTN                           handleBufferLength;     ///< Temp Handle buffer length.
    EFI_SHELL_PARAMETERS_PROTOCOL   *param;                 ///< The param binded on this imageHandle.
    EFI_SHELL_PROTOCOL              *shell;                 ///< A shell reference.

    /* public functions */
    HISTORIAN_CONSTRUCTOR           constructor;            ///< Construct this.
    HISTORIAN_DESTRUCTOR            destructor;             ///< Destruct this.
    TELL_HISTORY                    tellStory;              ///< Tell recent lines of screen.
};

extern Historian gHistorian;

/**
 * Construct a Historian.
 * @param[IN] This                  The instance of Historian.
 * 
 * @retval 1                        Operation pass.
 * @retval 0                        Operation failed.
 */
UINTN HistorianConstructor(Historian *This);

/**
 * Destruct a Historian.
 * @param[IN] This                  The instance of Historian.
 * 
 * @retval 1                        Operation pass.
 * @retval 0                        Operation failed.
 */
UINTN HistorianDestructor(Historian *This);

/**
 * Tell recent lines of history messages.
 * 
 * @param[IN] This                  The instance of Historian.
 * @parma[IN] DesiredRowCount       How many recent history-message-lines to output,DesiredRowCount > 0.
 * @param[IN] IsOnlyGetLastLine     It is the last one line(counting from down to top)we will output.(It is our first line in screen-buffer's view.)
 * 
 * @retval 1                        Operation pass.
 * @retval 0                        Operation failed.
 */
UINTN TellStory(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine);

/**
 * Debug function,used to dump the screen-buffer-raw-data.
 * 
 * @param[IN] This                  The instance of Historian.
 */
VOID Dump(Historian *This);

#endif //__HISTORIAN__H__