/* Copyright (c) 2015-2016 The Squash Authors
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Evan Nemerson <evan@nemerson.com>
 */
/* IWYU pragma: private, include <squash.h> */

#ifndef SQUASH_FILE_H
#define SQUASH_FILE_H

#if !defined (SQUASH_H_INSIDE) && !defined (SQUASH_COMPILATION)
#error "Only <squash/squash.h> can be included directly."
#endif

#include <squash/squash.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

HEDLEY_BEGIN_C_DECLS

HEDLEY_SENTINEL(0)
HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashFile*  squash_file_open                     (SquashCodec* codec,
                                                              const char* filename,
                                                              const char* mode,
                                                              ...);
HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashFile*  squash_file_open_with_options        (SquashCodec* codec,
                                                              const char* filename,
                                                              const char* mode,
                                                              SquashOptions* options);

HEDLEY_SENTINEL(0)
HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashFile*  squash_file_steal                    (SquashCodec* codec,
                                                              FILE* fp,
                                                              ...);
HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashFile*  squash_file_steal_with_options       (SquashCodec* codec,
                                                              FILE* fp,
                                                              SquashOptions* options);

HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashStatus squash_file_read                     (SquashFile* file,
                                                              size_t* decompressed_size,
                                                              uint8_t decompressed[HEDLEY_ARRAY_PARAM(*decompressed_size)]);
HEDLEY_NON_NULL(1, 3)
SQUASH_API SquashStatus squash_file_write                    (SquashFile* file,
                                                              size_t uncompressed_size,
                                                              const uint8_t uncompressed[HEDLEY_ARRAY_PARAM(uncompressed_size)]);

#if defined(__GNUC__)
__attribute__((__format__ (__printf__, 2, 3)))
#endif
HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashStatus squash_file_printf                   (SquashFile* file,
                                                              const char* format,
                                                              ...);
HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashStatus squash_file_vprintf                  (SquashFile* file,
                                                              const char* format,
                                                              va_list ap);

HEDLEY_NON_NULL(1)
SQUASH_API SquashStatus squash_file_flush                    (SquashFile* file);
HEDLEY_NON_NULL(1)
SQUASH_API SquashStatus squash_file_close                    (SquashFile* file);
SQUASH_API SquashStatus squash_file_free                     (SquashFile* file,
                                                              FILE** fp);
HEDLEY_NON_NULL(1)
SQUASH_API bool         squash_file_eof                      (SquashFile* file);
HEDLEY_NON_NULL(1)
SQUASH_API SquashStatus squash_file_error                    (SquashFile* file);

HEDLEY_NON_NULL(1)
SQUASH_API void         squash_file_lock                     (SquashFile* file);
HEDLEY_NON_NULL(1)
SQUASH_API void         squash_file_unlock                   (SquashFile* file);
HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashStatus squash_file_read_unlocked            (SquashFile* file,
                                                              size_t* decompressed_size,
                                                              uint8_t decompressed[HEDLEY_ARRAY_PARAM(*decompressed_size)]);
HEDLEY_NON_NULL(1, 3)
SQUASH_API SquashStatus squash_file_write_unlocked           (SquashFile* file,
                                                              size_t uncompressed_size,
                                                              const uint8_t uncompressed[HEDLEY_ARRAY_PARAM(uncompressed_size)]);
HEDLEY_NON_NULL(1)
SQUASH_API SquashStatus squash_file_flush_unlocked           (SquashFile* file);

#if defined(SQUASH_ENABLE_WIDE_CHAR_API)
HEDLEY_SENTINEL(0)
HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashFile*  squash_file_wopen                    (SquashCodec* codec,
                                                              const wchar_t* filename,
                                                              const wchar_t* mode,
                                                              ...);
HEDLEY_NON_NULL(1, 2, 3)
SQUASH_API SquashFile*  squash_file_wopen_with_options       (SquashCodec* codec,
                                                              const wchar_t* filename,
                                                              const wchar_t* mode,
                                                              SquashOptions* options);

HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashStatus squash_file_wprintf                  (SquashFile* file,
                                                              const wchar_t* format,
                                                              ...);
HEDLEY_NON_NULL(1, 2)
SQUASH_API SquashStatus squash_file_vwprintf                 (SquashFile* file,
                                                              const wchar_t* format,
                                                              va_list ap);
#endif /* defined(SQUASH_ENABLE_WIDE_CHAR_API) */

HEDLEY_END_C_DECLS

#endif /* SQUASH_FILE_H */
