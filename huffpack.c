//gcc huffpack.c -Ofast huffman.c -o huffpack.exe
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "huffman.h"

void print_Hufftree(Hufftree*h);
void print_Hufftree2(Hufftree*h);


int main(int argc, char**argv) {

   FILE *f = fopen(argv[1],"rb");
   assert(f);
   fseek(f,0,SEEK_END);
   _off64_t size64 = (size_t) ftello64(f);
   if ( sizeof(size_t) == 4 && size64 >> 32) {
      printf("for this file needs 64bit version of this app!\n");
      return 2;
   }
   size_t size = size64;
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
      f = fopen(out_file,"rb");
      if (f) {
         fclose(f);
         printf("decompressed file ready exist!\n");
         free(data);
         return 1;
      }
      f = fopen(out_file,"wb");
      assert(f);
      /* decompression here */
      GetBitContext gb = {0};
      init_get_bits(&gb, data, size);

      size_t nb_symbols = get_bits_long(&gb, 32);
      if (sizeof(size_t) == 8)
         nb_symbols |= (uint64_t)get_bits_long(&gb, 32) << 32;
      else {
         if ( get_bits_long(&gb, 32) ) {
            printf("to decompress this file needs 64bit version of this app!\n");
            return 2;
         }
      }

      printf("decompress %s %zu -> %zu bytes...\n",out_file,  size, nb_symbols);

      Hufftree *decoder = new_Hufftree2(&gb,256);

      uint8_t * data_out = malloc(nb_symbols);
      uint8_t * data_p = data_out;
      const uint8_t * data_end = data_p + nb_symbols;
      while (data_p < data_end) {
         *data_p++ = decode_Hufftree(decoder,&gb);
      }
      fwrite(data_out,1,nb_symbols,f);
      fclose(f);
      delete_Hufftreep(&decoder);
      free(data);
      free(data_out);
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
   put_bits32(&pb, (uint32_t)size64);
   put_bits32(&pb, size64>>32);

   size_t tree_size = encodetree_Hufmantree(encoder,&pb);
   printf("tree bits: %zu\n", tree_size);

   uint8_t * data_p = data;
   uint8_t * data_end = data_p + size;

   while (data_p<data_end) {
      encode_Hufftree(encoder, *data_p++, &pb);
   }

   size_t packed_size = put_bytes_count(&pb);

   printf("compressed: %zu -> %zu bytes\n",size,packed_size);
   fwrite(pb.buf,1,packed_size,f);
   fclose(f);
   free(pb.buf);
   free(data);
   delete_Hufftree(encoder);

}
