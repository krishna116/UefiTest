#ifndef _REGEX_H_
#define _REGEX_H_

#include<Uefi.h>

typedef struct regex_t* PATTERN_T;

/**
 * @brief Compile pattern string to pattern.
 * 
 * @param[in] PatternString     Pattern string to compile.
 * 
 * @return NOT NULL             Operation success.
 * @return NULL                 Operation fail.
 */
PATTERN_T EfiRegexCompile(CONST CHAR8* PatternString);

/**
 * @brief Find matches of the compiled pattern inside text
 * 
 * @param[in] Pattern           Pattern to match.
 * @param[in] Text              Text used to search the pattern.
 * @param[out] MatchLength      String length of matched pattern.
 * 
 * @return INTN >= 0            Operation success and it is the matched position in the text.
 * @return INTN < 0             Operation fail.
 */
INTN EfiRegexMatchPattern(PATTERN_T Pattern, CONST CHAR8* Text, INTN* MatchLength);

/**
 * @brief Find matches of the txt pattern inside text (will compile automatically first).
 * 
 * @param[in] PatternString     Pattern string to compile.
 * @param[in] Text              Text used to search the pattern.
 * @param[out] MatchLength      String length of matched pattern.
 * 
 * @return INTN >= 0            Operation success and it is the matched position in the text.
 * @return INTN < 0             Operation fail.
 */
INTN EfiRegexCompileAndMatch(CONST CHAR8* PatternString, CONST CHAR8* Text, INTN* MatchLength);

#endif //_REGEX_H_
