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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef int DataType;
typedef int Frequency;
struct Hufftree;
typedef struct Hufftree Hufftree;


Hufftree* new_Hufftree(Frequency *frequency, int size);
Hufftree*  new_Hufftree2(char * from, char **end);
void delete_Hufftree(Hufftree* hufftree);
void delete_Hufftreep(Hufftree** hufftree);
char * encode_Hufftree(Hufftree*h, unsigned v);
int decode_Hufftree(Hufftree*h, char * from, char ** end);

char * encode_Hufftree2(Hufftree*h, unsigned v);
int decode_Hufftree2(Hufftree*h, char * from, char ** end);
char *  encodetree_Hufmantree( Hufftree*h);

#define BITSTR_LEN(s) (((uint16_t*)(s))[-1])

static char * bitstr_dup(const char* bitstr) {
   uint16_t size = BITSTR_LEN(bitstr) + 2;
   char * dupstr = (char*) malloc(size);
   memcpy(dupstr, bitstr-2, size);
   return dupstr+2;
}

static char* new_bitstr(int size) {
   return  (char*) calloc(size+2,1) + 2;
}

static void delete_bitstr(char*bitstr) {
   if (bitstr) {
      free( bitstr-2 );
   }
}

static void delete_bitstrp(char**bitstr ) {
   if (bitstr) {
      delete_bitstr(*bitstr);
      *bitstr=NULL;
   }
}

static void bitstr_print(char*bitstr) {
   if (bitstr) {
      int size = BITSTR_LEN(bitstr);
      while (size--) printf("%d",(int)*bitstr++);
   }
}

static char * bitstr_add(const char* str1,  const char* str2) {
   const int len1 = BITSTR_LEN(str1);
   const int len2 = BITSTR_LEN(str2);
   const int len3 = len1 + len2;

   char * str3 = new_bitstr( len3 );
   BITSTR_LEN(str3) = len3;
   memcpy(str3, str1, len1);
   memcpy(str3+len1, str2, len2);
   return str3;
}

/* golomb */

static char * encode_golomb_u_bitstr( unsigned d) {
   d += 1;
   int bit_count = 32 - __builtin_clz(d);
   int ziro_count = bit_count-1;

   char *res = new_bitstr(ziro_count + bit_count);
   int len=0;

   while (ziro_count--) {
      res[len++] = 0;
   }
   while (bit_count--) {
      int bit = !!( d & (1 << (bit_count)) );
      res[len++] = bit;
   }
   BITSTR_LEN(res) = len;
   return res;
}


static unsigned decode_golomb_u_str(char*from, char**end) {
    int bit_count=0;
    while ( *from++==0) {
        bit_count++;
    }
    if (bit_count == 0) {
        if (end) *end = from;
        return 0;
    }
    unsigned res = 1;
    while (bit_count--) {
        res = res << 1 | (*from++);
    }

   if (end) *end = from;
    return res-1;
}

#endif
