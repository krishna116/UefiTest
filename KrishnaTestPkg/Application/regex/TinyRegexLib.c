#include "TinyRegexLib.h"
#include "tiny-regex-c/re.h"

PATTERN_T EfiRegexCompile(CONST CHAR8* PatternString)
{
    return re_compile(PatternString);
}

INTN EfiRegexMatchPattern(PATTERN_T Pattern, CONST CHAR8* Text, INTN* MatchLength)
{
    return (INTN)re_matchp(Pattern, Text, (int*)MatchLength);
}

INTN EfiRegexCompileAndMatch(CONST CHAR8* PatternString, CONST CHAR8* Text, INTN* MatchLength)
{
    return (INTN)re_match(PatternString, Text, (int*)MatchLength);
}
