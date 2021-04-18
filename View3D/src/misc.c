/*subfile:  misc.c  ***********************************************************/
/*                                                                            */
/*  Copyright (c) 2014 Jason W. DeGraw                                        */
/*                                                                            */
/*  This file is part of View3D.                                              */
/*                                                                            */
/*  View3D is free software: you can redistribute it and/or modify it         */
/*  under the terms of the GNU General Public License as published by         */
/*  the Free Software Foundation, either version 3 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  View3D is distributed in the hope that it will be useful, but             */
/*  WITHOUT ANY WARRANTY; without even the implied warranty of                */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         */
/*  General Public License for more details.                                  */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with View3D.  If not, see <http://www.gnu.org/licenses/>.           */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*    Utility Functions                                                       */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> /* variable argument list macro definitions */
#include "types.h"  /* define U1, I2, etc.  */
#include "prtyp.h"  /* miscellaneous function prototypes */

#ifndef _WIN32 /* Try to use rusage routines on non-Windows machines */
#define USE_RUSAGE
#endif

#ifdef USE_RUSAGE
#include <sys/time.h>
#include <sys/resource.h>
#endif

extern FILE *_ulog;   /* LOG output file */
extern I1 _string[LINELEN];  /* buffer for ReadXX(); helps debugging */

/***  error  ******************************************************************/

/*  Standard error message routine - _ulog MUST be defined.  */

IX error(IX severity, I1 *file, IX line, ...)
/* severity;  values 0 - 3 defined below
 * file;      file name: __FILE__
 * line;      line number: __LINE__
 * ...;       string variables (up to LINELEN characters total) */
{
  va_list args; /* variable argument list */
  I1* format; /* format string for vprintf */
  static IX count=0;   /* count of severe errors */
  static const I1 *head[4] = { "NOTE", "WARNING", "ERROR", "FATAL" };

  if(severity >= 0) {
    if(severity>3) {
      severity = 3;
    } else if(severity==2) {
      count += 1;
    }
    fprintf(_ulog, "%s %s (%s,%d): ", PROGRAMSTR, head[severity],
            file, line);
    //va_start(args, format);
    va_start(args,line);
    format = va_arg(args, char*);
    //vfprintf(stderr, format, args);
    vfprintf(_ulog, format, args);
    va_end(args);
    if(severity > 2) {
      exit(EXIT_FAILURE);
    }
    //fputs("\n", stderr);
    fputs("\n", _ulog);
  }
  else if(severity < -1) {   /* clear error count */
    count = 0;
  }
  return count;
}  /*  end error  */

#include <limits.h> /* define: SHRT_MAX, SHRT_MIN */

/***  LongCon.c  *************************************************************/

/*  Use ANSI functions to convert a \0 terminated string to a long integer.
 *  Return 1 if string is invalid, 0 if valid.
 *  Global errno will indicate overflow.
 *  Used in place of atol() because of error processing.  */

IX LongCon(I1 *str, I4 *i)
{
  I1 *endptr;
  I4 value;
  IX eflag=0;
#if( !__GNUC__)
  extern IX errno;
  errno = 0;
#endif

  endptr = str;
  value = strtol(str, &endptr, 0);
  if(*endptr) {
    eflag = 1;
  }
#if( !__GNUC__)
  if(errno) {
    eflag = 1;
  }
#endif

  if(eflag) {
    *i = 0L;
  } else {
    *i = value;
  }
  return eflag;
  
}  /* end of LongCon */

/***  IntCon.c  **************************************************************/

/*  Use ANSI functions to convert a \0 terminated string to a short integer.
 *  Return 1 if string is invalid, 0 if valid.
 *  Short integers are in the range -32767 to +32767 (INT_MIN to INT_MAX).
 *  Used in place of atoi() because of error processing.  */

IX IntCon(I1 *str, IX *i)
{
  I4 value;    /* compute result in long integer, then chop */
  IX eflag=0;

  if(LongCon(str, &value)) {
    eflag = 1;
  }
  if(value > SHRT_MAX) {
    eflag = 1;
  }
  if(value < SHRT_MIN) {
    eflag = 1;
  }

  if(eflag) {
    *i = 0;
  } else {
    *i = (IX)value;
  }
  return eflag;
  
}  /* end of IntCon */

extern FILE *_unxt;   /* NXT input file */
extern IX _echo;      /* if true, echo NXT input file */
I1 *_nxtbuf;   /* large buffer for NXT input file */

/***  NxtOpen  ****************************************************************/

/*  Open file_name as UNXT file.  */

