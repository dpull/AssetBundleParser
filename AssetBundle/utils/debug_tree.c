#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "platform.h"

#define BUFFER_SIZE 2048
#define PREFIX_BUFFER_SIZE 512

struct debug_tree
{
	struct debug_tree* parent;
	struct debug_tree* child;
	struct debug_tree* sibling;
	char* data;
};

struct debug_tree* debug_tree_insert(struct debug_tree* parent, char* data)
{
	struct debug_tree* tree = (struct debug_tree*)malloc(sizeof(*tree));
	memset(tree, 0, sizeof(*tree));
	tree->data = data;

	if (!parent)
		return tree;

	if (parent->child) {
		struct debug_tree* sibling = parent->child;
		while (sibling->sibling) 
			sibling = sibling->sibling;
		
		sibling->sibling = tree;
		tree->parent = sibling->parent;
	}
	else {
		parent->child = tree;
		tree->parent = parent;
	}

	return tree;
}

struct debug_tree* debug_tree_create(struct debug_tree* parent, const char* format, ...)
{
    va_list marker;
	char buff[BUFFER_SIZE];
	
	va_start(marker, format);
	int length = vsnprintf(buff, sizeof(buff), format, marker);
	va_end(marker);

	if (length < 0 || length >= sizeof(buff))
		return NULL;

	char* data = (char*)malloc(length + 1);
	memcpy(data, buff, length);
	data[length] = '\0';
	return debug_tree_insert(parent, data);
}

void debug_tree_destory(struct debug_tree* tree)
{
	while (tree->child) {
		debug_tree_destory(tree->child);
	}

	if (tree->parent) {
		if (tree->parent->child == tree) {
			tree->parent->child = tree->parent->child->sibling;
		} else {
			struct debug_tree* sibling = tree->parent->child;
			while (sibling->sibling) {
				if (sibling->sibling == tree) {
					sibling->sibling = sibling->sibling->sibling;
					break;
				}
				sibling = sibling->sibling;
			}
		}
	}

	tree->parent = NULL;
	tree->sibling = NULL;

	free(tree->data);
	free(tree);
}

void debug_tree_print(struct debug_tree* tree, FILE* stream, const char* prefix)
{
   	char child_prefix[PREFIX_BUFFER_SIZE];
    char last_child_prefix[PREFIX_BUFFER_SIZE];
 
    if (!prefix)
        prefix = "";
    
    snprintf(child_prefix, sizeof(child_prefix), "%s│  ", prefix);
    snprintf(last_child_prefix, sizeof(last_child_prefix), "%s    ", prefix);
    
    char* child_prefix_ref;
    
    if (tree->sibling) {
        fprintf(stream, "%s├─%s\n", prefix, tree->data);
        child_prefix_ref = child_prefix;
    }
    else {
        fprintf(stream, "%s└─%s\n", prefix, tree->data);
        child_prefix_ref = last_child_prefix;
    }
    
    if (tree->child)
        debug_tree_print(tree->child, stream, child_prefix_ref);
    
    if (tree->sibling)
        debug_tree_print(tree->sibling, stream, prefix);
}
