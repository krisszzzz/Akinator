#include "akinator.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Stack_T(String)  // Stack used to "definition" and "comparison" modes

#define BUFFER_SIZE 256   // buffer that will used to get input

// DSL to simplify code
#define LEFT_IS_FILLED			\
node->child_nodes[0].is_filled

#define RIGHT_IS_FILLED			\
node->child_nodes[1].is_filled

#define LEFT					\
node->child_nodes[0]

#define RIGHT					\
node->child_nodes[1]

// For correct input
int InputString(char *buffer) 
{
	char character = 0;
	int iter_count = 0;
	
	while((character = getchar()) != '\n') 
	{
		buffer[iter_count] = character;
		iter_count++;
		if(iter_count >= BUFFER_SIZE - 1) {
			printf("Warning: buffer overflow\n");
			return EOF;
		}
	} 
	buffer[iter_count] = '\0';
	return 0;
}

// ++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+-+-
// For writing tree to file

int AkinatorOutputNode(Tree_node *node, FILE *dst)
{
	if(node == nullptr) {
		return 0;	
	}
	RET_IF(FprintNodeData(node, dst) == INCCORECT_NODE_TYPE, INCCORECT_NODE_TYPE,
		   "Unknown name of type\n");
	
	if(node->child_nodes) { // if child_nodes != nullptr
		if(LEFT_IS_FILLED) {
			fprintf(dst, "{");	
			AkinatorOutputNode(&LEFT,  dst);
			fprintf(dst, "}");
		}

		if(RIGHT_IS_FILLED) {
			fprintf(dst, "{");	
			AkinatorOutputNode(&RIGHT, dst);
			fprintf(dst, "}");
		}
	}
	return 0;
}

int AkinatorStartWritingToFile(Tree *tree, FILE *dst)
{
	RET_IF(tree == nullptr, NULLPTR_TREE, "Can not work with nullptr object\n");
	RET_IF(dst  == nullptr, NULLPTR_FILE, "You have to give valid FILE*\n");
	fprintf(dst, "{");
	int ret_val = AkinatorOutputNode(&tree->root_node, dst); // Print all child nodes from root node
	fprintf(dst, "}");
	return ret_val;
}

/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/// Quessing mode part

static int InitMode(FILE *dst, char *buffer) 
{
	printf("Unknown who? [Y/n] ");
	while(true)
	{
		if(InputString(buffer) == 0 && ('y' == tolower(buffer[0]) || 'n' == tolower(buffer[0]))) { break; }
		printf("Inccorect input\n");
	}
	printf("This is init mode - it only show how the program will work\n"
		   "Independently what your answer, you should print a feature by which you can distinquish what "
		   "you have printed from unknown who\n");
	InputString(buffer);

	Tree tree = {};
	TreeCtor(&tree, 2, STRING, buffer);
	AkinatorStartWritingToFile(&tree, dst);
	TreeDtor(&tree);

	return 0;
}

static int AnswerNoProcessing(Tree_node *node, char *buffer)
{
	printf("Ok, it's strange. Probably my data base is small. But you can help me human!.\n"
			"Say, on what feature program can distinquish what have guessed differ from %s \n"
			"Also type what you have guessed\n", node->data.type_string);

	printf("feature: ");
	InputString(buffer);
	AddNode(node, STRING, node->data.type_string);
	if(node->is_filled) {
		free(node->data.type_string);
	}

	FillData(node, STRING, buffer);
	printf("what you guessed: ");
	InputString(buffer);
	AddNode(node, STRING, buffer);
	return 0;
}

static int InputToConsole(Tree_node *node, FILE *dst, char *buffer) 
{
	FprintNodeData(node, stdout);
	putchar('?');
	printf(" [Y/n] ");
	
	while(true)
	{
		if(InputString(buffer) == 0 && ('y' == tolower(buffer[0]) || 'n' == tolower(buffer[0]))) { break; } 
		printf("Inccorect input\n");
	}
	char answer = buffer[0];

	switch(tolower(answer)) 
	{
		/// if user say yes the quessing is over
		case 'y': {
			if(node->child_nodes == nullptr || !RIGHT_IS_FILLED) {
				printf("This what you quess.\n");
			} else {
				InputToConsole(&RIGHT, dst, buffer);
			}
			break;
		}
		/// if user say no quessing is continue
		case 'n': {
			if(node->child_nodes == nullptr || !LEFT_IS_FILLED) { // No child nodes, can not continue quessing
				AnswerNoProcessing(node, buffer);								
			} else {
				InputToConsole(&LEFT, dst, buffer); // recursively call function to continue quessing
			}
			break;
		}
		default: {
			ErrorPrint("Something went wrong\n");
			return UNEXPECTED_ERROR;
		}
	}

	return 0;
}
/// Quessing Mode

