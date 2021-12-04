#if !defined AKINATOR_INCLUDED

#define AKINATOR_INCLUDED

#include "tree.h"
#include "../Stack.h"

typedef Tree_node* node_t;

enum AKINATOR_ERRORS
{
    INCCORECT_INPUT = -100500,
    FILE_OPEN_FAILED,
    INCCORECT_READING,
    UNEXPECTED_ERROR,
    NULLPTR_FILE
};

struct String 
{
    char* string;
    int   is_negative;
};

int AkinatorStartWritingToFile(Tree *tree, FILE *dst);

int GetString(Tree_node *node, FILE *src, char *buffer);

int GetBrackets(Tree_node *node, FILE *src, char *buffer);

int StartQuessingMode();

int StartObjectComparison(Tree *tree, char *buffer);

Tree* ReadAndMakeTree(FILE *src);

int StartObjectDefinition(Tree *tree, char *buffer);

Tree_node* FindObjectByStringData(Tree_node *node, char *string_to_find);

#endif

