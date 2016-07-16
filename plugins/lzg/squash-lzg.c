/* Copyright (c) 2013-2016 The Squash Authors
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <squash/squash.h>

#include "liblzg/src/include/lzg.h"

enum SquashLzgOptIndex {
  SQUASH_LZG_OPT_LEVEL = 0,
  SQUASH_LZG_OPT_FAST
};

static SquashOptionInfo squash_lzg_options[] = {
  { "level",
    SQUASH_OPTION_TYPE_RANGE_INT,
    .info.range_int = {
      .min = 1,
      .max = 9 },
    .default_value.int_value = 5 },
  { "fast",
    SQUASH_OPTION_TYPE_BOOL,
    .default_value.bool_value = true },
  { NULL, SQUASH_OPTION_TYPE_NONE, }
};

const lzg_encoder_config_t squash_lzg_default_config = {
  LZG_LEVEL_DEFAULT,
  LZG_TRUE,
  NULL,
  NULL
};

SQUASH_PLUGIN_EXPORT
SquashStatus             squash_plugin_init_codec   (SquashCodec* codec, SquashCodecImpl* impl);

static size_t
squash_lzg_get_max_compressed_size (SquashCodec* codec, size_t uncompressed_size) {
#if UINT32_MAX < SIZE_MAX
  if (HEDLEY_UNLIKELY(UINT32_MAX < uncompressed_size))
    return (squash_error (SQUASH_RANGE), 0);
#endif

  const lzg_uint32_t res = LZG_MaxEncodedSize ((lzg_uint32_t) uncompressed_size);

#if SIZE_MAX < UINT32_MAX
  if (HEDLEY_UNLIKELY(SIZE_MAX < res))
    return (squash_error (SQUASH_RANGE), 0);
#endif

  return (size_t) res;
}

static size_t
squash_lzg_get_uncompressed_size (SquashCodec* codec,
                                  size_t compressed_size,
                                  const uint8_t compressed[HEDLEY_ARRAY_PARAM(compressed_size)]) {
#if UINT32_MAX < SIZE_MAX
  if (HEDLEY_UNLIKELY(UINT32_MAX < compressed_size))
    return (squash_error (SQUASH_RANGE), 0);
#endif

  const lzg_uint32_t res = LZG_DecodedSize ((const unsigned char*) compressed, (lzg_uint32_t) compressed_size);

#if SIZE_MAX < UINT32_MAX
  if (HEDLEY_UNLIKELY(SIZE_MAX < res))
    return (squash_error (SQUASH_RANGE), 0);
#endif

  return (size_t) res;
}

static SquashStatus
squash_lzg_compress_buffer (SquashCodec* codec,
                            size_t* compressed_size,
                            uint8_t compressed[HEDLEY_ARRAY_PARAM(*compressed_size)],
                            size_t uncompressed_size,
                            const uint8_t uncompressed[HEDLEY_ARRAY_PARAM(uncompressed_size)],
                            SquashOptions* options) {
  lzg_encoder_config_t cfg = {
    squash_options_get_int_at (options, codec, SQUASH_LZG_OPT_LEVEL),
    squash_options_get_bool_at (options, codec, SQUASH_LZG_OPT_FAST),
    NULL,
    NULL
  };

#if UINT32_MAX < SIZE_MAX
  if (HEDLEY_UNLIKELY(UINT32_MAX < uncompressed_size) ||
      HEDLEY_UNLIKELY(UINT32_MAX < *compressed_size))
    return squash_error (SQUASH_RANGE);
#endif

  uint8_t* workmem = squash_calloc (LZG_WorkMemSize (&cfg), 1);
  if (HEDLEY_UNLIKELY(workmem == NULL))
    return squash_error (SQUASH_MEMORY);
  lzg_uint32_t res = LZG_EncodeFull ((const unsigned char*) uncompressed, (lzg_uint32_t) uncompressed_size,
                                     (unsigned char*) compressed, (lzg_uint32_t) *compressed_size,
                                     &cfg, workmem);
  squash_free (workmem);

  if (res == 0) {
    return squash_error (SQUASH_FAILED);
  } else {
#if SIZE_MAX < UINT32_MAX
    if (HEDLEY_UNLIKELY(SIZE_MAX < res))
      return squash_error (SQUASH_RANGE);
#endif
    *compressed_size = (size_t) res;
    return SQUASH_OK;
  }
}

static SquashStatus
squash_lzg_decompress_buffer (SquashCodec* codec,
                              size_t* decompressed_size,
                              uint8_t decompressed[HEDLEY_ARRAY_PARAM(*decompressed_size)],
                              size_t compressed_size,
                              const uint8_t compressed[HEDLEY_ARRAY_PARAM(compressed_size)],
                              SquashOptions* options) {
  lzg_uint32_t res;

#if UINT32_MAX < SIZE_MAX
  if (HEDLEY_UNLIKELY(UINT32_MAX < compressed_size) ||
      HEDLEY_UNLIKELY(UINT32_MAX < *decompressed_size))
    return squash_error (SQUASH_RANGE);
#endif

  if (LZG_DecodedSize ((const unsigned char*) compressed, (lzg_uint32_t) compressed_size) > *decompressed_size)
    return squash_error (SQUASH_BUFFER_FULL);

  res = LZG_Decode ((const unsigned char*) compressed, (lzg_uint32_t) compressed_size,
                    (unsigned char*) decompressed, (lzg_uint32_t) *decompressed_size);

  if (HEDLEY_UNLIKELY(res == 0)) {
    return squash_error (SQUASH_FAILED);
  } else {
#if SIZE_MAX < UINT32_MAX
    if (HEDLEY_UNLIKELY(SIZE_MAX < res))
      return squash_error (SQUASH_RANGE);
#endif
    *decompressed_size = (size_t) res;
    return SQUASH_OK;
  }
}

SquashStatus
squash_plugin_init_codec (SquashCodec* codec, SquashCodecImpl* impl) {
  const char* name = squash_codec_get_name (codec);

  if (HEDLEY_LIKELY(strcmp ("lzg", name) == 0)) {
    impl->options = squash_lzg_options;
    impl->get_uncompressed_size = squash_lzg_get_uncompressed_size;
    impl->get_max_compressed_size = squash_lzg_get_max_compressed_size;
    impl->decompress_buffer = squash_lzg_decompress_buffer;
    impl->compress_buffer = squash_lzg_compress_buffer;
  } else {
    return squash_error (SQUASH_UNABLE_TO_LOAD);
  }

  return SQUASH_OK;
}
