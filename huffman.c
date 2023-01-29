#include "huffman.h"
#include <assert.h>
#include <stdio.h>
struct Node;
typedef struct Node Node;


struct Hufftree
{
  Node* tree;
  int  size;
  char** encoding;
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


  static void fill_Node(Node *n, char ** encoding,char*prefix )
  {
    if (n->leftChild)
    {
      int level_len = BITSTR_LEN(prefix)+1;
      BITSTR_LEN(prefix) = level_len;
      prefix[ level_len-1 ] = 0;
      fill_Node(n->leftChild,  encoding, prefix);
      BITSTR_LEN(prefix) = level_len;
      prefix[ level_len-1 ] = 1;
      fill_Node(n->rightChild, encoding, prefix);
      BITSTR_LEN(prefix) = level_len-1;
    }
    else {
      encoding[n->data] = bitstr_dup( prefix );
    }
}

void delete_Hufftree(Hufftree** hufftree) {
   for (int i=0; i<  hufftree[0]->size; i++)
      delete_bitstr( hufftree[0]->encoding[i] );
   free(*hufftree);
   *hufftree = NULL;
}


static int compare(const void*a, const void *b) {
    Node* aa = *(Node**) a;
    Node* bb = *(Node**) b;
    if (aa == bb)
      return 0;
   // if (aa == NULL)
   //    return 1;
   // if (bb == NULL)
   //    return -1;
   return aa->frequency - bb->frequency;
}

Hufftree* new_Hufftree(Frequency *frequency, int size)
{
  Hufftree *hufftree = (Hufftree *)calloc(1, sizeof(Hufftree));
  hufftree->encoding = calloc(size,sizeof(char*));
  hufftree->size = size;
  Node** pqueue = alloca(size*sizeof(Node*));
  int pqueue_sz = 0;
  for (int i=0; i<size; i++) {
     if (frequency[i])
        pqueue[pqueue_sz++] = new_Node(frequency[i], i);
   }

   Node** pqueue_p = pqueue;
   assert( pqueue_sz <= size);
   int nb_nodes=pqueue_sz;
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
    pqueue_p++; pqueue_sz--; nb_nodes++;
    pqueue_p[0] = new_Node2(top,top2);
  }
  char* bitvec = new_bitstr(nb_nodes+4);
  assert(BITSTR_LEN(bitvec) == 0);
  fill_Node(hufftree->tree, hufftree->encoding, bitvec);
  delete_bitstr( bitvec );
  return hufftree;
}


void print_Hufftree(Hufftree*h) {
   for (int i=0; i<h->size; i++) {
      if (h->encoding[i] == NULL)
         continue;

      // if ((i >= '0' && i <='9') || (i >= 'a' && i<='z' || (i >= 'A' && i<='Z')))
      //    printf("%4c: ", i);
      // else
         printf("%4d: ", i);

      char *codeword = h->encoding[i];
      if (codeword ) {

         bitstr_print(codeword);
      }
      printf("\n");
   }
}

char * encode_Hufftree(Hufftree*h, unsigned v)
{
   assert( v < h->size );
   return   h->encoding[ v ];
}


 int decode_Hufftree(Hufftree*h, char * from, char ** end)
{
  Node* node = h->tree;
  char * from_p = from;

  for(;;){
    node = *from_p++ ? node->rightChild : node->leftChild;
    if (!node->leftChild)
    {
      *end = from_p;
      return node->data;
    }
  }

}
