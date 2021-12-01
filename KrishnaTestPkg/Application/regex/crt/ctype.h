#ifndef _CTYPE_H_
#define _CTYPE_H_

#ifndef isdigit
#define isdigit(x)              ('0'<= x && x <= '9')
#endif // !isdigit

#ifndef isalpha
#define isalpha(x)              (('a'<= x && x <= 'z') || ('A'<= x && x <= 'Z'))
#endif // !isalpha

#ifndef isspace
#define isspace(x)              ((x == ' ') || (x == '\t') || (x == '\r') || (x == '\v') || (x == '\f') || (x == '\n'))
#endif // !isspace

#endif //_CTYPE_H_
