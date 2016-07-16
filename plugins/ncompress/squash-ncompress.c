/* Copyright (c) 2015 The Squash Authors
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
#include <errno.h>

#include <squash/squash.h>

#include "compress.h"

SQUASH_PLUGIN_EXPORT
SquashStatus                 squash_plugin_init_codec       (SquashCodec* codec, SquashCodecImpl* impl);

static size_t
squash_ncompress_get_max_compressed_size (SquashCodec* codec, size_t uncompressed_size) {
  return uncompressed_size + 4 + (uncompressed_size / 2);
}

static SquashStatus
squash_ncompress_status_to_squash_status (enum CompressStatus status) {
  switch (status) {
    case COMPRESS_OK:
      return SQUASH_OK;
    case COMPRESS_WRITE_ERROR:
      return squash_error (SQUASH_BUFFER_FULL);
    case COMPRESS_READ_ERROR:
    case COMPRESS_FAILED:
      return SQUASH_FAILED;
  }
  HEDLEY_UNREACHABLE();
}

static SquashStatus
squash_ncompress_decompress_buffer (SquashCodec* codec,
                                    size_t* decompressed_size,
                                    uint8_t decompressed[HEDLEY_ARRAY_PARAM(*decompressed_size)],
                                    size_t compressed_size,
                                    const uint8_t compressed[HEDLEY_ARRAY_PARAM(compressed_size)],
                                    SquashOptions* options) {
  enum CompressStatus res = decompress (decompressed, decompressed_size, compressed, compressed_size);

  return squash_ncompress_status_to_squash_status (res);
}

static SquashStatus
squash_ncompress_compress_buffer (SquashCodec* codec,
                                  size_t* compressed_size,
                                  uint8_t compressed[HEDLEY_ARRAY_PARAM(*compressed_size)],
                                  size_t uncompressed_size,
                                  const uint8_t uncompressed[HEDLEY_ARRAY_PARAM(uncompressed_size)],
                                  SquashOptions* options) {
  enum CompressStatus res = compress (compressed, compressed_size, uncompressed, uncompressed_size);

  return squash_ncompress_status_to_squash_status (res);
}

SquashStatus
squash_plugin_init_codec (SquashCodec* codec, SquashCodecImpl* impl) {
  const char* name = squash_codec_get_name (codec);

  if (HEDLEY_LIKELY(strcmp ("compress", name) == 0)) {
    impl->get_max_compressed_size = squash_ncompress_get_max_compressed_size;
    impl->decompress_buffer = squash_ncompress_decompress_buffer;
    impl->compress_buffer = squash_ncompress_compress_buffer;
  } else {
    return squash_error (SQUASH_UNABLE_TO_LOAD);
  }

  return SQUASH_OK;
}