void NxtOpen(I1 *file_name, I1 *file, IX line)
/* file;  source code file name: __FILE__
 * line;  line number: __LINE__ */
{
  if(_unxt) {
    error(3, file, line, "_UNXT already open");
  }
  _unxt = fopen(file_name, "r");  /* = NULL if no file */
  if(!_unxt) {
    error(3, file, line, "Could not open file: %s", file_name);
  }
}  /* end NxtOpen */

/***  NxtClose  ***************************************************************/

/*  Close _unxt.  */

void NxtClose(void)
{
  if(_unxt) {
    if(fclose(_unxt)) {
      error(2, __FILE__, __LINE__, "Problem while closing _UNXT");
    }
    _unxt = NULL;
  }

}  /* end NxtClose */

/***  NxtLine  ****************************************************************/

/*  Get characters to end of line (\n --> \0); used by NxtWord().  */

I1 *NxtLine(I1 *str, IX maxlen)
{
  IX c=0;       /* character read from _unxt */
  IX i=0;       /* current position in str */

  while(c!='\n')
  {
    c = getc(_unxt);
    if(c==EOF) {
      return NULL;
    }
    if(_echo) {
      putc(c, _ulog);
    }
    if(maxlen < 1) {
      continue;   // do not fill buffer
    }
    if(c == '\r') {
      continue;    // 2007/10/07 Linux EOL = \n\r
    }
    str[i++] = (I1)c;
    if(i == maxlen)
    {
      str[i-1] = '\0';
      error(3, __FILE__, __LINE__, "Buffer overflow: %s", str);
    }
  }
  if(i) {
    str[i-1] = '\0';
  } else {
    str[0] = '\0';
  }

  return str;

}  /* end NxtLine */

/***  NxtWord  ****************************************************************/

/*  Get the next word from file _unxt.  Return NULL at end-of-file.
 *  Assuming standard word separators (blank, comma, tab),
 *  comment identifiers (! to end-of-line), and
 *  end of data (* or end-of-file). */
/*  Major change October 2007:  ContamX uses NxtWord to read multiple files
 *  which are open simultaneously. The old static variable "newl" may cause
 *  an error. It has been replaced by using ungetc() to note end-of-word (EOW)
 *  which may also be end-of-line (EOL) character.
 *  Initialization with flag = -1 in now invalid - debug checked. */

I1 *NxtWord(I1 *str, IX flag, IX maxlen)
/* str;   buffer where word is stored; return pointer.
 * flag:  0:  get next word from current position in _unxt;
          1:  get 1st word from next line of _unxt;
          2:  get remainder of current line from _unxt (\n --> \0);
          3:  get next data line from _unxt (\n --> \0);
          4:  get next line (even if comment) (\n --> \0).
 * maxlen: length of buffer to test for overflow. */
{
  IX c;         // character read from _unxt
  IX i=0;       // current position in str
  IX done=0;    // true when start of word is found or word is complete

#ifdef DEBUG
  if(!_unxt) {
    error(3, __FILE__, __LINE__, "_UNXT not open");
  }
  if(maxlen < 16) {
    error(3, __FILE__, __LINE__, "Invalid maxlen: %d", maxlen);
  }
#endif
  c = getc(_unxt);
  if(flag > 0) {
    if(c != '\n') {  // last call did not end at EOL; ready to read next char.
                     // would miss first char if reading first line of file.
      if(flag == 2) {
        NxtLine(str, maxlen);  // read to EOL filling buffer
      } else {
        NxtLine(str, 0);  // skip to EOL
      }
      // if flag = 1; continue to read first word on next line
    }
    if(flag > 1) {
      // if flag = 2; return (partial?) line just read
      if(flag > 2) {
        // if flag > 2; return all of next line (must fit in buffer)
        NxtLine(str, maxlen);
        if(flag == 3) { // skip comment lines
          while(str[0] == '!') {
            NxtLine(str, maxlen);
          }
        }
#ifdef DEBUG
        if(flag > 4) {
          error(3, __FILE__, __LINE__, "Invalid flag: %d", flag);
        }
#endif
      }
      ungetc('\n', _unxt);  // restore EOL character for next call
      return str;
    }
  }
  else
  {
#ifdef DEBUG
    if(flag < 0) {
      error(3, __FILE__, __LINE__, "Invalid flag: %d", flag);
    }
#endif
    if(c == ' ' || c == ',' || c == '\n' || c == '\t') {
      ; // skip EOW char saved at last call
    } else {
      ungetc(c, _unxt);  // restore first char of first line
    }
  }

  while(!done) {  // search for start of next word
    c = getc(_unxt);
    if(c==EOF) {
      return NULL;
    }
    if(_echo) {
      putc(c, _ulog);
    }
    switch(c) {
    case ' ':          // skip word separators
    case ',':
    case '\n':
    case '\r':
    case '\t':
    case '\0':
      break;
    case '!':          // begin comment; skip to EOL
      NxtLine(str, 0);
      break;
    case '*':          // end-of-file indicator
      NxtClose();
      return NULL;
    default:           // first character of word found
      str[i++] = (I1)c;
      done = 1;
      break;
    }
  }  // end start-of-word search

  done = 0;
  while(!done) {  // search for end-of-word (EOW)
    c = getc(_unxt);
    if(c==EOF) {
      return NULL;
    }
    if(_echo) {
      putc(c, _ulog);
    }
    switch(c) {
    case '\n':   // EOW characters
    case ' ':
    case ',':
    case '\t':
      str[i] = '\0';
      done = 1;
      break;
    case '\r':   // 2004/01/14 here for Linux: EOL = \n\r
    case '\0':
      break;
    default:     // accumulate word in buffer
      str[i++] = (I1)c;
      if(i == maxlen) { // with overflow test
        str[i-1] = '\0';
        error(3, __FILE__, __LINE__, "Buffer overflow: %s", str);
      }
      break;
    }
  }  // end EOW search
  ungetc(c, _unxt); // save EOW character for next call

  return str;

}  /* end NxtWord */

