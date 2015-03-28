#ifndef FIELD_TYPE_H
#define FIELD_TYPE_H

struct field_type_db* field_type_db_load(const char* file);
void field_type_db_destory(struct field_type_db* field_type_db);
void field_type_db_print(struct field_type_db* field_type_db, struct debug_tree* root);

#endif
