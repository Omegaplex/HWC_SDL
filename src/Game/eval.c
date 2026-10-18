/* ==========================================================================

  --- Module: eval.c

  --- Purpose: evaluates formulas in strings: e.g. "1+1" -> 2.

  --- Developed with: Pentium PC, Linux 1.2.13, gcc 2.7.0.

  --- Author: Guido Gonzato, Via Monte Ortigara 19/a, 37127 Verona, Italy;
              Tel. +39-45-8345513; e-mail: guido@ibogfs.cineca.it

  --- Last updated: 9 December 1996.

========================================================================== */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
//#include <values.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"
#include "randy.h"

#include "eval.h"

#define  strequ(s1, s2) !strcmp(s1, s2)
#define  nelems(arr) (sizeof(arr) / sizeof(arr[0]))
#define  TOKLEN  256

/* these definitions are used by bsearch()---see forward */

typedef int (*ftpr)(const void *, const void *);

typedef struct {
  int order;
  char s[6];
} FUNC_ITEM;

/* these are the possible tokens (objects): */

typedef enum

{ PLUS, MINUS, MULT, DIV, OPEN_PAR, CLOSED_PAR, COMMA, DIGIT,
  LT, LE, GT, GE, EQ, NEQ,
  BEGIN_FUNC,
  ACOS, ASIN, ATAN, ATAN2, CEIL, COS, COSH, EXP, FABS, FACT, FLOOR, FMOD,
  LDEXP, LOG, LOG10, _PI, POW, RAND, RND, SIN, SINH, SQR, SQRT, TAN, TANH,
  _ERROR, EOL } TOKEN;

/* to add more functions: add the function name above and in func_vector[]
   below, implement the function and add a reference to it in function
   factor(), then add the grammatical check in function check_fact() */

/* ----- */

static char      *s;                /* local string         */
static TOKEN     cur_token;         /* last token read      */
static char      str_token[TOKLEN]; /* last token as string */
static double    val_token;         /* numerical value      */
static int       pc;                /* string counter       */
static ERR_TYPE  exiterr;           /* error occurred       */
static FUNC_ITEM func_vector[] =    /* must be in alphabetical order */

{  1,  "ACOS",
   2,  "ASIN",
   3,  "ATAN",
   4,  "ATAN2",
   5,  "CEIL",
   6,  "COS",
   7,  "COSH",
   8,  "EXP",
   9,  "FABS",
   10, "FACT",
   11, "FLOOR",
   12, "FMOD",
   13, "LDEXP",
   14, "LOG",
   15, "LOG10",
   16, "PI",
   17, "POW",
   18, "RAND",
   19, "RND",
   20, "SIN",
   21, "SINH",
   22, "SQR",
   23, "SQRT",
   24, "TAN",
   25, "TANH" };

/* ----- */

double fact(int n)
{

  int    i;
  double f = 1;

  for (i = 1; i <= n; i++)
    f *= i;

  return(f);

} /* fact() */

/* ----- */

static void init_str(char *eval_str)
/* Initialize s with eval_str. */
{

  pc = 0;
  s = eval_str;

//  while (*eval_str = toupper(*eval_str++))
//    ;
  _strupr(eval_str);

} /* init_str() */

/* ----- */

static void math_err(ERR_TYPE err)
{

  cur_token = _ERROR;
  exiterr = err;

} /* math_err() */

/* ----- */

static int lesser(const FUNC_ITEM *a, const FUNC_ITEM *b)
{

  return( strcmp(a->s, b->s) );

} /* lesser() */

/* ----- */

static FUNC_ITEM *lookup(FUNC_ITEM key)
{

  FUNC_ITEM *itemptr;

  itemptr = (FUNC_ITEM *) bsearch (&key, func_vector, nelems(func_vector),
            sizeof(func_vector[0]), (ftpr) lesser);

  return itemptr;

} /* lookup() */

/* ----- */

