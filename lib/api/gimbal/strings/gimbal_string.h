/*! \file
 *  \brief C char and char array manipulation
 *  \ingroup strings
 *  \todo
 *      - gblStrdupa()
 *      - gblStrndupa()
 *      - Unit tests
 *
 *  \author Falco Girgis
 */

#ifndef GIMBAL_STRING_H
#define GIMBAL_STRING_H

#include "../core/gimbal_decls.h"
#include <ctype.h>

GBL_DECLS_BEGIN

GBL_EXPORT int   gblAsciiDigitValue    (char c)                        GBL_NOEXCEPT;
GBL_EXPORT int   gblAsciiHexDigitValue (char c)                        GBL_NOEXCEPT;

GBL_EXPORT int   gblStrCaseCmp         (const char* pStr1,
                                        const char* pStr2)             GBL_NOEXCEPT;
GBL_EXPORT int   gblStrnCaseCmp        (const char* pStr1,
                                        const char* pStr2,
                                        size_t      length)            GBL_NOEXCEPT;

GBL_EXPORT char* gblStrdup             (const char* pString)           GBL_NOEXCEPT;

GBL_EXPORT char* gblStrndup            (const char* pString,
                                        size_t      length)            GBL_NOEXCEPT;

GBL_EXPORT char* gblStrnReverse        (char* pString, size_t  length) GBL_NOEXCEPT;
GBL_EXPORT char* gblStrnUpper          (char* pString)                 GBL_NOEXCEPT;
GBL_EXPORT char* gblStrnLower          (char* pString)                 GBL_NOEXCEPT;
GBL_EXPORT char* gblStpcpy             (char* pString)                 GBL_NOEXCEPT;
// everything string view has
// everything that can be done in-place

GBL_DECLS_END


#endif // GIMBAL_STRING_H
