#ifndef ASSETFILE_DIFF_H
#define ASSETFILE_DIFF_H

struct assetfile_diff* assetfile_diff(struct assetfile** fromfiles, size_t fromfiles_count, struct assetfile* tofile);
void assetfile_diff_destory(struct assetfile_diff* diff);

size_t assetfile_diff_loadfile(struct assetfile** retfile, unsigned char* data, size_t offset);
size_t assetfile_diff_savefile(struct assetfile* file, unsigned char* data, size_t offset);

size_t assetfile_diff_loaddiff(struct assetfile_diff** retdiff, unsigned char* data, size_t offset);
size_t assetfile_diff_savediff(struct assetfile_diff* diff, unsigned char* data, size_t offset);

bool assetfile_diff_merge(struct assetfile** fromfiles, size_t fromfiles_count, struct assetfile* tofile, struct assetfile_diff* diff, unsigned char* data, size_t start, size_t size);

void assetfile_diff_print(struct assetfile_diff* diff, struct debug_tree* root);

#endif
