/* Copyright (c) 2013 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Huffman_coding_(C_Plus_Plus)?action=history&offset=20090129100015

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Huffman_coding_(C_Plus_Plus)?oldid=16057
*/

#ifndef HUFFMAN_H_INC
#define HUFFMAN_H_INC
#define BINARY_CODES

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

typedef uint32_t DataType;
typedef int Frequency;
struct Hufftree;
typedef struct Hufftree Hufftree;
struct PutBitContext;
typedef struct PutBitContext PutBitContext;
struct GetBitContext;
typedef struct GetBitContext GetBitContext;

Hufftree* new_Hufftree(Frequency *frequency, int size);
Hufftree*  new_Hufftree2(GetBitContext *s, int size);
size_t encodetree_Hufmantree( Hufftree*h, PutBitContext *s);
DataType decode_Hufftree(Hufftree*h, GetBitContext *s);
void encode_Hufftree(Hufftree*h, DataType v, PutBitContext *s);

void delete_Hufftree(Hufftree* hufftree);
void delete_Hufftreep(Hufftree** hufftree);

struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
};

static inline void init_put_bits(PutBitContext *s, uint8_t *buffer, size_t buffer_size)
{
    if (buffer_size < 0) {
        buffer_size = 0;
        buffer      = NULL;
    }

    s->buf          = buffer;
    s->buf_end      = s->buf + buffer_size;
    s->buf_ptr      = s->buf;
    s->bit_left     = 32;
    s->bit_buf      = 0;
}

static inline void rebase_put_bits(PutBitContext *s, uint8_t *buffer,
                                   size_t buffer_size)
{
    s->buf_end = buffer + buffer_size;
    s->buf_ptr = buffer + (s->buf_ptr - s->buf);
    s->buf     = buffer;
}

static void check_grow( PutBitContext* pb ) {
      if (pb->buf == NULL) {
          init_put_bits(pb, malloc(256+32), 256 );
      }
      else
      if ( pb->buf_end-pb->buf_ptr  < 256 ) {
         size_t sz = (pb->buf_end-pb->buf)*2;
         if (sz < 256) sz = 256;
         rebase_put_bits(pb,realloc(pb->buf, sz +  32), sz);
      }
}


static inline size_t put_bytes_count(PutBitContext *s)
{
   check_grow(s);
    if (s->bit_left < 32)
        s->bit_buf <<= s->bit_left;
    while (s->bit_left < 32) {
        *s->buf_ptr++ = s->bit_buf >> (32 - 8);
        s->bit_buf  <<= 8;
        s->bit_left  += 8;
    }
    s->bit_left = 32;
    s->bit_buf  = 0;
    return s->buf_ptr - s->buf + (size_t)((32 - s->bit_left + (7)) >> 3);
}



static inline void put_bits(PutBitContext *s, int n, uint32_t value)
{
    check_grow(s);
    uint32_t bit_buf;
    int bit_left;
    bit_left = s->bit_left;
    bit_buf  = s->bit_buf;

    if (n < bit_left) {
        bit_buf     = (bit_buf << n) | value;
        bit_left   -= n;
    } else {
        bit_buf   <<= bit_left;
        bit_buf    |= value >> (n - bit_left);
        *((uint32_t*) s->buf_ptr) = __builtin_bswap32(bit_buf);
        s->buf_ptr += 4;
        bit_left   += 32 - n;
        bit_buf     = value;
    }
    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;
}

/**
 * Write exactly 32 bits into a bitstream.
 */
static void put_bits32(PutBitContext *s, uint32_t value)
{
   check_grow(s);
    uint32_t bit_buf;
    int bit_left;

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    bit_buf     = (uint64_t)bit_buf << bit_left;
    bit_buf    |= (uint32_t)value >> (32 - bit_left);
    *((uint32_t*) s->buf_ptr) = __builtin_bswap32(bit_buf);
   s->buf_ptr += 4;

    bit_buf     = value;
    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;
}



struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
    size_t index;
    size_t size_in_bits;
};


static void init_get_bits(GetBitContext *s, const uint8_t *buffer, size_t buffer_size)
{
    s->buffer             = buffer;
    s->size_in_bits       = buffer_size << 3;
    s->buffer_end         = buffer + buffer_size;
    s->index              = 0;
}


static inline unsigned int get_bits1(GetBitContext *s)
{
    size_t index = s->index;
    if (index >= s->size_in_bits) {
      //printf("overflow!");
       return -1;
    }
    uint8_t result     = s->buffer[index >> 3];
    result <<= index & 7;
    result >>= 8 - 1;
    index++;
    s->index = index;
    return result;
}

/**
 * Read 1-25 bits.
 */
static inline unsigned int get_bits(GetBitContext *s, int n)
{
   assert(n>0 && n<=25);
    union unaligned_32 { uint32_t l; } __attribute__((packed)) __attribute__((may_alias));
    register unsigned int tmp;
    unsigned int re_index = s->index;
    unsigned int re_cache;
    re_cache = __builtin_bswap32( ( (const union unaligned_32 *)(s->buffer + (re_index >> 3))   )->l) << (re_index & 7);
    tmp = (((uint32_t)(re_cache))>>(32-n));
    re_index += n;
    s->index = re_index;
    return tmp;
}

/**
 * Read 0-32 bits.
 */

static inline unsigned int get_bits_long(GetBitContext *s, int n)
{
   assert(n>=0 && n<=32);
    if (!n) {
        return 0;

    } else if (n <= 25) {
        return get_bits(s, n);
    } else {
        unsigned ret = get_bits(s, 16) << (n - 16);
        return ret | get_bits(s, n - 16);

    }

}

#endif
