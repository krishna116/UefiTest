#ifndef _DIGITS_H_
#define _DIGITS_H_

#include <Protocol/GraphicsOutput.h>

#define DIGIT_DECORATE_L (10)
#define DIGIT_DECORATE_R (11)

typedef struct {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buf;
    UINTN* width;
    UINTN* height;
}DigitInfo;

/**
 * It is used to get the digit's image for drawing with GOP.
 * If invalid digit input, then ASSERT.
 * Vaild digit input is: 0~9, DIGIT_DECORATE_L, DIGIT_DECORATE_R
 * 
 * @param[in] i             A digit number.
 * @return DigitInfo*       The digit's image.
 */
DigitInfo* GetDigitInfo(UINTN i);

#endif //_DIGITS_H_