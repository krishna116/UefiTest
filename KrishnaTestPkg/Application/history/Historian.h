#ifndef __HISTORIAN__H__
#define __HISTORIAN__H__

typedef struct historian_ Historian;

typedef Historian* (*HISTORIAN_CREATE)();
typedef EFI_STATUS (*HISTORIAN_TELL_HISTORY)(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine);
typedef VOID (*HISTORIAN_DESTROY)(Historian **This);

typedef struct {
    EFI_SHELL_PARAMETERS_PROTOCOL   *Param;
    CHAR16                          *ScreenBuffer;
    UINTN                           ScreenBufferLength;
    UINTN                           RowsPerScreen;
    UINTN                           ColsPerScreen;
}HistorianPrivateInfo;

struct historian_{
    //public:
    HISTORIAN_TELL_HISTORY          TellStory;
    HISTORIAN_DESTROY               Destroy;

    //private:
    HistorianPrivateInfo            Private_;
};

/**
 * @brief Create a new historian.
 * 
 * @return NOT NULL     A new historian instance.
 * @return NULL         Operation fail.
 */
Historian* HistorianCreate();

/**
 * @brief Read console text-lines and output them.
 * 
 * @param[in] This                  A historian instance
 * @param[in] DesiredRowCount       Max desired row count.
 * @param[in] IsOnlyGetLastLine     Only output last line.
 * 
 * @return EFI_SUCCESS              Operation success.
 * @return Others                   Operation fail.
 */
EFI_STATUS HistorianTellStory(Historian *This, UINTN DesiredRowCount, BOOLEAN IsOnlyGetLastLine);

/**
 * @brief Historian destructor.
 * 
 * @param[in] This      Historian instance address.
 */
VOID HistorianDestroy(Historian **This);

#endif //__HISTORIAN__H__
