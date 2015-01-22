#ifndef DEBUG_TREE_H
#define DEBUG_TREE_H

struct debug_tree* debug_tree_create(struct debug_tree* parent, const char* format, ...);
void debug_tree_destory(struct debug_tree* tree);
void debug_tree_print(struct debug_tree* tree);
#endif