int StartQuessingMode()
{
	char buffer[BUFFER_SIZE] = {};
	FILE *to_save_quess_result_in_file = fopen("DON'T_REMOVE_THIS_FILE.txt", "a+");
	RET_IF(to_save_quess_result_in_file == nullptr, INCCORECT_FILE_OPEN, "Something went wrong while opening file "																  
															 	         "to write/read quess result\n");
	fseek(to_save_quess_result_in_file, 0, SEEK_END);
	long num_of_char = ftell(to_save_quess_result_in_file);
	fseek(to_save_quess_result_in_file, 0, SEEK_SET);

	if(num_of_char == 0) {
		return InitMode(to_save_quess_result_in_file, buffer);
	}

	Tree* tree = nullptr;
	tree	   = ReadAndMakeTree(to_save_quess_result_in_file);

	fclose(to_save_quess_result_in_file); // reopen the file to rewriting
	to_save_quess_result_in_file = fopen("DON'T_REMOVE_THIS_FILE.txt", "w+");
	RET_IF(to_save_quess_result_in_file == nullptr, INCCORECT_FILE_OPEN, "Something went wrong while opening file "																  
															 	         "to write/read quess result\n");

	RET_IF(InputToConsole(&tree->root_node, to_save_quess_result_in_file, buffer) != 0, INCCORECT_READING, "Error in InputConsole()\n");

	StartObjectDefinition(tree, buffer); // Object definition
	StartObjectComparison(tree, buffer); // Object comparison

	AkinatorStartWritingToFile(tree, to_save_quess_result_in_file); // Save tree in file

	fclose(to_save_quess_result_in_file);
	TreeDtor(tree);
	free(tree);

	return 0;
}


/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/// For reading from file

Tree* ReadAndMakeTree(FILE *src)
{
	RET_IF(src == nullptr, nullptr, "Can not work with nullptr file\n");	
	char buffer[BUFFER_SIZE] = {};
	fseek(src, 0, SEEK_SET); /// Use this because the file maybe have offset
	RET_IF(fgetc(src) != '{', nullptr, "Inccorect, the first element of file should be \"{\"\n");

	fscanf(src, "%[^{}]", buffer);
	Tree* tree = (Tree*)calloc(1, sizeof(Tree));
	RET_IF(tree == nullptr, nullptr, "Memory allocation error\n");
	const int num_of_branches = 2; 		   			  // Our tree is binary

	TreeCtor(tree, num_of_branches, STRING, buffer);  // Warning: transferring the to_save_in_tree, you are responsible for data loss in him

	RET_IF(GetBrackets(&tree->root_node, src, buffer) != 0, nullptr, "Error while reading the file\n");

	return tree;
}

/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/// Function for Akinator file reading

int GetString(Tree_node *node, FILE *src, char *buffer)
{											///TODO change error code name
	fscanf(src, "%[^{}]", buffer);
	return AddNode(node, STRING, buffer);
}

int GetBrackets(Tree_node *node, FILE *src, char *buffer)
{
	int brackets = 0;
	while((brackets = fgetc(src)) != EOF)
	{
		switch(brackets) {
			case '{': {
				int node_degree = node->node_degree;

				RET_IF(GetString(node, src, buffer) != 0, INCCORECT_READING, "Error in GetString function\n");
				if(node_degree == 0) {
					node = &LEFT; // if meet bracket { go to child nodes, firstly to left after to right
				} else { 
					node = &RIGHT; 
				}
				break;
			}
			case '}': {
				node = node->parent; // go to parent node
				break;
			}
			default: {
				ErrorPrint("Something went wrong, expected brackets { or }\n");
				return INCCORECT_READING;
			}
		}
	}
	return 0;

}

#define PARENT \
node->parent
									/// Stack that will be created			// node to build stack
static int MakeStackFromNode(Stack_String* stack, String* buffer, Tree* tree, Tree_node *node)
{
	static int string_count = 0;
	if(node == &(PARENT->child_nodes[0])) { 		        // child_nodes[0] is left, left means no variant 
		buffer[string_count].is_negative = 1;    // Mark the string
	}
	buffer[string_count].string = PARENT->data.type_string;

	StackPush_String(stack, &buffer[string_count]);
	string_count++;
	if(PARENT != &tree->root_node) {
		MakeStackFromNode(stack, buffer, tree, PARENT);
	}

	return 0;
}

/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/// All function related with "Definition" mode

#define FIND_OBJECT(ptr_to_save_finding_result)										\
while(ptr_to_save_finding_result == nullptr) {										\
	InputString(buffer);											    			\
	ptr_to_save_finding_result = FindObjectByStringData(&tree->root_node, buffer);  \
	if(ptr_to_save_finding_result == nullptr) {										\
		printf("Not found such node\n");											\
	}																			    \
}

