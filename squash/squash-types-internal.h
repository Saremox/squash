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
/* IWYU pragma: private, include "squash-internal.h" */

#ifndef SQUASH_TYPES_INTERNAL_H
#define SQUASH_TYPES_INTERNAL_H

#if !defined (SQUASH_COMPILATION)
#error "This is internal API; you cannot use it."
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

HEDLEY_BEGIN_C_DECLS

typedef SQUASH_TREE_HEAD(SquashPluginTree_, SquashPlugin_) SquashPluginTree;
typedef SQUASH_TREE_HEAD(SquashCodecTree_, SquashCodec_) SquashCodecTree;
typedef SQUASH_TREE_HEAD(SquashCodecRefTree_, SquashCodecRef_) SquashCodecRefTree;

struct SquashContext_ {
  SquashPluginTree plugins;
  SquashCodecRefTree codecs;
  SquashCodecRefTree extensions;
};

struct SquashPlugin_ {
  SquashContext* context;

  char* name;
  char* directory;
  SquashLicense* license;

#if !defined(_WIN32)
  void* plugin;
#else
  HMODULE plugin;
#endif

  SquashCodecTree codecs;

  SQUASH_TREE_ENTRY(SquashPlugin_) tree;
};

struct SquashCodec_ {
  SquashPlugin* plugin;

  char* name;
  int priority;
  char* extension;

  bool initialized;
  SquashCodecImpl impl;

  SQUASH_TREE_ENTRY(SquashCodec_) tree;
};

typedef struct SquashCodecRef_ {
  SquashCodec* codec;

  SQUASH_TREE_ENTRY(SquashCodecRef_) tree;
} SquashCodecRef;

typedef struct SquashBuffer_ {
  uint8_t* data;
  size_t size;
  size_t allocated;
} SquashBuffer;

HEDLEY_END_C_DECLS

#endif /* SQUASH_TYPES_INTERNAL_H */