static void read_token()
/* Read the next token from the input string. The last token read is
   cur_token, while str_token is the token as a string. For example, reading
   from the string the number 12.3371, cur_token is DIGIT, str_token is
   "12.3371", val_token is 12.3371. */
{

//  TOKEN       tok;   /* for searching */
  int         i;     /* for loop      */
  char        *err;  /* for strtod()  */
  FUNC_ITEM   f,
              *ptr;

  /* skip blanks */

  while (isspace(s[pc]))
    pc++;

  if (isalpha(s[pc])) {   /* it may be a function */

    strcpy(str_token, "");
    i = 0;
    while ( (isalnum(s[pc])) && (i < TOKLEN) )  /* !!! NOT isalpha() */
      str_token[i++] = s[pc++];
    str_token[i] = '\0';

    /* check whether it is a function */

    f.order = 0; /* dummy */
    strcpy(f.s, str_token);

    if ( (ptr = lookup(f)) != NULL )
      cur_token = BEGIN_FUNC + ptr->order;
    else
      math_err(SYNTAX);

    return;

  } /* if isalpha() */

  if (isdigit(s[pc]) || s[pc] == '.') {  /* it's a number */

    strcpy(str_token, "");
    i = 0;
    while ( (isdigit(s[pc]) || s[pc] == '.') && i < TOKLEN )
      str_token[i++] = s[pc++];
    if (s[pc] == 'E') {
      str_token[i++] = s[pc++];
      if (s[pc] == '+' || s[pc] == '-')
        str_token[i++] = s[pc++];
      while (isdigit(s[pc]) && i < TOKLEN )
        str_token[i++] = s[pc++];
    }

/*****
    while ( (isdigit(s[pc]) || s[pc] == '.' || (s[pc] == 'E') ||
            (s[pc] == '+') || (s[pc] == '-')) && (i < TOKLEN) )
      str_token[i++] = s[pc++];
*****/

    str_token[i] = '\0';

    val_token = strtod(str_token, &err);

    if (*err == '\0')
      cur_token = DIGIT;
    else
      math_err(SYNTAX);

    return;

  } /* if isdigit() */

  /* other chars */

  switch (s[pc]) {

    case '+':   cur_token = PLUS;
                break;

    case '-':   cur_token = MINUS;
                break;

    case '*':   cur_token = MULT;
                break;

    case '/':   cur_token = DIV;
                break;

    case '(':   cur_token = OPEN_PAR;
                break;

    case ')':   cur_token = CLOSED_PAR;
                break;

    case ',':   cur_token = COMMA;
                break;

    case '<':   if (s[pc + 1] == '=') {
                  cur_token = LE;
                  pc++;
                }
                else
                  cur_token = LT;
                break;

    case '>':   if (s[pc + 1] == '=') {
                  cur_token = GE;
                  pc++;
                }
                else
                  cur_token = GT;
                break;

    case '=':   if (s[pc + 1] == '=') {
                  cur_token = EQ;
                  pc++;
                }
                else {
                  math_err(SYNTAX);
                  return;
                }
                break;

    case '!':   if (s[pc + 1] == '=') {
                  cur_token = NEQ;
                  pc++;
                }
                else {
                  math_err(SYNTAX);
                  return;
                }
                break;

    case '\0':  cur_token = EOL;
                break;

    default:    math_err(SYNTAX);
                break;

  } /* switch *pc */

  pc++;

} /* read_token() */

/* ----- */

static double expression();
static double multiply();
static double factor();

/* ----- */

static double logical_expression()
{

  double e1, e2;
  TOKEN  opr;

  e1 = expression();

  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == LT || cur_token == LE ||
         cur_token == GT || cur_token == GE ||
         cur_token == EQ || cur_token == NEQ) {

    opr = cur_token;
    read_token();
    e2 = expression();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {

      case  LT:   e1 = (e1 < e2);
                  break;

      case  LE:   e1 = (e1 <= e2);
                  break;

      case  GT:   e1 = (e1 > e2);
                  break;

      case  GE:   e1 = (e1 >= e2);
                  break;

      case  EQ:   e1 = (e1 == e2);
                  break;

      case  NEQ:  e1 = (e1 != e2);
                  break;

    } /* switch */

  } /* while */

  return(e1);

} /* logical_expression() */

/* ----- */

static double expression()
/* This function evaluates expressions. In case of an error, it aborts
   without looking for possible other errors. In order to check ALL the
   possible errors (e.g. syntax), use evalSyntaxOK(). */
{

  double   p, e;    /* for partial results */
  TOKEN    opr;     /* either + or - */

  /* cur_token is already initialised */

  if (cur_token == MINUS) {
    read_token();
    e = -multiply();
  }
  else
    e = multiply();

  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == PLUS || cur_token == MINUS) {

    opr = cur_token;
    read_token();
    p = multiply();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {
      case PLUS:  e += p;
                  break;
      case MINUS: e -= p;
    } /* switch */

  } /* while */

  return(e);

} /* expression() */

