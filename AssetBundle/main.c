//
//  main.c
//  AssetBundle
//
//  Created by dpull on 15/1/15.
//  Copyright (c) 2015年 dpull. All rights reserved.
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "assetbundle.h"
#include "assetbundle_diff.h"
#include "assetfile.h"
#include "filemapping.h"
#include "field_type.h"
#include "utils/traversedir.h"
#include "assetbundle_diff.h"

#define DIR "/Users/dpull/Documents/AssetBundle/"

bool readfile_in_dir(unsigned char* buffer, const char* filename, size_t offset, size_t length, void* userdata)
{
	char* dir = (char*)userdata;
	char filename_buffer[512];
	snprintf(filename_buffer, sizeof(filename_buffer), "%s/%s", dir, filename);

	struct filemapping* filemapping = filemapping_create_readonly(filename_buffer);
	if (!filemapping)
		return false;
	size_t file_length = filemapping_getlength(filemapping);
	if (file_length < offset + length) {
		filemapping_destory(filemapping);
		return false;
	}

	unsigned char* data = filemapping_getdata(filemapping);
	memcpy(buffer, data + offset, length);
	filemapping_destory(filemapping);
	return true;
}

int main(int argc, const char * argv[])
{
	int ret;

	ret = assetbundle_diff(DIR"Untitled", NULL, DIR"Untitled.asset", DIR"diff.asset");
	assert(ret == 0);

	// ret = assetbundle_merge(readfile_in_dir, DIR"Untitled/Data", DIR"Untitled.asset", DIR"Untitled_new.asset", DIR"diff.asset");
	// assert(ret == 0);

	assetbundle_diff_print(DIR"diff.asset", DIR"log.txt");


	return 0;
}
