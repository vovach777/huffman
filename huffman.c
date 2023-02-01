#include "huffman.h"
#include <assert.h>
#include <stdio.h>

struct Node;
typedef struct Node Node;
struct SEncoding;
typedef struct SEncoding SEncoding;


struct SEncoding {
   char * code;    //classic huffman
   char * code_value; //code + symbol
   size_t send_count;
};

void SEncoding_init(SEncoding* se, char * prefix, unsigned v) {
   se->code = bitstr_dup(prefix);
   char * tmp = encode_golomb_u_bitstr(v);
   se->code_value = bitstr_add(prefix, tmp);
   delete_bitstr(tmp);
   se->send_count = 0;
}

void SEncoding_free(SEncoding* se)  {
   delete_bitstrp(&se->code);
   delete_bitstrp(&se->code_value);
   se->send_count = 0;
}


struct Hufftree
{
  Node* tree;
  int size;     //alphabet size
  int symbols;  //symbols (freq > 0);
  SEncoding* encoding;
};


struct Node
{
   Frequency frequency;
   Node* leftChild;
   Node* rightChild; // if leftChild != 0
   DataType data;  // if leftChild == 0
};

  Node* new_Node(Frequency f, DataType d) {
      Node* n = calloc(1,sizeof(Node));
      n->frequency = f;
      n->data = d;
      return n;
  }



  Node* new_Node2(Node* left, Node* right) {
   Node* n = calloc(1,sizeof(Node));
    n->frequency = left->frequency + right->frequency;
    n->data = (left->data << 8) | (right->data << 16);
    n->leftChild = left;
    n->rightChild = right;
    return n;
  }

  void delete_Node(Node *n)
  {
    if (n->leftChild)
    {
      delete_Node(n->leftChild);
      delete_Node(n->rightChild);
    }
  };


  static void fill_Node(Node *n, Hufftree* h,char*prefix )
  {
    if (n->leftChild)
    {
      int level_len = BITSTR_LEN(prefix)+1;
      BITSTR_LEN(prefix) = level_len;
      prefix[ level_len-1 ] = 0;
      fill_Node(n->leftChild, h, prefix);
      BITSTR_LEN(prefix) = level_len;
      prefix[ level_len-1 ] = 1;
      fill_Node(n->rightChild, h, prefix);
      BITSTR_LEN(prefix) = level_len-1;
    }
    else {
      SEncoding_init( &h->encoding[n->data], prefix, n->data );
    }
}

//static int codes_to_values[16] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 255};


void delete_Hufftree(Hufftree* hufftree) {
   if (hufftree) {
      for (int i=0; i<  hufftree->size; i++) {
         delete_bitstrp( &hufftree->encoding[i].code );
         delete_bitstrp( &hufftree->encoding[i].code_value );
      }
      delete_Node(hufftree->tree);
      hufftree->tree=NULL;
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
   // if (aa->frequency > bb->frequency)
   //    return 1;

   // if (aa->frequency == bb->frequency) {
   //    if (aa->leftChild == NULL && (bb->leftChild == NULL)) {
   //       //both leaf
   //       return aa->data - bb->data;
   //    } else
   //      return (aa->leftChild) ? 0 : 1; //leaf > node

   // } else
   //   return -1;
}

Hufftree* new_Hufftree(Frequency *frequency, int size)
{
   Hufftree *hufftree = (Hufftree *)calloc(1, sizeof(Hufftree));
   hufftree->encoding = calloc(size,sizeof(SEncoding));
   hufftree->size = size;
   Node** pqueue = alloca(size*sizeof(Node*));
   int pqueue_sz = 0;
   for (int i=0; i<size; i++) {
      if (frequency[i])
         pqueue[pqueue_sz++] = new_Node(frequency[i], i);
   }

   Node** pqueue_p = pqueue;
   int nb_nodes=pqueue_sz;
   hufftree->symbols = nb_nodes;
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
      pqueue_p[0] = new_Node2(top,top2);
   }
   char* bitvec = new_bitstr(nb_nodes*2);
   assert(BITSTR_LEN(bitvec) == 0);
   fill_Node(hufftree->tree, hufftree, bitvec);
   delete_bitstr( bitvec );
   return hufftree;
}


void print_Hufftree(Hufftree*h) {
   printf("size: %d, symbols: %d\n", h->size, h->symbols);
   for (int i=0; i<h->size; i++) {
      if (h->encoding[i].code) {

      // if ((i >= '0' && i <='9') || (i >= 'a' && i<='z' || (i >= 'A' && i<='Z')))
      //    printf("%4c: ", i);
      // else
         printf("%4d: ", i);
         bitstr_print(h->encoding[i].code);
         //bitstr_print(h->encoding[i].code_value);
         printf("\n");
      }
   }
   printf("\n");
}

char * encode_Hufftree(Hufftree*h, unsigned v)
{
   assert(v < h->size );
   return h->encoding[ v ].code;
}


int decode_Hufftree(Hufftree*h, char * from, char ** end)
{
   Node* node = h->tree;
   char * from_p = from;

   for(;;){
      node = *from_p++ ? node->rightChild : node->leftChild;
      if (!node->leftChild)
      {
         if (end) *end = from_p;
         return node->data;
      }
   }

}


// static int ilog2_32(uint32_t v)
// {
//    if (!v)
//       return 0;

//    return 32-__builtin_clz(v);
// }


char *  encodetree_Hufmantree( Hufftree*h) {

   int len = 0;

   char * packtree = new_bitstr(h->symbols*2); /* idnk how to calculate max size*/

   void pack(Node*n ) {
        if (n->leftChild) {
            packtree[len++] = 0;
            pack(n->leftChild);
            pack(n->rightChild);
        } else {
          packtree[len++] = 1;
          h->encoding[n->data].send_count = 0;
        }
   }

   pack(h->tree);
   BITSTR_LEN(packtree) = len;

   return packtree;
}



Hufftree*  new_Hufftree2(char * from, char **end) {

   Hufftree *hufftree = calloc(1, sizeof(Hufftree));

   Node* unpack() {
      if  (*from++)
         return new_Node(0,0);
      else {
           Node* left = unpack();
           Node* right = unpack();
           return new_Node2( left, right );
      }
   }
   hufftree->tree = unpack();
   if (end) *end = from;
   return hufftree;
}

char * encode_Hufftree2(Hufftree*h, unsigned v)
{
   assert(v < h->size );
   if (h->encoding[v].send_count++) {
      return  h->encoding[ v ].code;
   } else {
      return  h->encoding[ v ].code_value;
   }
}


int decode_Hufftree2(Hufftree*h, char * from, char ** end)
{
  Node* node = h->tree;
  char * from_p = from;
  //printf("{decode: ");

  for(;;){
   //printf("%d",*from_p);
    node = *from_p++ ? node->rightChild : node->leftChild;
    if (!node->leftChild)
    {

      if (node->frequency++ == 0)
           node->data = decode_golomb_u_str(from_p,&from_p);
      if (end) *end = from_p;
     // printf("=%u}\n",node->data);
      return node->data;
    }
  }

}


static void PrintPretty(Node*n, char * indent,  bool last)
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
         printf("*\n");
      else
         printf("%d\n",n->data);

       if (n->leftChild) {
           PrintPretty(n->leftChild, indent, false);
           PrintPretty(n->rightChild, indent, true);
       }
       indent[len] = 0;
   }


void print_Hufftree2(Hufftree*h) {
    char indent[8192] = {0};
    PrintPretty(h->tree,indent,true);
}
