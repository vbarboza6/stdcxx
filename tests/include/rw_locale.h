/************************************************************************
 *
 * rw_locale.h - declarations of locale testsuite helpers
 *
 * $Id$
 *
 ************************************************************************
 *
 * Licensed to the Apache Software  Foundation (ASF) under one or more
 * contributor  license agreements.  See  the NOTICE  file distributed
 * with  this  work  for  additional information  regarding  copyright
 * ownership.   The ASF  licenses this  file to  you under  the Apache
 * License, Version  2.0 (the  "License"); you may  not use  this file
 * except in  compliance with the License.   You may obtain  a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the  License is distributed on an  "AS IS" BASIS,
 * WITHOUT  WARRANTIES OR CONDITIONS  OF ANY  KIND, either  express or
 * implied.   See  the License  for  the  specific language  governing
 * permissions and limitations under the License.
 *
 * Copyright 2001-2006 Rogue Wave Software.
 * 
 **************************************************************************/

#ifndef RW_LOCALE_H_INCLUDED
#define RW_LOCALE_H_INCLUDED


#include <testdefs.h>


#define _UNUSED_CAT 69


_TEST_EXPORT char*
rw_locales (int = _UNUSED_CAT, const char* = 0);


// invokes the locale utility with the arguments specified by 'arg',
// redirecting its output to the file named by 'fname' if non-null
// returns the exit status of the utility
_TEST_EXPORT int
rw_locale (const char* /* args */, const char* /* fname */);


// creates a temporary directory and defines the RWSTD_LOCALE_ROOT
// environment variable to the name of the directory; if the environment
// variable RW_PUTENV is defined, defines any additional environment
// variable specified by it (by calling rw_putenv(0))
// returns the absolute pathname of the directory
#define LOCALE_ROOT_ENVAR   "RWSTD_LOCALE_ROOT"

_TEST_EXPORT const char*
rw_set_locale_root ();


// invokes localedef to create a locale database named by the last argument,
// if non-0, or in a directory specified by the RWSTD_LOCALE_ROOT environment
// variable otherwise, if it is defined, otherwise in the current working
// directory
// returns the name of the locale; successive calls to the function may
// change the contents of the character array pointed to by the pointer
// returned from prior calls
_TEST_EXPORT const char*
rw_localedef (const char*, const char*, const char*, const char*);


// an array of multibyte characters 1 to MB_LEN_MAX bytes in length
typedef char
rw_mbchar_array_t [_RWSTD_MB_LEN_MAX][_RWSTD_MB_LEN_MAX];

// fills consecutive elements of the `mb_chars' array with multibyte
// characters between 1 and MB_CUR_MAX bytes long for the currently
// set locale
// returns the number of elements populated (normally, MB_CUR_MAX)
_TEST_EXPORT _RWSTD_SIZE_T
rw_get_mb_chars (rw_mbchar_array_t /* mb_chars */);


// finds the multibyte locale with the largest MB_CUR_MAX value and
// fills consecutive elements of the `mb_chars' array with multibyte
// characters between 1 and MB_CUR_MAX bytes long for such a locale
_TEST_EXPORT const char*
rw_find_mb_locale (_RWSTD_SIZE_T* /* mb_cur_max */,
                   rw_mbchar_array_t /* mb_chars */);

// invokes localedef with charmap and locale def to create a locale database 
// in a directory specified by the RWSTD_LOCALE_ROOT environment
// variable, if it is defined, otherwise in the current working
// directory
// returns the name of the locale
_TEST_EXPORT const char*
rw_create_locale (const char* /* charmap */, const char* /* locale */);


// NUL-separated list of locale names specified by the --locales
// command line option (set by the rw_opt_setlocales handler)
_TEST_EXPORT extern const char* const&
rw_opt_locales;


// callback function invoked in response to the --locales=<locale-list>
// command line option to set the rw_opt_locales global variable to the
// NUL-separated list of locale names to exercise
_TEST_EXPORT int
rw_opt_setlocales (int, char*[]);

#endif   // RW_LOCALE_H_INCLUDED
