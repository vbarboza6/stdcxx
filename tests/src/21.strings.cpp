/************************************************************************
 *
 * 21.strings.cpp - definitions of helpers used in clause 21 tests
 *
 * $Id$
 *
 ***************************************************************************
 *
 * Copyright 2006 The Apache Software Foundation or its licensors,
 * as applicable.
 *
 * Copyright 2006 Rogue Wave Software.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 **************************************************************************/

// expand _TEST_EXPORT macros
#define _RWSTD_TEST_SRC

#include <21.strings.h>

#include <cmdopt.h>       // for rw_enabled()
#include <driver.h>       // for rw_info()
#include <environ.h>      // for rw_putenv()
#include <rw_printf.h>    // for rw_asnprintf()

#include <stdarg.h>       // for va_arg, ...
#include <stddef.h>       // for size_t
#include <stdlib.h>       // for free()

/**************************************************************************/

static const char* const
_rw_char_names[] = {
    "char", "wchar_t", "UserChar"
};


static const char* const
_rw_traits_names[] = {
    "char_traits", "UserTraits"
};


static const char* const
_rw_alloc_names[] = {
    "allocator", "UserAllocator"
};


static const char* const
_rw_memfun_names[] = {
    "append", "assign", "erase", "insert", "replace", "operator+="
};

/**************************************************************************/

char StringMembers::
long_string [StringMembers::long_string_len];

static int
_rw_opt_memfun_disabled [StringMembers::sig_last];

static int
_rw_opt_no_char_types [3];

static int
_rw_opt_no_traits_types [2];

static int
_rw_opt_no_alloc_types [2];

static int
_rw_opt_no_exceptions;

static int
_rw_opt_no_exception_safety;

/**************************************************************************/

// coputes integral base-2 logarithm of its argument
static size_t
_rw_ilog2 (size_t n)
{
    unsigned ilog2 = 0;

    while (n >>= 1)
        ++ilog2;

    return ilog2;
}

/**************************************************************************/