static int DefineObject(Tree *tree, Tree_node *node)
{
	const int num_of_strings = 64;
	RET_IF(tree == nullptr, NULLPTR_TREE, "Can not work with nullptr tree\n");
	RET_IF(node == nullptr, NULLPTR_NODE, "Can not work with nullptr node\n");

	Stack_String stack = {};
	int stack_min_size = 16;

	String buffer[num_of_strings] = {};

	CtorStack_String(&stack, stack_min_size);
	MakeStackFromNode(&stack, buffer, tree, node);
	
	printf("So the object definition is: ");
	while(stack.size != 0)
	{
		String curr_string = StackPop_String(&stack);
		if(curr_string.is_negative) {
			printf("not ");
		}
		printf("%s->", curr_string.string);
	}
	FprintNodeData(node, stdout);
	printf("\n");
	return 0;
}

int StartObjectDefinition(Tree *tree, char *buffer)
{
	printf("Want to define some object? [Y/n] ");
	while(true)
	{
		if(InputString(buffer) == 0 && ('y' == tolower(buffer[0]) || 'n' == tolower(buffer[0]))) { break; } 
		printf("Inccorect input\n");
	}
	char answer = tolower(buffer[0]);
	if(answer == 'y') {
		/// Function
		Tree_node *node_to_find = nullptr;
		printf("Please type object name: ");				 			            
		while(node_to_find == nullptr) {
			InputString(buffer);						
			if(strncmp(buffer, "Ded64", 5) == 0) {
				system("xdg-open Cringe.jpg");
			}			
			node_to_find = FindObjectByStringData(&tree->root_node, buffer);  
			if(node_to_find == nullptr) {					
				printf("Not found such node\n");
			}																			    
		}

		DefineObject(tree, node_to_find);	
	} 
	return 0;
}

static int PrintComparisonResult(Stack_String *first_object_stack, Stack_String *second_object_stack, 
								 Tree_node 	  *first_object,       Tree_node    *second_object)
{
	while(first_object_stack->size != 0 && second_object_stack->size != 0) 
	{
		String first  = StackPop_String(first_object_stack);
		String second = StackPop_String(second_object_stack);
		if((first.is_negative && second.is_negative) || (!first.is_negative && !second.is_negative)) { 
			if(strcmp(first.string, second.string) == 0) {
				printf("both ");
				if(first.is_negative) {
					printf("not ");
				}
				printf("%s.\n", first.string);
		 	}
		} else {
			if(strcmp(first.string, second.string) == 0) {
				if(second.is_negative) {
					Tree_node* temp = first_object;
					first_object    = second_object; // switch objects
					second_object   = temp;
				}
		 		printf("%s is not %s, but %s is %s.\n", first_object->data.type_string,  first.string, 
				    	 	 						    second_object->data.type_string, second.string);
			}
		}
	}		
	return 0;

}

int StartObjectComparison(Tree *tree, char *buffer)
{
	printf("Want to compare some objects? [Y/n] ");
	while(true)
	{
		if(InputString(buffer) == 0 && ('y' == tolower(buffer[0]) || 'n' == tolower(buffer[0]))) { break; } 
		printf("Inccorect input\n");
	}
	char answer = tolower(buffer[0]);

	Tree_node* first_object  = nullptr;
	Tree_node* second_object = nullptr;

	if(answer == 'y') {

		printf("Please, write a first object name: ");
		FIND_OBJECT(first_object);

		printf("Please. write a second object name: ");
		FIND_OBJECT(second_object);

		const int capacity 			 = 16;
		const int first_buffer_size  = 64;
		const int second_buffer_size = 64;

		Stack_String first_object_stack  = {};
		Stack_String second_object_stack = {};

		CtorStack_String(&first_object_stack,  capacity);
		CtorStack_String(&second_object_stack, capacity);

		String first_buffer[first_buffer_size]   = {};
		String second_buffer[second_buffer_size] = {};

		MakeStackFromNode(&first_object_stack,  first_buffer,  tree, first_object);
		MakeStackFromNode(&second_object_stack, second_buffer, tree, second_object);

		PrintComparisonResult(&first_object_stack, &second_object_stack, first_object, second_object);		
		
		DtorStack_String(&first_object_stack);		
		DtorStack_String(&second_object_stack);		

	}
	return 0;
	
}


/// ++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+-+-
/// Finding nodes
// Very slow because use strcmp to find strings
// use it ratinally

			// Starting node, from this node findings will start (First node not checked only his child nodes)
Tree_node* FindObjectByStringData(Tree_node *node, char *string_to_find)
{
	Tree_node* to_ret = nullptr;
	if(strcmp(node->data.type_string, string_to_find) == 0) {
		return node;
	}

	for(int node_count = 0; node_count < node->node_degree; ++node_count)
	{
		if(node->child_nodes[node_count].type_id == STRING) {
			if(strcmp(node->child_nodes[node_count].data.type_string, string_to_find) == 0) {
				return &node->child_nodes[node_count];
			}
		}
		if((to_ret = FindObjectByStringData(&node->child_nodes[node_count], string_to_find)) != nullptr) {
			return to_ret;
		}		
	}	
	return nullptr;
}
