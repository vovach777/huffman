#include "huffman.h"
#include <assert.h>
#include <stdio.h>

struct Node;
typedef struct Node Node;
struct SEncoding;
typedef struct SEncoding SEncoding;


struct SEncoding {
   uint32_t code;
   int      bit_length;
};



struct Hufftree
{
  Node* tree;
  int size;     //alphabet size
  int symbols;  //symbols (freq > 0);
  SEncoding* encoding;
  Node*nodesPool;
};


struct Node
{
   Node* leftChild;
   Node* rightChild; // if leftChild != 0
   Frequency frequency;
   DataType data;  // if leftChild == 0
};


static int ilog2_32(uint32_t v)
{
   if (!v)
      return 0;

   return 32-__builtin_clz(v);
}


  static Node* init_Node(Frequency f, DataType d, Node* n) {
      n->frequency = f;
      n->data = d;
      return n;
  }



  static Node* init_Node2(Node* left, Node* right, Node*n) {
    n->frequency = left->frequency + right->frequency;
    n->data = 0;
    n->leftChild = left;
    n->rightChild = right;
    return n;
  }


static void fill_Node(Node *n, Hufftree* h,uint32_t prefix, int bit_length)
  {
    if (n->leftChild)
    {
      fill_Node(n->leftChild,  h, prefix<<1, bit_length+1);
      fill_Node(n->rightChild, h, prefix<<1|1, bit_length+1);
    }
    else {
      h->encoding[n->data].bit_length = bit_length;
      h->encoding[n->data].code = prefix;
    }
}

void delete_Hufftree(Hufftree* hufftree) {
   if (hufftree) {
      free(hufftree->encoding);
      free(hufftree->nodesPool);
      free(hufftree);
   }
}

void delete_Hufftreep(Hufftree** hufftree) {
   delete_Hufftree(*hufftree);
   *hufftree=NULL;
}

static int compare(const void*a, const void *b) {
    Node* aa = *(Node**) a;
    Node* bb = *(Node**) b;
    assert (aa != bb);
    return aa->frequency - bb->frequency;
}

Hufftree* new_Hufftree(Frequency *frequency, int size)
{
   Hufftree *hufftree = (Hufftree *)calloc(1, sizeof(Hufftree));
   Node* nodespool = hufftree->nodesPool = (Node*) calloc(size*2, sizeof(Node));
   assert(hufftree);
   hufftree->encoding = calloc(size,sizeof(SEncoding));
   assert(hufftree->encoding);
   hufftree->size = size;
   Node** pqueue = malloc(size*sizeof(Node*));
   assert(pqueue);
   int pqueue_sz = 0;
   for (int i=0; i<size; i++) {
      if (frequency[i])
         pqueue[pqueue_sz++] = init_Node(frequency[i], i, nodespool++);
   }

   Node** pqueue_p = pqueue;
   int nb_nodes=pqueue_sz;
   hufftree->symbols = nb_nodes;
   assert( pqueue_sz > 1 );
   while (pqueue_sz)
   {
      if (pqueue_sz >= 2) {
         qsort(pqueue_p,pqueue_sz,sizeof(*pqueue_p), compare);
      }

      Node* top = pqueue_p[0];
      Node* top2 = pqueue_sz >= 2 ? pqueue_p[1] : NULL;
      pqueue_p[0] = NULL;

      if (top2 == NULL)
      {
         hufftree->tree = top;
         break;
      }
      pqueue_p++; pqueue_sz--;
      pqueue_p[0] = init_Node2(top,top2,nodespool++);
   }
   free(pqueue);
   assert(hufftree->tree);
   fill_Node(hufftree->tree, hufftree, 0, 0);
   return hufftree;
}



char* toBinary(uint32_t n, int len)
{
    static char binary[64];
    char *binary_p = binary;
    uint32_t mask = 1 << len-1;
    while (mask) {
      *binary_p++ = n & mask ? '1' : '0';
      mask >>= 1;
    }
    *binary_p = 0;
    return binary;
}


void print_Hufftree(Hufftree*h) {
   printf("size: %d, symbols: %d\n", h->size, h->symbols);
   for (int i=0; i<h->size; i++) {
      if (h->encoding[i].bit_length) {
         printf("%4d: %s\n", i, toBinary(h->encoding[i].code, h->encoding[i].bit_length));
      }
   }
   printf("\n");
}

void encode_Hufftree(Hufftree*h, DataType v, PutBitContext *s)
{
   assert(v < h->size );
   int bit_length = h->encoding[ v ].bit_length;
   put_bits(s, h->encoding[ v ].bit_length, h->encoding[ v ].code);
}


DataType decode_Hufftree(Hufftree*h, GetBitContext *s)
{
   Node* node = h->tree;
   for(;;){
      node =  get_bits1(s) ? node->rightChild : node->leftChild;
      if (!node->leftChild)
      {
         return node->data;
      }
   }
}

size_t encodetree_Hufmantree( Hufftree*h, PutBitContext *s)
{
   size_t bit_legnth = 0;
   int bitwidth = ilog2_32( h->size-1 );
   // int bitwidth = ilog2_32(h->size-1);
   // put_bits(s, 4, bitwidth-1);

   void pack(Node*n ) {
        if (n->leftChild) {
            put_bits(s,1,0);
            pack(n->leftChild);
            pack(n->rightChild);
        } else {
            put_bits(s,1,1);
            put_bits(s,bitwidth,n->data);
            bit_legnth+=8;
        }
        bit_legnth++;
   }
   pack(h->tree);
   return bit_legnth;
}

Hufftree*  new_Hufftree2(GetBitContext *s, int size)
{

   Hufftree *hufftree = calloc(1, sizeof(Hufftree));

//   int bitsize = get_bits(s,4)+1;
//   hufftree->size = 1 << bitsize;
   hufftree->size = size;
   int bitsize = ilog2_32(size-1);
   Node* nodespool = hufftree->nodesPool = (Node*) calloc(hufftree->size*2, sizeof(Node));

   Node* unpack() {
      if (get_bits1(s)) {
         hufftree->symbols++;
         return init_Node(0,get_bits(s,bitsize),nodespool++);
      }
      else {
           Node* left = unpack();
           Node* right = unpack();
           return init_Node2( left, right, nodespool++ );
      }
   }
   hufftree->tree = unpack();

   return hufftree;
}

static void PrintPretty(Node*n, char * indent,  bool last, const char *name)
   {
       printf(indent);
       int len = strlen(indent);

       if (last)
       {
           printf("\\-");
           strcat(indent,"  ");
       }
       else
       {
           printf("|-");
           strcat(indent, "| ");
       }

       if (n->leftChild)
         printf("%s\n",name);
      else
         printf("%s = %u\n",name,n->data);

       if (n->leftChild) {
           PrintPretty(n->leftChild, indent, false,"Bit0");
           PrintPretty(n->rightChild, indent, true,"Bit1");
       }
       indent[len] = 0;
   }


void print_Hufftree2(Hufftree*h) {
    static char indent[8192] = {0};
    PrintPretty(h->tree,indent,true,"root");
}
