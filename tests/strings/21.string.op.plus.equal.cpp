/***************************************************************************
 *
 * 21.string.plus.equal.cpp - test exercising [lib.string.op+=]
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

#include <string>        // for string
#include <stdexcept>     // for out_of_range, length_error

#include <cstddef>       // for size_t

#include <21.strings.h>   // for StringMembers
#include <alg_test.h>     // for InputIter
#include <driver.h>       // for rw_test()
#include <rw_char.h>      // for rw_expand()
#include <rw_new.h>       // for bad_alloc, replacement operator new

/**************************************************************************/

// for convenience and brevity
#define OpPlusEq(which)   StringMembers::op_plus_eq_ ## which

typedef StringMembers::OverloadId OverloadId;
typedef StringMembers::TestCase   TestCase;
typedef StringMembers::Test       Test;
typedef StringMembers::Function   MemFun;

static const char* const exceptions[] = {
    "unknown exception", "out_of_range", "length_error",
    "bad_alloc", "exception"
};

/**************************************************************************/

// exercises:
// operator += (const value_type*)
static const TestCase
ptr_test_cases [] = {

#undef TEST
#define TEST(str, src, res, bthrow)                            \
    { __LINE__, -1, -1, -1, -1, -1, str, sizeof str - 1, src,  \
      sizeof src - 1, res, sizeof res - 1, bthrow }

    //    +----------------------------------------- controlled sequence
    //    |             +--------------------------- sequence to be appended
    //    |             |             +------------- expected result sequence
    //    |             |             |        +---- exception info
    //    |             |             |        |         0 - no exception
    //    |             |             |        |         1 - length_error
    //    |             |             |        |        -1 - exc. safety
    //    |             |             |        |
    //    |             |             |        +--------------+
    //    V             V             V                       V
    TEST ("ab",         "c",          "abc",                  0),

    TEST ("",           "",           "",                     0),
    TEST ("",           "\0",         "",                     0),
    TEST ("",           "abc",        "abc",                  0),

    TEST ("\0",         "",           "\0",                   0),
    TEST ("\0",         "a",          "\0a",                  0),
    TEST ("\0",         "\0\0",       "\0",                   0),

    TEST ("ab",         "cd",         "abcd",                 0),
    TEST ("bcd",        "a",          "bcda",                 0),
    TEST ("cde",        "ab",         "cdeab",                0),
    TEST ("abc",        "",           "abc",                  0),
    TEST ("ab",         "c\0e",       "abc",                  0),

    TEST ("\0\0ab",     "cdefghij",   "\0\0abcdefghij",       0),
    TEST ("a\0\0b",     "cdefghij",   "a\0\0bcdefghij",       0),
    TEST ("ab\0\0",     "cdefghij",   "ab\0\0cdefghij",       0),
    TEST ("a\0b\0\0c",  "e\0",        "a\0b\0\0ce",           0),
    TEST ("\0ab\0\0c",  "e\0",        "\0ab\0\0ce",           0),
    TEST ("abcdefghij", "abcdefghij", "abcdefghijabcdefghij", 0),

    TEST ("",           "x@4096",     "x@4096",               0),
    TEST ("x@4096",     "",           "x@4096",               0),
    TEST ("x@2048",     "y@2048",     "x@2048y@2048",         0),

    TEST ("",           0,            "",                     0),
    TEST ("abc",        0,            "abcabc",               0),
    TEST ("a\0\0bc",    0,            "a\0\0bca",             0),
    TEST ("\0\0abc",    0,            "\0\0abc",              0),
    TEST ("abc\0\0",    0,            "abc\0\0abc",           0),
    TEST ("x@2048",     0,            "x@4096",               0),

    TEST ("",           "x@4096",     "x@4096",              -1),

    TEST ("last",       "test",       "lasttest",             0)
};

/**************************************************************************/

