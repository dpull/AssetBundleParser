#ifndef ASSETBUNDLE_DIFF_H
#define ASSETBUNDLE_DIFF_H

enum ASSETBUNDLE_RETCODE
{
	ASSETBUNDLE_FAILED = -1,
	ASSETBUNDLE_SUCCEED = 0,

	ASSETBUNDLE_FROM_LOAD_FAILED,
	ASSETBUNDLE_DIFF_CREATE_FAILED,
	ASSETBUNDLE_DIFF_LOAD_FAILED,
    ASSETBUNDLE_TO_CREATE_FAILED,
};

API_EXTERN int assetbundle_diff(const char* dir, const char* from, const char* to, const char* diff);
API_EXTERN void assetbundle_diff_print(const char* filename, const char* output);

typedef bool (API_CALLBACK readfile_callback)(unsigned char* buffer, const char* filename, size_t offset, size_t length, void* userdata);
API_EXTERN int assetbundle_merge(readfile_callback* fn_readfile, void* userdata, const char* from, const char* to, const char* diff);

#endif
