#ifndef ASSETBUNDLE_DIFF_H
#define ASSETBUNDLE_DIFF_H

enum ASSETBUNDLE_RETCODE
{
	ASSETBUNDLE_FAILED = -1, 
	ASSETBUNDLE_SUCCEED = 0, 

	ASSETBUNDLE_DIFF_CREATE_FAILED, 
	ASSETBUNDLE_DIFF_SAVE_FAILED, 
	
	ASSETBUNDLE_FROM_LOAD_FAILED, 
	ASSETBUNDLE_TO_LOAD_FAILED, 
	ASSETBUNDLE_DIFF_LOAD_FAILED, 

	ASSETBUNDLE_FROM_CHECK_FAILED, 
	ASSETBUNDLE_TO_CHECK_FAILED, 
};

EXTERN_API errno_t assetbundle_diff(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff);
EXTERN_API errno_t assetbundle_merge(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff);

#endif