// exercises:
// operator += (const basic_string&)
static const TestCase
str_test_cases [] = {

#undef TEST
#define TEST(str, src, res, bthrow)                            \
    { __LINE__, -1, -1, -1, -1, -1, str, sizeof str - 1, src,  \
      sizeof src - 1, res, sizeof res - 1, bthrow }

    //    +----------------------------------------- controlled sequence
    //    |             +------------------------- sequence to be appended
    //    |             |             +------------- expected result sequence
    //    |             |             |        +---- exception info
    //    |             |             |        |         0 - no exception
    //    |             |             |        |         1 - length_error
    //    |             |             |        |        -1 - exc. safety
    //    |             |             |        |
    //    |             |             |        +--------------+
    //    V             V             V                       V
    TEST ("ab",         "c",          "abc",                  0),

    TEST ("",           "",           "",                     0),
    TEST ("",           "\0",         "\0",                   0),
    TEST ("",           "abc",        "abc",                  0),

    TEST ("\0",         "",           "\0",                   0),
    TEST ("\0",         "a",          "\0a",                  0),
    TEST ("\0",         "\0\0",       "\0\0\0",               0),

    TEST ("ab",         "cd",         "abcd",                 0),
    TEST ("bcd",        "a",          "bcda",                 0),
    TEST ("cde",        "ab",         "cdeab",                0),
    TEST ("abc",        "",           "abc",                  0),
    TEST ("ab",         "c\0e",       "abc\0e",               0),

    TEST ("\0\0ab",     "cdefghij",   "\0\0abcdefghij",       0),
    TEST ("a\0\0b",     "cdefghij",   "a\0\0bcdefghij",       0),
    TEST ("ab\0\0",     "cdefghij",   "ab\0\0cdefghij",       0),
    TEST ("a\0b\0\0c",  "e\0",        "a\0b\0\0ce\0",         0),
    TEST ("\0ab\0\0c",  "e\0",        "\0ab\0\0ce\0",         0),
    TEST ("ab\0\0c\0",  "\0e",        "ab\0\0c\0\0e",         0),
    TEST ("abcdefghij", "abcdefghij", "abcdefghijabcdefghij", 0),

    TEST ("",           "x@4096",     "x@4096",               0),
    TEST ("x@4096",     "",           "x@4096",               0),
    TEST ("x@2048",     "y@2048",     "x@2048y@2048",         0),

    TEST ("",           0,            "",                     0),
    TEST ("abc",        0,            "abcabc",               0),
    TEST ("a\0\0bc",    0,            "a\0\0bca\0\0bc",       0),
    TEST ("\0\0abc",    0,            "\0\0abc\0\0abc",       0),
    TEST ("abc\0\0",    0,            "abc\0\0abc\0\0",       0),
    TEST ("x@2048",     0,            "x@4096",               0),

    TEST ("",           "x@4096",     "x@4096",              -1),

    TEST ("last",       "test",       "lasttest",             0)
};

/**************************************************************************/

// exercises:
// operator+= (value_type)
static const TestCase
val_test_cases [] = {

#undef TEST
#define TEST(str, val, res, bthrow)                              \
    { __LINE__, -1, -1, -1, -1, val, str, sizeof str - 1, 0, 0,  \
      res, sizeof res - 1, bthrow }

    //    +---------------------------------- controlled sequence
    //    |            +--------------------- character to be appended
    //    |            |   +----------------- expected result sequence
    //    |            |   |           +----- exception info
    //    |            |   |           |        0 - no exception
    //    |            |   |           |        1 - length_error
    //    |            |   |           |       -1 - exc. safety
    //    |            |   |           |
    //    |            |   |           +----+
    //    V            V   V                V
    TEST ("ab",        'c', "abc",          0),

    TEST ("",          'a',  "a",           0),
    TEST ("",          '\0', "\0",          0),

    TEST ("\0",        'a',  "\0a",         0),
    TEST ("\0",        '\0', "\0\0",        0),

    TEST ("cde",       'a',  "cdea",        0),
    TEST ("abc",       '\0', "abc\0",       0),

    TEST ("a\0b\0\0c", '\0', "a\0b\0\0c\0", 0),
    TEST ("\0ab\0\0c", '\0', "\0ab\0\0c\0", 0),
    TEST ("a\0bc\0\0", 'a',  "a\0bc\0\0a",  0),

    TEST ("last",      't', "lastt",        0)
};