// sets the {CLASS}, {FUNC}, {FUNCSIG}, and optionally {FUNCALL}
// environment variables as follows:
// CLASS:   the name of basic_string specialization
// FUNC:    the name of the basic_string member function
// FUNCSIG: the name and signature of a specific overload
//          of the basic_string member function
// FUNCALL: a string describing the call to the basic_string member
//          function with function with function arguments expanded
//          (as specified by the TestCase argument)
void
_rw_setvars (const StringMembers::Function &fun,
             const StringMembers::TestCase *pcase = 0)
{
    char*  buf     = 0;
    size_t bufsize = 0;

    if (0 == pcase) {
        // set the {CLASS}, {FUNC}, and {FUNCSIG} environment variables
        // to the name of the specialization of the template, the name
        // of the member function, and the name of the overload of the
        // member function, respectively, when no test case is given

        if (   StringMembers::DefaultTraits == fun.traits_id_
            && (   StringMembers::Char == fun.char_id_
                || StringMembers::WChar == fun.char_id_)) {
            // format std::string and std::wstring
            rw_asnprintf (&buf, &bufsize, "std::%{?}w%{;}string",
                          StringMembers::WChar == fun.char_id_);
        }
        else {
            // format std::basic_string specializations other than
            // std::string and std::wstring, leaving out the name
            // of the default allocator for brevity
            rw_asnprintf (&buf, &bufsize,
                          "std::basic_string<%s, %s<%1$s>%{?}, %s<%1$s>%{;}>",
                          _rw_char_names [fun.char_id_ - 1],
                          _rw_traits_names [fun.traits_id_ - 1],
                          StringMembers::DefaultAllocator != fun.alloc_id_,
                          _rw_alloc_names [fun.alloc_id_ - 1]);
        }

        // set the {CLASS} variable to the name of the specialization
        // of basic_string
        rw_putenv ("CLASS=");
        rw_fprintf (0, "%{$CLASS:=*}", buf);

        // determine the member function name
        const size_t funinx  = _rw_ilog2 (size_t (fun.which_) >> 5);
        const size_t memfuns =
            sizeof _rw_memfun_names / sizeof *_rw_memfun_names;

        RW_ASSERT (funinx < memfuns);

        const char* const fname = _rw_memfun_names [funinx];

        free (buf);
        buf     = 0;
        bufsize = 0;

        // set the {FUNC} variable to the unqualified name
        // of the member function
        rw_asnprintf (&buf, &bufsize, "%s", fname);

        rw_putenv ("FUNC=");
        rw_fprintf (0, "%{$FUNC:=*}", buf);

        static const char* const signatures[] = {
            "void",
            "const value_type*",
            "const basic_string&",
            "size_type",
            "const value_type*, size_type",
            "const basic_string&, size_type, size_type",
            "size_type, const value_type*, size_type",
            "size_type, const basic_string&, size_type, size_type",
            "size_type, value_type",
            "size_type, const basic_string&",
            "size_type, size_type",
            "size_type, size_type, const value_type*",
            "size_type, size_type, const basic_string&",
            "size_type, size_type, value_type",
            "size_type, size_type, const value_type*, size_type",
            "size_type, size_type, const value_type*, size_type, size_type",
            "size_type, size_type, size_type, value_type",
            "value_type",
            "InputIterator, InputIterator",
            "iterator, value_type",
            "iterator, size_type, value_type",
            "iterator, InputIterator, InputIterator",
            "iterator, iterator, const value_type*",
            "iterator, iterator, const basic_string&",
            "iterator, iterator, const value_type*, size_type",
            "iterator, iterator, size_type, value_type",
            "iterator, iterator, InputIterator, InputIterator",
        };

        const size_t siginx =
            size_t (fun.which_ & ~StringMembers::mem_mask) - 1U;

        RW_ASSERT (siginx < sizeof signatures / sizeof *signatures);

        // append the function signature
        rw_asnprintf (&buf, &bufsize, "%{+} (%s)", signatures [siginx]);

        rw_putenv ("FUNCSIG=");
        rw_fprintf (0, "%{$FUNCSIG:=*}", buf);
        free (buf);

        return;
    }

    // do the function call arguments reference *this?
    const bool self = 0 == pcase->arg;

    // append the ctor argument(s) and the member function name
    rw_asnprintf (&buf, &bufsize,
                  "%{$CLASS} (%{?}%{#*s}%{;}).%{$FUNC} ",
                  pcase->str != 0, int (pcase->str_len), pcase->str);

    // format and append member function arguments
    switch (fun.which_) {
    case StringMembers::append_ptr:
    case StringMembers::assign_ptr:
    case StringMembers::op_plus_eq_ptr:
        rw_asnprintf (&buf, &bufsize,
                      "%{+}(%{?}%{#*s}%{;}%{?}this->c_str ()%{;})",
                      !self, int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::append_str:
    case StringMembers::assign_str:
    case StringMembers::op_plus_eq_str:
        rw_asnprintf (&buf, &bufsize,
                      "%{+}(%{?}string (%{#*s})%{;}%{?}*this%{;})",
                      !self, int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::append_ptr_size:
    case StringMembers::assign_ptr_size:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%{?}%{#*s}%{;}%{?}this->c_str ()%{;}, %zu)",
                      !self, int (pcase->arg_len), pcase->arg,
                      self, pcase->size);
        break;

    case StringMembers::append_str_size_size:
    case StringMembers::assign_str_size_size:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%{?}string (%{#*s})%{;}%{?}*this%{;}, %zu, %zu)",
                      !self, int (pcase->arg_len), pcase->arg, self,
                      pcase->off, pcase->size);
        break;

    case StringMembers::append_size_val:
    case StringMembers::assign_size_val:
        rw_asnprintf (&buf, &bufsize,
                      "%{+} (%zu, %#c)", pcase->size, pcase->val);
        break;

    case StringMembers::append_range:
    case StringMembers::assign_range:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%{?}%{#*s}%{;}%{?}this->%{;}.begin() + %zu, "
                      "%{?}%{#*s}%{;}%{?}this->%{;}.begin() + %zu)",
                      !self, int (pcase->arg_len), pcase->arg,
                      self, pcase->off, !self, int (pcase->arg_len),
                      pcase->arg,
                      self, pcase->off + pcase->size);
        break;

    case StringMembers::insert_size_ptr:
        rw_asnprintf (&buf, &bufsize, 
                      "%{+} (%zu, %{?}%{#*s}%{;}%{?}this->c_str ()%{;})",
                      pcase->off, !self, int (pcase->arg_len), 
                      pcase->arg, self);
        break;

    case StringMembers::insert_size_str:
        rw_asnprintf (&buf, &bufsize,  
                      "%{+} (%zu, %{?}string (%{#*s})%{;}%{?}*this%{;})",
                      pcase->off, !self, int (pcase->arg_len), 
                      pcase->arg, self);
        break;

    case StringMembers::insert_size_ptr_size:
        rw_asnprintf (&buf, &bufsize, "%{+} ("
                      "%zu, %{?}%{#*s}%{;}%{?}this->c_str ()%{;}, %zu)", 
                      pcase->off, !self, int (pcase->arg_len),
                      pcase->arg, self, pcase->size2);
        break;

    case StringMembers::insert_size_str_size_size:
        rw_asnprintf (&buf, &bufsize, "%{+} ("
                      "%zu, %{?}string (%{#*s})%{;}%{?}*this%{;}, %zu, %zu)",
                      pcase->off, !self, int (pcase->arg_len), pcase->arg,
                      self, pcase->off2, pcase->size2);
        break;

    case StringMembers::insert_size_size_val:
        rw_asnprintf (&buf, &bufsize,
                      "%{+} (%zu, %zu, %#c)",
                      pcase->off, pcase->size2, pcase->val);
        break;

    case StringMembers::insert_size_val:
        rw_asnprintf (&buf, &bufsize, 
                      "%{+} (begin + %zu, %zu, %#c)",
                      pcase->off, pcase->size2, pcase->val);
        break;

    case StringMembers::insert_val:
        rw_asnprintf (&buf, &bufsize,
                      "%{+} (begin + %zu, %#c)",
                      pcase->off, pcase->val);
        break;

    case StringMembers::insert_range:
        rw_asnprintf (&buf, &bufsize, "%{+} (begin + %zu, "
                      "%{?}%{#*s}%{;}%{?}*this%{;}.begin + %zu, "
                      "%{?}%{#*s}%{;}%{?}*this%{;}.begin + %zu)", 
                      pcase->off, !self, int (pcase->arg_len), pcase->arg,
                      self, pcase->off2, !self, int (pcase->arg_len),
                      pcase->arg, self, pcase->off2 + pcase->size2);
        break;

    case StringMembers::replace_size_size_ptr:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%zu, %zu, %{?}%{#*s}%{;}%{?}this->c_str ()%{;})",
                      pcase->off, pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::replace_size_size_str:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%zu, %zu, %{?}string (%{#*s})%{;}%{?}*this%{;})",
                      pcase->off, pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::replace_size_size_ptr_size:
        rw_asnprintf (&buf, &bufsize, "%{+}("
                      "%zu, %zu, %{?}%{#*s}%{;}%{?}this->c_str ()%{;}, %zu)", 
                      pcase->off, pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self, pcase->size2);
        break;

    case StringMembers::replace_size_size_str_size_size:
        rw_asnprintf (&buf, &bufsize, "%{+}(%zu, %zu, "
                      "%{?}string (%{#*s})%{;}%{?}*this%{;}, %zu, %zu)",
                      pcase->off, pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self, 
                      pcase->off2, pcase->size2);
        break;

    case StringMembers::replace_size_size_size_val:
        rw_asnprintf (&buf, &bufsize, 
                      "%{+}(%zu, %zu, %zu, %#c)",
                      pcase->off, pcase->size, pcase->size2, pcase->val);
        break;

    case StringMembers::replace_iter_iter_ptr:
        rw_asnprintf (&buf, &bufsize, "%{+}(begin() + %zu, begin() + %zu, "
                      "%{?}%{#*s}%{;}%{?}this->c_str ()%{;})",
                      pcase->off, pcase->off + pcase->size, 
                      !self, int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::replace_iter_iter_str:
        rw_asnprintf (&buf, &bufsize, "%{+}(begin() + %zu, begin() + %zu, " 
                      "%{?}string (%{#*s})%{;}%{?}*this%{;})",
                      pcase->off, pcase->off + pcase->size, 
                      !self, int (pcase->arg_len), pcase->arg, self);
        break;

    case StringMembers::replace_iter_iter_ptr_size:
        rw_asnprintf (&buf, &bufsize, "%{+}(begin() + %zu, begin() + %zu, " 
                      "%{?}%{#*s}%{;}%{?}this->c_str ()%{;}, %zu)", 
                      pcase->off, pcase->off + pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self, pcase->size2);
        break;

    case StringMembers::replace_iter_iter_size_val:
        rw_asnprintf (&buf, &bufsize, 
                      "%{+}(begin() + %zu, begin() + %zu, %zu, %#c)",
                      pcase->off, pcase->off + pcase->size, 
                      pcase->size2, pcase->val);
        break;

    case StringMembers::replace_iter_iter_range:
        rw_asnprintf (&buf, &bufsize, "%{+}(begin() + %zu, begin() + %zu, "
                      "%{?}%{#*s}%{;}%{?}this->%{;}begin() + %zu, "
                      "%{?}%{#*s}%{;}%{?}this->%{;}begin() + %zu)", 
                      pcase->off, pcase->off + pcase->size, !self, 
                      int (pcase->arg_len), pcase->arg, self, pcase->off2,
                      !self, int (pcase->arg_len), pcase->arg, self, 
                      pcase->off2 + pcase->size2);
        break;

    case StringMembers::op_plus_eq_val:
        rw_asnprintf (&buf, &bufsize,
                      "%{+} (%#c)", pcase->val);
        break;

    default:
        RW_ASSERT (!"test logic error: unknown overload");
    }

    rw_putenv ("FUNCALL=");
    rw_fprintf (0, "%{$FUNCALL:=*}", buf);
    free (buf);
}

/**************************************************************************/

static StringMembers::TestFun*
_rw_test_callback;

static const StringMembers::Test*
_rw_string_tests;

static size_t
_rw_string_test_count;


static int
_rw_run_test (int, char*[])
{
#ifdef _RWSTD_NO_EXCEPTIONS

    rw_note (0, 0, 0, "exception tests disabled (macro "
             "_RWSTD_NO_EXCEPTIONS #defined)");

    // disable all exception tests and avoid further notes
    _rw_no_exceptions       = 2;
    _rw_no_exception_safety = 2;

#endif   // _RWSTD_NO_EXCEPTIONS

#ifdef _RWSTD_NO_WCHAR_T

    rw_note (0, 0, 0, "wchar_t tests disabled (macro "
             "_RWSTD_NO_WCHAR_T #defined)");

    // disable wchar_t tests and avoid further notes
    _rw_opt_no_char_types [1] = 2;

#endif   // _RWSTD_NO_WCHAR_T

    if ('\0' == StringMembers::long_string [0]) {
        // initialize long_string
        for (size_t i = 0; i != sizeof StringMembers::long_string - 1; ++i)
            StringMembers::long_string [i] = 'x';
    }

    static const StringMembers::charT char_types[] = {
        StringMembers::Char,
        StringMembers::WChar,
        StringMembers::UChar,
        StringMembers::UnknownChar
    };

    static const StringMembers::Traits traits_types[] = {
        StringMembers::DefaultTraits,
        StringMembers::UserTraits,
        StringMembers::UnknownTraits,
    };

    static const StringMembers::Allocator alloc_types[] = {
        StringMembers::DefaultAllocator,
        // StringMembers::UserAllocator,   // not implemented yet
        StringMembers::UnknownAllocator
    };

    // exercise different charT specializations last
    for (size_t i = 0; char_types [i]; ++i) {

        if (_rw_opt_no_char_types [i]) {
            // issue only the first note
            rw_note (1 < _rw_opt_no_char_types [i]++, __FILE__, __LINE__,
                     "%s tests disabled", _rw_char_names [i]);
            continue;
        }

        // exercise all specializations on Traits before those on charT
        for (size_t j = 0; traits_types [j]; ++j) {

            if (0 == j && StringMembers::UChar == char_types [i]) {
                // std::char_traits can only be instantiated on
                // char and wchar_t, only UserTraits may be used
                // with UserChar
                continue;
            }

            if (_rw_opt_no_traits_types [j]) {
                // issue only the first note
                rw_note (1 < _rw_opt_no_traits_types [j]++, __FILE__, __LINE__,
                         "%s tests disabled", _rw_traits_names [j]);
                continue;
            }

            for (size_t k = 0; alloc_types [k]; ++k) {

                if (_rw_opt_no_alloc_types [k]) {
                    // issue only the first note
                    rw_note (1 < _rw_opt_no_alloc_types [k]++, __FILE__,
                             __LINE__, "%s tests disabled",
                             _rw_alloc_names [k]);
                    continue;
                }

                for (size_t m = 0; m != _rw_string_test_count; ++m) {

                    const StringMembers::Test& test = _rw_string_tests [m];

                    // create an object uniquely identifying the overload
                    // of the member function exercised by the set of test
                    // cases defined to exercise it
                    const StringMembers::Function memfun = {
                        char_types [i],
                        traits_types [j],
                        alloc_types [k],
                        test.which
                    };

                    // set the {CLASS}, {FUNC}, and {FUNCSIG} environment
                    // variable to the name of the basic_string specializaton
                    // and its member function being exercised
                    _rw_setvars (memfun);

                    rw_info (0, 0, 0, "%{$CLASS}::%{$FUNCSIG}");

                    // compute the function overload's 0-based index
                    const size_t siginx =
                        size_t (test.which & ~StringMembers::mem_mask) - 1U;

                    // check if tests of the function overload
                    // have been disabled
                    if (_rw_opt_memfun_disabled [siginx]) {
                        rw_note (0, __FILE__, __LINE__,
                                 "%{$CLASS}::%{$FUNCSIG} tests disabled");
                        continue;
                    }

                    const size_t case_count = test.case_count;

                    // iterate over all test cases for this function overload
                    // invoking the test case handler for each in turn
                    for (size_t n = 0; n != case_count; ++n) {

                        const StringMembers::TestCase& tcase = test.cases [n];

                        // check to see if this is an exception safety
                        // test case and avoid running it when exception
                        // safety has been disabled via a command line
                        // option
                        if (   -1 == tcase.bthrow
                            && _rw_opt_no_exception_safety) {

                            // issue only the first note
                            rw_note (1 < _rw_opt_no_exception_safety++,
                                     __FILE__, __LINE__,
                                     "exception safety tests disabled");
                            continue;
                        }

                        // check to see if this is a test case that
                        // involves the throwing of an exception and
                        // avoid running it when exceptions have been
                        // disabled
                        if (tcase.bthrow && _rw_opt_no_exceptions) {

                            // issue only the first note
                            rw_note (1 < _rw_opt_no_exceptions++,
                                     __FILE__, __LINE__,
                                     "exception tests disabled");
                            continue;
                        }

                        // check to see if the test case is enabled
                        if (rw_enabled (tcase.line)) {

                            // set the {FUNCALL} environment variable
                            // to describe the function call specified
                            // by this test case
                            _rw_setvars (memfun, &tcase);

                            // invoke the test function
                            _rw_test_callback (memfun, tcase);
                        }
                        else
                            rw_note (0, __FILE__, tcase.line,
                                     "test on line %d disabled",
                                     tcase.line);
                    }
                }
            }
        }
    }

    return 0;
}

/**************************************************************************/

int StringMembers::
run_test (int         argc,
          char       *argv [],
          const char *file,
          const char *clause,
          TestFun    *test_callback,
          const Test *tests,
          size_t      test_count)
{
    // set the global variables accessed in _rw_run_test
    _rw_test_callback     = test_callback;
    _rw_string_tests      = tests;
    _rw_string_test_count = test_count;

    return rw_test (argc, argv, file, clause,
                    0,   // comment
                    _rw_run_test,
                    "|-no-char# "
                    "|-no-wchar_t# "
                    "|-no-UserChar# "
                    "|-no-char_traits# "
                    "|-no-UserTraits# "
                    "|-no-allocator# "
                    "|-no-UserAllocator# "

                    "|-no-exceptions# "
                    "|-no-exception-safety# "

                    "|-no-void# "
                    "|-no-ptr# "
                    "|-no-str# "
                    "|-no-size# "
                    "|-no-ptr_size# "
                    "|-no-str_size_size# "
                    "|-no-size_ptr_size# "
                    "|-no-size_str_size_size# "
                    "|-no-size_val# "
                    "|-no-size_str# "
                    "|-no-size_size# "
                    "|-no-size_size_ptr# "
                    "|-no-size_size_str# "
                    "|-no-size_size_val# "
                    "|-no-size_size_ptr_size# "
                    "|-no-size_size_str_size_size# "
                    "|-no-size_size_size_val# "
                    "|-no-val# "
                    "|-no-range# "
                    "|-no-iter_val# "
                    "|-no-iter_size_val# "
                    "|-no-iter_range# "
                    "|-no-iter_iter_ptr# "
                    "|-no-iter_iter_str# "
                    "|-no-iter_iter_ptr_size# "
                    "|-no-iter_iter_size_val# "
                    "|-no-iter_iter_range# ",

                    // handlers controlling specializations of the template
                    _rw_opt_no_char_types + 0,
                    _rw_opt_no_char_types + 1,
                    _rw_opt_no_char_types + 2,
                    _rw_opt_no_traits_types + 0,
                    _rw_opt_no_traits_types + 1,
                    _rw_opt_no_alloc_types + 0,
                    _rw_opt_no_alloc_types + 1,


                    // handlers controlling exceptions
                    &_rw_opt_no_exceptions,
                    &_rw_opt_no_exception_safety,

                    // handlers controlling specific overloads of a function
                    _rw_opt_memfun_disabled + sig_void - 1,
                    _rw_opt_memfun_disabled + sig_ptr - 1,
                    _rw_opt_memfun_disabled + sig_str - 1,
                    _rw_opt_memfun_disabled + sig_size - 1,
                    _rw_opt_memfun_disabled + sig_ptr_size - 1,
                    _rw_opt_memfun_disabled + sig_str_size_size - 1,
                    _rw_opt_memfun_disabled + sig_size_ptr_size - 1,
                    _rw_opt_memfun_disabled + sig_size_str_size_size - 1,
                    _rw_opt_memfun_disabled + sig_size_val - 1,
                    _rw_opt_memfun_disabled + sig_size_str - 1,
                    _rw_opt_memfun_disabled + sig_size_size - 1,
                    _rw_opt_memfun_disabled + sig_size_size_ptr - 1,
                    _rw_opt_memfun_disabled + sig_size_size_str - 1,
                    _rw_opt_memfun_disabled + sig_size_size_val - 1,
                    _rw_opt_memfun_disabled + sig_size_size_ptr_size - 1,
                    _rw_opt_memfun_disabled + sig_size_size_str_size_size - 1,
                    _rw_opt_memfun_disabled + sig_size_size_size_val - 1,
                    _rw_opt_memfun_disabled + sig_val - 1,
                    _rw_opt_memfun_disabled + sig_range - 1,
                    _rw_opt_memfun_disabled + sig_iter_val - 1,
                    _rw_opt_memfun_disabled + sig_iter_size_val - 1,
                    _rw_opt_memfun_disabled + sig_iter_range - 1,
                    _rw_opt_memfun_disabled + sig_iter_iter_ptr - 1,
                    _rw_opt_memfun_disabled + sig_iter_iter_str - 1,
                    _rw_opt_memfun_disabled + sig_iter_iter_ptr_size - 1,
                    _rw_opt_memfun_disabled + sig_iter_iter_size_val - 1,
                    _rw_opt_memfun_disabled + sig_iter_iter_range - 1,

                    // sentinel
                    (void*)0);
}