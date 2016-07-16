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
#include <limits.h>

#include <squash/squash.h>

#include "libzling.h"

#define SQUASH_ZLING_DEFAULT_LEVEL 0

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

enum SquashZlingOptIndex {
  SQUASH_ZLING_OPT_LEVEL = 0
};

static SquashOptionInfo squash_zling_options[] = {
  { (char*) "level",
    SQUASH_OPTION_TYPE_RANGE_INT, },
  { NULL, SQUASH_OPTION_TYPE_NONE, }
};

typedef struct _SquashZlingStream SquashZlingStream;

#define SQUASH_ZLING_BUFFER_UNUSED INT_MAX

struct SquashZlingIO: public baidu::zling::Inputter, public baidu::zling::Outputter {
public:
  void* user_data_;
  SquashReadFunc reader_;
  SquashWriteFunc writer_;

  SquashZlingIO(void* user_data, SquashReadFunc reader, SquashWriteFunc writer) :
    user_data_ (user_data),
    reader_ (reader),
    writer_ (writer),
    eof (false),
    last_res (SQUASH_OK),
    buffer (SQUASH_ZLING_BUFFER_UNUSED) { }

  bool eof;
  SquashStatus last_res;
  int buffer;

  size_t GetData(unsigned char* buf, size_t len);
  size_t PutData(unsigned char* buf, size_t len);
  bool   IsEnd();
  bool   IsErr();
};

extern "C" SQUASH_PLUGIN_EXPORT
SquashStatus               squash_plugin_init_codec     (SquashCodec* codec, SquashCodecImpl* impl);
extern "C" SQUASH_PLUGIN_EXPORT
SquashStatus               squash_plugin_init_plugin    (SquashPlugin* plugin);

size_t
SquashZlingIO::GetData (unsigned char* buf, size_t len) {
  size_t bytes_read = 0;

  if (this->IsErr ())
    return 0;

  if (this->buffer != SQUASH_ZLING_BUFFER_UNUSED) {
    assert (this->buffer >= 0x00 && this->buffer <= 0xff);
    buf[0] = (char) this->buffer;
    bytes_read = 1;
    this->buffer = SQUASH_ZLING_BUFFER_UNUSED;
    len--;

    if (len == 0)
      return 1;
  }

  this->last_res = this->reader_ (&len, (uint8_t*) buf + bytes_read, this->user_data_);
  if (this->last_res == SQUASH_END_OF_STREAM)
    this->eof = true;
  bytes_read += len;

  return bytes_read;
}

size_t
SquashZlingIO::PutData(unsigned char* buf, size_t len) {
  const size_t requested = len;

  if (this->IsErr())
    return 0;

  this->last_res = this->writer_ (&len, (const uint8_t*) buf, this->user_data_);

  /* zling will just keep trying to write if we return 0, so pretend
     we wrote what it asked.  Set EOF so at least we don't read any
     more. */
  if (len == 0 && this->last_res == SQUASH_END_OF_STREAM) {
    this->eof = true;
    return requested;
  }

  if (this->last_res != SQUASH_OK)
    return 0;

  return len;
}

bool
SquashZlingIO::IsEnd() {
  if (this->eof)
    return true;
  if (this->buffer != SQUASH_ZLING_BUFFER_UNUSED)
    return false;

  size_t l = 1;
  uint8_t buf;
  this->last_res = this->reader_ (&l, &buf, this->user_data_);
  if (this->last_res == SQUASH_END_OF_STREAM) {
    this->eof = true;
  } else if (this->last_res > 0) {
    assert (!this->eof);
    this->buffer = buf;
  }

  return this->eof;
}

bool
SquashZlingIO::IsErr() {
  return this->last_res < 0;
}

static SquashStatus
squash_zling_splice (SquashCodec* codec,
                     SquashOptions* options,
                     SquashStreamType stream_type,
                     SquashReadFunc read_cb,
                     SquashWriteFunc write_cb,
                     void* user_data) {
  try {
    SquashZlingIO stream(user_data, read_cb, write_cb);
    int zres = 0;

    if (stream_type == SQUASH_STREAM_COMPRESS) {
      zres = baidu::zling::Encode(&stream, &stream, NULL, squash_options_get_int_at (options, codec, SQUASH_ZLING_OPT_LEVEL));
    } else {
      zres = baidu::zling::Decode(&stream, &stream, NULL);
    }

    if (zres == 0)
      return SQUASH_OK;
    else if (HEDLEY_LIKELY(stream.last_res < 0))
      return stream.last_res;
    else
      return squash_error (SQUASH_FAILED);
  } catch (const std::bad_alloc& e) {
    return squash_error (SQUASH_MEMORY);
  } catch (const SquashStatus e) {
    return e;
  } catch (...) {
    return squash_error (SQUASH_FAILED);
  }

  HEDLEY_UNREACHABLE ();
}

static size_t
squash_zling_get_max_compressed_size (SquashCodec* codec, size_t uncompressed_size) {
  return
    uncompressed_size + 288 + (uncompressed_size / 8);
}

extern "C" SquashStatus
squash_plugin_init_plugin (SquashPlugin* plugin) {
  const SquashOptionInfoRangeInt level_range = { 0, 4, 0, false };

  squash_zling_options[SQUASH_ZLING_OPT_LEVEL].default_value.int_value = 0;
  squash_zling_options[SQUASH_ZLING_OPT_LEVEL].info.range_int = level_range;

  return SQUASH_OK;
}

extern "C" SquashStatus
squash_plugin_init_codec (SquashCodec* codec, SquashCodecImpl* impl) {
  const char* name = squash_codec_get_name (codec);

  if (HEDLEY_LIKELY(strcmp ("zling", name) == 0)) {
    impl->options = squash_zling_options;
    impl->splice = squash_zling_splice;
    impl->get_max_compressed_size = squash_zling_get_max_compressed_size;
  } else {
    return squash_error (SQUASH_UNABLE_TO_LOAD);
  }

  return SQUASH_OK;
}
