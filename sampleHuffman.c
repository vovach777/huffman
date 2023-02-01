//gcc sampleHuffman.c huffman.c  -o sampleHuffman.exe
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "huffman.h"

static int dataset[] = {
 108,111,108,110,105,107,109,109,108,106,106,109,109,104,107,107,
 107,109,105,107,108,104,107,107,106,104, 99,106,107,105,108,107,
 103,103,109,101,111,118,137,126,141,137,140,141,140,133,121,139,
 131,116,105,110,103,105,105,105,110,108,109,109,109,110,108,102,
 125,137,139,132,138,141,141,141,138,131,117,125,132,140,138,135,
 120,120,116,112,117,110,113,116,114,113,110,112,115,117,118,114,
 113,113,109,111,112,109,109,113,110,109,108,106,107,108,112,109,
 112,112,114,113,114,114,123,118,136,140,141,141,138,127,119,135,
 118,116,114,116,113,112,112,112,111,110,109,109,109,112,113,110,
 107,108,107,106,106,106,108,107,108,109,108,108,108,108,108,108,
 108,108,106,107,106,106,105,106,106,106,108,108,108,107,108,108,
 108,108,109,108,109,109,110,110,111,112,112,111,109,109,107,107,
 111,108,111,111,112,117,121,114,117,132,141,127,113,109,110,112,
 111,126,136,118,132,142,138,138,140,142,136,137,138,142,139,140,
 139,138,130,139,128,111, 96,112,101,102,109,104,101,112,127,113,
 130,137,137,135,136,140,140,139,140,133,127,135,137,138,136,137
 };

void print_Hufftree(Hufftree*h);
void print_Hufftree2(Hufftree*h);


int main() {

   int frequency[256] = { 0};
   for (int i=0; i<256; i++)
      frequency[ dataset[i] ]++;
   Hufftree *encoder  = new_Hufftree(frequency,256);
   print_Hufftree(encoder);

    char * tree_code = encodetree_Hufmantree(encoder);
    printf("tree size: %d bit\n", BITSTR_LEN(tree_code));

   char * packed_data = bitstr_dup(tree_code);

   for (int i=0; i<256; i++) {
      char *tmp = packed_data;
      packed_data = bitstr_add(packed_data, encode_Hufftree2(encoder, dataset[i]));
      delete_bitstr(tmp);
   }

   printf("compressed: %d -> %d bit\n",256*8,BITSTR_LEN(packed_data));
   char * packed_data_p = packed_data;
   //decompress & verify
   Hufftree*decoder = new_Hufftree2(packed_data_p,&packed_data_p);
   for (int i=0; i<256; i++) {
      if (decode_Hufftree2(decoder,packed_data_p,&packed_data_p) != dataset[i])
         printf("fail at %d!\n",i);
   }

   print_Hufftree2(decoder);

   delete_bitstr(packed_data);
   delete_Hufftree(encoder);
   delete_Hufftree(decoder);
}
