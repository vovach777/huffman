//gcc huffpack.c huffman.c -o huffpack.exe
//gcc sampleHuffman.c huffman.c  -o sampleHuffman.exe
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "huffman.h"


typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
} PutBitContext;

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
                                   int buffer_size)
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


static void put_bitstr(PutBitContext *s, char * b) {
   int sz = BITSTR_LEN(b);
   while (sz--) {
      assert(*b == 0 || *b == 1);
      put_bits(s,1, *b++);
   }
}

typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
    size_t index;
    size_t size_in_bits;
} GetBitContext;


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
      printf("overflow!");
       return -1;
    }
    uint8_t result     = s->buffer[index >> 3];
    result <<= index & 7;
    result >>= 8 - 1;
    index++;
    s->index = index;
    return result;
}



void print_Hufftree(Hufftree*h);
void print_Hufftree2(Hufftree*h);

int main(int argc, char**argv) {

   FILE *f = fopen(argv[1],"rb");
   assert(f);
   fseek(f,0,SEEK_END);
   size_t size = (size_t) ftello64(f);
   fseek(f,0,SEEK_SET);
   uint8_t * data = malloc(size);
   fread(data,1,size,f);
   fclose(f);
   char out_file[_MAX_PATH] = {0};
   strcat(out_file,argv[1]);
   int ofl = strlen(out_file);
   //printf( "%d %d [%s]\n ",ofl, strcmp(out_file+ofl-8,".huffman"),out_file+ofl-8);

   if ( ofl > 8 && ( strcmp(out_file+ofl-8,".huffman")==0) ) {
      out_file[ ofl-8 ] = 0;
      f = fopen(out_file,"r");
      if (f) {
         fclose(f);
         printf("decompressed file ready exist!");
         free(data);
         return 1;
      }
      f = fopen(out_file,"wb");
      assert(f);
      /* decompression here */
      GetBitContext gb = {0};
      init_get_bits(&gb, data, size);
      char * bitcodes = malloc(size<<3);
      int bit;
      char * bitcodes_p = bitcodes;
      char * bitcodes_end = bitcodes + (size<<3);
      while (bitcodes_p < bitcodes_end)
         *bitcodes_p++=get_bits1(&gb);

      bitcodes_p = bitcodes;
      size_t nb_symbols = decode_golomb_u_str(bitcodes_p,&bitcodes_p);

      printf("decompress %s %zu -> %zu bytes...\n",out_file,  size, nb_symbols);

      Hufftree *decoder = new_Hufftree2(bitcodes_p,&bitcodes_p);

      data = realloc(data,nb_symbols);
      uint8_t * data_p = data;
      for (size_t count=nb_symbols; count; count--) {
         *data_p++=decode_Hufftree2(decoder,bitcodes_p,&bitcodes_p);
      }
      fwrite(data,1,nb_symbols,f);
      fclose(f);
      delete_Hufftreep(&decoder);
      free(bitcodes);
      free(data);
      return 0;
   }

   strcat(out_file,".huffman");
   f = fopen(out_file,"wb");
   assert(f);

   static int frequency[256] = { 0};
   for (int i=0; i<size; i++)
      frequency[ data[i] ]++;
   Hufftree *encoder  = new_Hufftree(frequency,256);
   print_Hufftree(encoder);
   print_Hufftree2(encoder);
   PutBitContext pb;
   init_put_bits(&pb,malloc(size),size);
   char * sizeof_file = encode_golomb_u_bitstr(size);
   put_bitstr(&pb, sizeof_file);

    char * tree_code = encodetree_Hufmantree(encoder);
    printf("tree bits: "); bitstr_print(tree_code); printf("\n");
    printf("tree size: %d bit\n", BITSTR_LEN(tree_code));
    put_bitstr(&pb, tree_code);
    delete_bitstr(tree_code);


   uint8_t * data_p = data;
   uint8_t * data_end = data_p + size;

   while (data_p<data_end)
      put_bitstr(&pb, encode_Hufftree2(encoder, *data_p++) );

   size_t packed_size = put_bytes_count(&pb);

   printf("compressed: %d -> %d bytes\n",size,packed_size);
   fwrite(pb.buf,1,packed_size,f);
   fclose(f);
   free(pb.buf);
   free(data);
   delete_Hufftree(encoder);

}