/* ----- */

static double multiply()
/* Evaluate * and / */
{

  double  p1, p2, temp;
  TOKEN   opr;    /* either * or / */

  p1 = factor();
  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == MULT || cur_token == DIV) {
    opr = cur_token;
    read_token();
    p2 = factor();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {
      case  MULT: p1 *= p2;
                  break;
      case  DIV:  /* prevent division by zero */
                  temp = p2;
                  if (temp != 0.0)
                    p1 /= temp;
                  else {
                    math_err(DIV_ZERO);
                    return(0.0);
                  }
    } /* switch */

  } /* while */

  return (p1);

} /* multiply() */

/* ----- */

static double factor()
/* Return a factor */
{

  double  f, f2, tmp;
  TOKEN   func;
//  char    *err;

  /* I think I've discovered a bug in Turbo C++ 3.0. It strikes here in
     factor() when calling functions that can cause a RANGE error, like
     ln() or sqrt(). This program compiled with Turbo C++ will crash when
     the user enters something like sqrt(-1) or ln(0), but if compiled by
     gcc the error is trapped correctly. */

  if (cur_token == EOL || cur_token == _ERROR) {
  /* factor() must find something valid */
    cur_token = _ERROR;
    if (cur_token == EOL)
      exiterr = SYNTAX;
    return(0.0);
  }

  switch (cur_token) {

    case DIGIT:     f = val_token;
                    break;

    case OPEN_PAR:  read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);
                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    case _PI:       f = 3.14159265359789;
                    break;

    case ATAN2:
    case FMOD:
    case LDEXP:
    case POW:       func = cur_token;
                    read_token();  /* look for "(" */
                    if (cur_token != OPEN_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);
                    if (cur_token != COMMA) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f2 = expression();
                    if (cur_token == _ERROR)
                      return(0.0);

                    switch(func) {

                      case ATAN2: f = atan2(f, f2);
                                  break;

                      case FMOD:  f = fmod(f, f2);
                                  break;

                      case LDEXP: f = ldexp(f, (int) f2);
                                  break; /* !!! add HUGE_VAL condition */

                      case POW:   if (f == 0.0 && f2 <= 0.0) {
                                    math_err(RANGE);
                                    return(0.0);
                                  }
                                  f2 = modf(f2, &tmp);
                                  if (f < 0.0 && tmp != 0.0) {
                                    math_err(RANGE);
                                    return(0.0);
                                  }
                                  f = pow(f, tmp);
                                  break;

                    } /* switch func */

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    case ACOS:
    case ASIN:
    case ATAN:
    case CEIL:
    case COS:
    case COSH:
    case EXP:
    case FABS:
    case FACT:
    case FLOOR:
    case LOG:
    case LOG10:
    case RAND:
    case RND:
    case SIN:
    case SINH:
    case SQR:
    case SQRT:
    case TAN:
    case TANH:      func = cur_token;
                    read_token();  /* look for "(" */
                    if (cur_token != OPEN_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);

                    switch (func) {

                      case ACOS:   if ( fabs(f) <= 1)
                                     f = acos(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case ASIN:   if ( fabs(f) <= 1)
                                     f = asin(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case ATAN:   f = atan(f);
                                   break;

                      case CEIL:   f = ceil(f);
                                   break;

                      case COS:    f = cos(f);
                                   break;

                      case COSH:   f = cosh(f);
                                   break;

                      case EXP:    f = exp(f);
                                   break;

                      case FABS:   f = fabs(f);
                                   break;

                      case FACT:   if (f > 0.0)
                                     f = fact((int) f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case FLOOR:  f = floor(f);
                                   break;

                      case LOG:    if (f > 0.0)
                                     f = log(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case LOG10:  if (f > 0.0)
                                     f = log10(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case RAND:
                                   f = 1 + (int) ( f * ranRandom(RAN_ETG)/(double)(UDWORD_Max));
                                   break;

                      case RND:
                                   f *= (double) ranRandom(RAN_ETG) / (double)UDWORD_Max;
                                   break;

                      case SIN:    f = sin(f);
                                   break;

                      case SINH:   f = sinh(f);
                                   break;

                      case SQR:    f *= f;
                                   break;

                      case SQRT:   if (f >= 0)
                                     f = sqrt(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case TAN:    if (cos(f) != 0.0)
                                     f = tan(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case TANH:   f = tanh(f);
                                   break;

                           /* ...add more functions here... */

                    } /* switch func */

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    default:        math_err(SYNTAX);
                    return(0.0);

  } /* switch cur_token */

  read_token();
  return(f);

  /* I tried to debug. That strange error strikes HERE - exiting factor() */

} /* factor() */

/* ----- */

double evalEvaluate(char *str, ERR_TYPE *err)
/* This function calculates the expression in str, stores the result in v,
   and returns an exit code. */
{

  double e;

  exiterr = ALL_OK;  /* in the end we should find it unchanged */
  init_str(str);

  /* in case of an error, logical_expression() aborts */

  read_token();
  e = logical_expression();

  if ( (exiterr == ALL_OK) && (cur_token != EOL) )
    *err = SYNTAX;
  else
    *err = exiterr;

  return e;

} /* evalEvaluate() */

/* ----- */

/* Functions for checking the syntax of input string */

static void check_expr();
static void check_mult();
static void check_fact();

/* ----- */

void check_log_expr()
{

  check_expr();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == LT || cur_token == LE ||
         cur_token == GT || cur_token == GE ||
         cur_token == EQ || cur_token == NEQ) {
    read_token();
    check_expr();
    if (exiterr == SYNTAX)
      return;
  }

} /* --- check_log_expr() --- */

/* ----- */

void check_expr()
/* Used to verify the syntax in str */
{

  /* When check_expr starts, cur_token has already been set */

  if (cur_token == MINUS)
    read_token();
  check_mult();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == PLUS || cur_token == MINUS) {
    read_token();
    check_mult();
    if (exiterr == SYNTAX)
      return;
  }

} /* check_expr() */

/* ----- */

static void check_mult()
/* Check * and / */
{

  check_fact();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == MULT || cur_token == DIV) {
    read_token();
    check_fact();
    if (exiterr == SYNTAX)
      return;
  }

} /* check_mult() */

/* ----- */

static void check_fact()
{

  if (cur_token == EOL) {
    exiterr = SYNTAX;
    return;
  }

  switch (cur_token) {

    case DIGIT:
    case _PI:       break; /* do nothing */

    case OPEN_PAR:  read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* OPEN_PAR */

    case ATAN2:
    case FMOD:
    case LDEXP:
    case POW:       read_token();
                    if (cur_token != OPEN_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != COMMA) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* ATAN2 to POW */

    case ACOS:
    case ASIN:
    case ATAN:
    case CEIL:
    case COS:
    case COSH:
    case EXP:
    case FABS:
    case FACT:
    case FLOOR:
    case LOG:
    case LOG10:
    case RAND:
    case RND:
    case SIN:
    case SINH:
    case SQR:
    case SQRT:
    case TAN:
    case TANH:      read_token();
                    if (cur_token != OPEN_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* ACOS to TANH */

    default:        exiterr = SYNTAX;
                    return;
                    break;

  } /* switch (cur_token) */

  read_token();

} /* check_fact() */

/* ----- */

ERR_TYPE evalSyntaxOK(char *str)
/* Return OK if the expression in str is syntaxically correct, NOT_OK
   otherwise. */
{

  exiterr = ALL_OK;  /* if everything goes OK, exiterr will stay ALL_OK */
  init_str(str);
  read_token();
  check_log_expr();

  return(exiterr);

} /* evalSyntaxOK() */

/* ----- */

char *evalErrorStringTable[] =
{
    "OK",
    "Divide by zero",
    "Range",
    "Syntax"
};
//return a string for an error code
char *evalErrorString(ERR_TYPE type)
{
    if (type > SYNTAX || type < 0)
    {
        return("Unknown");
    }
    return(evalErrorStringTable[type]);
}
char *strdel(char *s, int pos, int num)
/* Delete num characters from string s, starting from pos. Equivalent to
   Turbo Pascal's Delete(). */
{

  int i;

  for (i = pos; s[i + num] != '\0'; i++)
    s[i] = s[i + num];

  s[i] = '\0';
  return(s);

} /* strdel() */

/* ----- */
//return a beautified string for an expression
char *evalNum2Str(double n, char *s)
/* From number to string using the shortest possible format */
{

  int i;

  if (n > 1E12)   /* keep exponential format */
    sprintf(s, "%f", n);
  else {
    sprintf(s, "%12.12f", n);
    for (i = strlen(s) - 1; s[i] == '0'; i--)
      strdel(s, i, 1);
    if (s[i] == '.')
      strdel(s, i, 1);
  }

  return(s);

} /* num2str() */

/* --- End of file eval.c --- */

#if 0  // Cat3: evalEvaluate
/*=== evalEvaluate (0x4719C0) ===*/
double __cdecl evalEvaluate(char *str, _DWORD *err)
{
  int v2; // ecx
  int v3; // esi
  double v4; // st7
  double result; // st7
  double v6; // [esp+10h] [ebp-8h]

  unk_A2E038 = 0;
  unk_A2E13C = 0;
  unk_A2E028 = str;
  _strupr(str);
  evalReadToken();
  v6 = evalExpression();
  v2 = unk_A2E140;
  if ( unk_A2E140 == 40 )
  {
LABEL_17:
    result = ZERO_2;
  }
  else
  {
LABEL_2:
    while ( v2 == 8 || v2 == 9 || v2 == 10 || v2 == 11 || v2 == 12 || v2 == 13 )
    {
      v3 = v2;
      evalReadToken();
      v4 = evalExpression();
      v2 = unk_A2E140;
      if ( unk_A2E140 == 40 )
        goto LABEL_17;
      switch ( v3 )
      {
        case 8:
          v6 = (double)(v6 < v4);
          break;
        case 9:
          v6 = (double)(v6 <= v4);
          break;
        case 10:
          v6 = (double)(v6 > v4);
          break;
        case 11:
          v6 = (double)(v6 >= v4);
          break;
        case 12:
          v6 = (double)(v6 == v4);
          break;
        case 13:
          v6 = (double)(v6 != v4);
          break;
        default:
          goto LABEL_2;
      }
    }
    result = v6;
  }
  if ( unk_A2E038 || v2 == 41 )
    *err = unk_A2E038;
  else
    *err = 3;
  return result;
}
#endif

#if 0  // Cat2: evalReadToken live code differs from binary or raw IDA offsets
/*=== evalReadToken (0x471B90) ===*/
char evalReadToken()
{
  int v0; // edx
  int i; // ecx
  int v2; // eax
  int v3; // eax
  int j; // esi
  int v5; // eax
  _DWORD *v6; // eax
  int v7; // eax
  char v8; // al
  int v9; // esi
  int v10; // ecx
  int v11; // ecx
  int k; // esi
  int v13; // eax
  int v14; // ecx
  char v15; // al
  int v16; // eax
  char *v18[2]; // [esp+10h] [ebp-1Ch] BYREF
  int v19[2]; // [esp+18h] [ebp-14h] BYREF
  int v20[3]; // [esp+20h] [ebp-Ch] BYREF

  v0 = unk_A2E028;
  for ( i = unk_A2E13C; ; unk_A2E13C = i )
  {
    if ( (int)cbMultiByte <= 1 )
      v2 = *((_BYTE *)off_8CE5D0 + 2 * *(char *)(i + v0)) & 8;
    else
    {
      v2 = _isctype(*(char *)(i + v0), 8);
      v0 = unk_A2E028;
      i = unk_A2E13C;
    }
    if ( !v2 )
      break;
    ++i;
  }
  if ( (int)cbMultiByte <= 1 )
    v3 = *((_WORD *)off_8CE5D0 + *(char *)(i + v0)) & 0x103;
  else
  {
    v3 = _isctype(*(char *)(i + v0), 259);
    v0 = unk_A2E028;
    i = unk_A2E13C;
  }
  if ( v3 )
  {
    unk_A2E03C[0] = data;
    for ( j = 0; ; ++j )
    {
      if ( (int)cbMultiByte <= 1 )
        v5 = *((_WORD *)off_8CE5D0 + *(char *)(i + v0)) & 0x107;
      else
      {
        v5 = _isctype(*(char *)(i + v0), 263);
        v0 = unk_A2E028;
        i = unk_A2E13C;
      }
      if ( !v5 || j >= 256 )
        break;
      unk_A2E03C[j] = *(_BYTE *)(i + v0);
      unk_A2E13C = ++i;
    }
    unk_A2E03C[j] = 0;
    v18[1] = 0;
    strcpy((char *)v19, unk_A2E03C);
    v20[0] = 0;
    v20[1] = v19[0];
    v20[2] = v19[1];
    v6 = bsearch(v20, &unk_87BC78, 0x19u, 0xCu, evalLesser);
    if ( v6 )
    {
      unk_A2E140 = *v6 + 14;
      return (char)v6;
    }
LABEL_67:
    unk_A2E140 = 40;
    unk_A2E038 = 3;
    return (char)v6;
  }
  if ( (int)cbMultiByte <= 1 )
    v7 = *((_BYTE *)off_8CE5D0 + 2 * *(char *)(i + v0)) & 4;
  else
  {
    v7 = _isctype(*(char *)(i + v0), 4);
    v0 = unk_A2E028;
    i = unk_A2E13C;
  }
  if ( v7 || (v8 = *(_BYTE *)(i + v0), v8 == 46) )
  {
    unk_A2E03C[0] = data;
    for ( k = 0; ; ++k )
    {
      if ( (int)cbMultiByte <= 1 )
        v13 = *((_BYTE *)off_8CE5D0 + 2 * *(char *)(i + v0)) & 4;
      else
      {
        v13 = _isctype(*(char *)(i + v0), 4);
        v0 = unk_A2E028;
        i = unk_A2E13C;
      }
      if ( !v13 && *(_BYTE *)(i + v0) != 46 || k >= 256 )
        break;
      unk_A2E03C[k] = *(_BYTE *)(i + v0);
      unk_A2E13C = ++i;
    }
    if ( *(_BYTE *)(i + v0) == 69 )
    {
      unk_A2E03C[k++] = 69;
      v14 = i + 1;
      unk_A2E13C = v14;
      v15 = *(_BYTE *)(v14 + v0);
      if ( v15 != 43 && v15 != 45 )
        goto LABEL_59;
      while ( 1 )
      {
        unk_A2E03C[k++] = v15;
        unk_A2E13C = ++v14;
LABEL_59:
        if ( (int)cbMultiByte <= 1 )
          v16 = *((_BYTE *)off_8CE5D0 + 2 * *(char *)(v14 + v0)) & 4;
        else
        {
          v16 = _isctype(*(char *)(v14 + v0), 4);
          v0 = unk_A2E028;
          v14 = unk_A2E13C;
        }
        if ( !v16 || k >= 256 )
          break;
        v15 = *(_BYTE *)(v14 + v0);
      }
    }
    unk_A2E03C[k] = 0;
    unk_A2E030 = strtod(unk_A2E03C, v18);
    LOBYTE(v6) = *v18[0];
    if ( !*v18[0] )
    {
      unk_A2E140 = 7;
      return (char)v6;
    }
    goto LABEL_67;
  }
  v9 = v8;
  LOBYTE(v6) = 0;
  switch ( v9 )
  {
    case 0:
      unk_A2E140 = 41;
      unk_A2E13C = i + 1;
      break;
    case 33:
      if ( *(_BYTE *)(i + v0 + 1) != 61 )
        goto LABEL_67;
      unk_A2E140 = 13;
      unk_A2E13C = i + 2;
      break;
    case 40:
      unk_A2E140 = 4;
      unk_A2E13C = i + 1;
      break;
    case 41:
      unk_A2E140 = 5;
      unk_A2E13C = i + 1;
      break;
    case 42:
      unk_A2E140 = 2;
      unk_A2E13C = i + 1;
      break;
    case 43:
      unk_A2E140 = 0;
      unk_A2E13C = i + 1;
      break;
    case 44:
      unk_A2E140 = 6;
      unk_A2E13C = i + 1;
      break;
    case 45:
      unk_A2E140 = 1;
      unk_A2E13C = i + 1;
      break;
    case 47:
      unk_A2E140 = 3;
      unk_A2E13C = i + 1;
      break;
    case 60:
      if ( *(_BYTE *)(i + v0 + 1) == 61 )
      {
        v10 = i + 2;
        unk_A2E140 = 9;
      }
      else
      {
        v10 = i + 1;
        unk_A2E140 = 8;
      }
      unk_A2E13C = v10;
      break;
    case 61:
      if ( *(_BYTE *)(i + v0 + 1) != 61 )
        goto LABEL_67;
      unk_A2E140 = 12;
      unk_A2E13C = i + 2;
      break;
    case 62:
      if ( *(_BYTE *)(i + v0 + 1) == 61 )
      {
        v11 = i + 2;
        unk_A2E140 = 11;
      }
      else
      {
        v11 = i + 1;
        unk_A2E140 = 10;
      }
      unk_A2E13C = v11;
      break;
    default:
      unk_A2E140 = 40;
      unk_A2E038 = 3;
      unk_A2E13C = i + 1;
      break;
  }
  return (char)v6;
}
#endif


#if 0  // Cat3: evalExpression
/*=== evalExpression (0x472110) ===*/
double evalExpression()
{
  double v0; // st7
  unsigned int v1; // eax
  unsigned int v3; // esi
  double v4; // st7
  double v5; // [esp+8h] [ebp-8h]

  if ( unk_A2E140 == 1 )
  {
    evalReadToken();
    v0 = -evalMultiply();
  }
  else
  {
    v0 = evalMultiply();
  }
  v1 = unk_A2E140;
  v5 = v0;
  if ( unk_A2E140 == 40 )
    return ZERO_2;
  while ( 1 )
  {
    if ( v1 > 1 )
      return v5;
    v3 = v1;
    evalReadToken();
    v4 = evalMultiply();
    v1 = unk_A2E140;
    if ( unk_A2E140 == 40 )
      break;
    if ( v3 )
      v5 = v5 - v4;
    else
      v5 = v4 + v5;
  }
  return ZERO_2;
}
#endif

#if 0  // Cat3: evalMultiply
/*=== evalMultiply (0x4721B0) ===*/
double evalMultiply()
{
  int v0; // ecx
  double result; // st7
  int v2; // esi
  double v3; // st7
  double v4; // [esp+8h] [ebp-8h]

  v4 = evalFactor();
  v0 = unk_A2E140;
  if ( unk_A2E140 == 40 )
    return ZERO_2;
  while ( 1 )
  {
    while ( 1 )
    {
      if ( v0 != 2 && v0 != 3 )
        return v4;
      v2 = v0;
      evalReadToken();
      v3 = evalFactor();
      v0 = unk_A2E140;
      if ( unk_A2E140 == 40 )
        return ZERO_2;
      if ( v2 != 2 )
        break;
      v4 = v3 * v4;
    }
    if ( v3 == ZERO_2 )
      break;
    v4 = v4 / v3;
  }
  unk_A2E140 = 40;
  result = ZERO_2;
  unk_A2E038 = 1;
  return result;
}
#endif

#if 0  // Cat3: evalNum2Str
/*=== evalNum2Str (0x4727B0) ===*/
char *__cdecl evalNum2Str(double a1, char *a2)
{
  unsigned int v3; // ecx
  char v4; // al
  unsigned int v5; // ecx
  char v6; // dl
  unsigned int i; // eax
  unsigned int v8; // eax
  char v9; // cl

  if ( a1 <= dbl_72A850 )
  {
    sprintf(a2, "%12.12f", a1);
    v3 = strlen(a2) + 1;
    v4 = a2[v3 - 2];
    v5 = v3 - 2;
    if ( v4 == 48 )
    {
      do
      {
        v6 = a2[v5 + 1];
        for ( i = v5; v6; ++i )
        {
          a2[i] = v6;
          v6 = a2[i + 2];
        }
        --v5;
        a2[i] = 0;
      }
      while ( a2[v5] == 48 );
    }
    if ( a2[v5] == 46 )
    {
      v8 = v5;
      if ( a2[v5 + 1] )
      {
        v9 = a2[v5 + 1];
        do
        {
          a2[v8] = v9;
          v9 = a2[v8++ + 2];
        }
        while ( v9 );
      }
      a2[v8] = 0;
    }
    return a2;
  }
  else
  {
    sprintf(a2, "%f", a1);
    return a2;
  }
}
#endif