#include <float.h>  /* define: FLT_MAX, FLT_MIN */

/***  ReadR8  *****************************************************************/

R8 ReadR8(IX flag)
{
  R8 value;

  NxtWord(_string, flag, sizeof(_string));
  if(DblCon(_string, &value)) {
    error(2, __FILE__, __LINE__, "%s is not a (valid) number", _string);
  }
  return value;

}  /* end ReadR8 */

/***  ReadR4  *****************************************************************/

/*  Convert next word from file _unxt to R4 real. */

R4 ReadR4(IX flag)
{
  R8 value;

  NxtWord(_string, flag, sizeof(_string));
  if(DblCon(_string, &value) || value > FLT_MAX || value < -FLT_MAX) {
    error(2, __FILE__, __LINE__, "Bad float value: %s", _string);
  }

  return (R4)value;

}  /* end ReadR4 */

/***  ReadIX  *****************************************************************/

/*  Convert next word from file _unxt to IX integer. */

IX ReadIX(IX flag)
{
  I4 value;

  NxtWord(_string, flag, sizeof(_string));
  if(LongCon(_string, &value) || value > INT_MAX || value < INT_MIN) { // max/min depends on compiler
    error(2, __FILE__, __LINE__, "Bad integer: %s", _string);
  }

  return (IX)value;

}  /* end ReadIX */

/***  DblCon  *****************************************************************/

/*  Use ANSI functions to convert a \0 terminated string to a double value.
 *  Return 1 if string is invalid, 0 if valid.
 *  Global errno will indicate overflow.
 *  Used in place of atof() because of error processing.  */

IX DblCon(I1 *str, R8 *f)
{
  I1 *endptr;
  R8 value;
  IX eflag=0;
#if( !__GNUC__)
  extern IX errno;
  errno = 0;
#endif

  endptr = str;
  value = strtod(str, &endptr);
  if(*endptr != '\0') {
    eflag = 1;
  }
#if( !__GNUC__)
  if(errno) {
    eflag = 1;
  }
#endif

  if(eflag) {
    *f = 0.0;
  } else {
    *f = value;
  }
  return eflag;
  
}  /* end of DblCon */

/***  FltCon  *****************************************************************/

/*  Use ANSI functions to convert a \0 terminated string to a float value.
 *  Return 1 if string is invalid, 0 if valid.
 *  Floats are in the range -3.4e38 to +3.4e38 (FLT_MIN to FLT_MAX).
 *  Used in place of atof() because of error processing.  */

IX FltCon(I1 *str, R4 *f)
{
  R8 value;    /* compute result in high precision, then chop */
  IX eflag=0;

  if(DblCon(str, &value)) {
    eflag = 1;
  }
  if(value > FLT_MAX) {
    eflag = 1;
  }
  if(value < -FLT_MAX) {
    eflag = 1;
  }

  if(eflag) {
    *f = 0.0;
  } else {
    *f = (R4)value;
  }
  return eflag;
  
}  /* end of FltCon */

#include <time.h>   /* prototype: clock;  define CLOCKS_PER_SEC */

/***  CPUtime  ****************************************************************/

/*  Determine elapsed time.  Call once to determine t1;
    call later to get elapsed time. */

R4 CPUtime(R4 t1)
{
  R4 t2;
#ifdef USE_RUSAGE
  struct rusage r2;
  getrusage(RUSAGE_SELF,&r2);
  t2 = (R4)r2.ru_utime.tv_sec + 1.0e-6*r2.ru_utime.tv_usec;
#else
  t2 = (R4) clock() / (R4) CLOCKS_PER_SEC;
#endif
  return (t2-t1);

}  /* end CPUtime */