/**************************************************************************/

template <class charT, class Traits>
void test_op_plus_eq (charT, Traits*,
                      OverloadId      which,
                      const TestCase &tcase)
{
    typedef std::allocator<charT>                        Allocator;
    typedef std::basic_string <charT, Traits, Allocator> String;
    typedef typename String::iterator                    StringIter;
    typedef typename UserTraits<charT>::MemFun           UTMemFun;

    static const std::size_t BUFSIZE = 256;

    static charT wstr_buf [BUFSIZE];
    static charT warg_buf [BUFSIZE];

    std::size_t str_len = sizeof wstr_buf / sizeof *wstr_buf;
    std::size_t arg_len = sizeof warg_buf / sizeof *warg_buf;

    charT* wstr = rw_expand (wstr_buf, tcase.str, tcase.str_len, &str_len);
    charT* warg = rw_expand (warg_buf, tcase.arg, tcase.arg_len, &arg_len);

    static charT wres_buf [BUFSIZE];
    std::size_t res_len = sizeof wres_buf / sizeof *wres_buf;
    charT* wres = rw_expand (wres_buf, tcase.res, tcase.nres, &res_len);

    // construct the string object to be modified
    // and the (possibly unused) argument string
    /* const */ String  s_str (wstr, str_len);
    const       String  s_arg (warg, arg_len);

    if (wstr != wstr_buf)
        delete[] wstr;

    if (warg != warg_buf)
        delete[] warg;

    wstr = 0;
    warg = 0;

    // save the state of the string object before the call
    // to detect wxception safety violations (changes to
    // the state of the object after an exception)
    const StringState str_state (rw_get_string_state (s_str));

    std::size_t res_off = 0;

    const charT* const arg_ptr = tcase.arg ? s_arg.c_str () : s_str.c_str ();
    const String&      arg_str = tcase.arg ? s_arg : s_str;
    const charT        arg_val = make_char (char (tcase.val), (charT*)0);

    std::size_t total_length_calls = 0;
    std::size_t n_length_calls = 0;
    std::size_t* const rg_calls = rw_get_call_counters ((Traits*)0, (charT*)0);

    if (rg_calls)
        total_length_calls = rg_calls [UTMemFun::length];

    rwt_free_store* const pst = rwt_get_free_store (0);

    // iterate for`throw_after' starting at the next call to operator new,
    // forcing each call to throw an exception, until the function finally
    // succeeds (i.e, no exception is thrown)
    std::size_t throw_after;
    for (throw_after = 0; ; ++throw_after) {

        // (name of) expected and caught exception
        const char* expected = 0;
        const char* caught   = 0;

#ifndef _RWSTD_NO_EXCEPTIONS

        if (1 == tcase.bthrow)
            expected = exceptions [2];      // length_error
        else if (-1 == tcase.bthrow) {
            expected = exceptions [3];      // bad_alloc
            *pst->throw_at_calls_ [0] = pst->new_calls_ [0] + throw_after + 1;
        }

#else   // if defined (_RWSTD_NO_EXCEPTIONS)

    if (tcase.bthrow) {
        if (wres != wres_buf)
            delete[] wres;

        return;
    }

#endif   // _RWSTD_NO_EXCEPTIONS

        try {
            switch (which) {
            case OpPlusEq (ptr): {
                const String& s_res = s_str += arg_ptr;
                res_off = &s_res - &s_str;
                if (rg_calls)
                    n_length_calls = rg_calls [UTMemFun::length];
                break;
            }

            case OpPlusEq (str): {
                const String& s_res = s_str += arg_str;
                res_off = &s_res - &s_str;
                break;
            }

            case OpPlusEq (val): {
                const String& s_res = s_str += arg_val;
                res_off = &s_res - &s_str;
                break;
            }

            default:
                RW_ASSERT ("test logic error: unknown operator += overload");
                return;
            }

            // verify the returned value
            rw_assert (0 == res_off, 0, tcase.line,
                       "line %d. %{$FUNCALL} returned invalid reference, "
                       "offset is %zu", __LINE__, res_off);

            // verfiy that strings length are equal
            rw_assert (res_len == s_str.size (), 0, tcase.line,
                       "line %d. %{$FUNCALL} expected %{#*s} "
                       "with length %zu, got %{/*.*Gs} with length %zu",
                       __LINE__, int (tcase.nres), tcase.res, res_len,
                       int (sizeof (charT)), int (s_str.size ()),
                       s_str.c_str (), s_str.size ());

            if (res_len == s_str.size ()) {
                // if the result length matches the expected length
                // (and only then), also verify that the modified
                // string matches the expected result
                const std::size_t match =
                    rw_match (tcase.res, s_str.c_str(), tcase.nres);

                rw_assert (match == res_len, 0, tcase.line,
                           "line %d. %{$FUNCALL} expected %{#*s}, "
                           "got %{/*.*Gs}, difference at offset %zu",
                           __LINE__, int (tcase.nres), tcase.res,
                           int (sizeof (charT)), int (s_str.size ()),
                           s_str.c_str (), match);
            }

            // verify that Traits::length was used
            if (OpPlusEq (ptr) == which && rg_calls) {
                rw_assert (n_length_calls - total_length_calls > 0, 
                           0, tcase.line, "line %d. %{$FUNCALL} doesn't "
                           "use traits::length()", __LINE__);
            }
        }

#ifndef _RWSTD_NO_EXCEPTIONS

        catch (const std::length_error &ex) {
            caught = exceptions [2];
            rw_assert (caught == expected, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (const std::bad_alloc &ex) {
            caught = exceptions [3];
            rw_assert (-1 == tcase.bthrow, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (const std::exception &ex) {
            caught = exceptions [4];
            rw_assert (0, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (...) {
            caught = exceptions [0];
            rw_assert (0, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught %s",
                       __LINE__, 0 != expected, expected, caught);
        }

#endif   // _RWSTD_NO_EXCEPTIONS

        if (caught) {
            // verify that an exception thrown during allocation
            // didn't cause a change in the state of the object
            str_state.assert_equal (rw_get_string_state (s_str),
                                    __LINE__, tcase.line, caught);

            if (-1 == tcase.bthrow) {
                // allow this call to operator new to succeed and try
                // to make the next one to fail during the next call
                // to the same function again
                continue;
            }
        }
        else if (-1 != tcase.bthrow) {
            rw_assert (caught == expected, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s, caught %s"
                       "%{:}unexpectedly caught %s%{;}",
                       __LINE__, 0 != expected, expected, caught, caught);
        }

        break;
    }

#ifndef _RWSTD_NO_REPLACEABLE_NEW_DELETE

    // verify that if exceptions are enabled and when capacity changes
    // at least one exception is thrown
    rw_assert (   *pst->throw_at_calls_ [0] == std::size_t (-1)
               || throw_after,
               0, tcase.line,
               "line %d: %{$FUNCALL}: failed to throw an expected exception",
               __LINE__);

#endif   // _RWSTD_NO_REPLACEABLE_NEW_DELETE

    *pst->throw_at_calls_ [0] = std::size_t (-1);

    if (wres != wres_buf)
        delete[] wres;
}

/**************************************************************************/

DEFINE_TEST_DISPATCH (test_op_plus_eq);

int main (int argc, char** argv)
{
    static const StringMembers::Test
    tests [] = {

#undef TEST
#define TEST(tag) {                                             \
        StringMembers::op_plus_eq_ ## tag, tag ## _test_cases,  \
        sizeof tag ## _test_cases / sizeof *tag ## _test_cases  \
    }

        TEST (ptr),
        TEST (str),
        TEST (val)
    };

    const std::size_t test_count = sizeof tests / sizeof *tests;

    return StringMembers::run_test (argc, argv, __FILE__,
                                    "lib.string.op+=",
                                    test_op_plus_eq, tests, test_count);
}
