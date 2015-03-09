//
//  main.c
//  AssetBundle
//
//  Created by dpull on 15/1/15.
//  Copyright (c) 2015年 dpull. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "assetbundle.h"
#include "assetbundle_diff.h"
#include "assetfile.h"
#include "filemaping.h"

struct file_asset
{
    size_t max_count;
    size_t count;
    char** names;
    struct assetfile** assets;
    struct filemaping** files;
};

bool load_assetfile(const char* fullpath, const char* filename, void* userdata)
{
    struct file_asset* file_asset = (struct file_asset*)userdata;
    if (file_asset->count == file_asset->max_count) {
        file_asset->max_count = 2 * ((file_asset->max_count == 0) ? 100 : file_asset->max_count);
        file_asset->names = (char**)realloc(file_asset->names, file_asset->max_count * sizeof(file_asset->names[0]));
        file_asset->assets = (struct assetfile**)realloc(file_asset->assets, file_asset->max_count * sizeof(file_asset->assets[0]));
        file_asset->files = (struct filemaping**)realloc(file_asset->files, file_asset->max_count * sizeof(file_asset->files[0]));
    }
    
    struct filemaping* filemaping = filemaping_create_readonly(fullpath);
    if (!filemaping)
        return true;
    
    unsigned char* data = filemaping_getdata(filemaping);
    size_t length = filemaping_getlength(filemaping);
    
    struct assetfile* asset = assetfile_load(data, 0, length);
    if (!asset) {
        filemaping_destory(filemaping);
        return true;
    }
    
    file_asset->names[file_asset->count] = strdup(fullpath);
    file_asset->assets[file_asset->count] = asset;
    file_asset->files[file_asset->count] = filemaping;
    file_asset->count++;
    return true;
}

int main(int argc, const char * argv[])
{
    size_t a = sizeof("unity");
    
    struct file_asset* file_asset = (struct file_asset*)malloc(sizeof(*file_asset));
    memset(file_asset, 0, sizeof(*file_asset));
    
    // traversedir("/Users/dpull/Documents/AssetBundle/Untitled", load_assetfile, file_asset, true);
    
    
    FILE* debug = fopen("/Users/dpull/Documents/AssetBundle/debug.txt", "w+");
    
    struct assetbundle* p1 = assetbundle_load("/Users/dpull/Documents/AssetBundle/update.asset");
    assert(p1);

    //assetbundle_diff_1(file_asset->assets, file_asset->count, p1, "/Users/dpull/Documents/AssetBundle/diff.asset ");
    
   
    struct debug_tree* root = debug_tree_create(NULL, "*");
    assetbundle_print(p1, root);

    debug_tree_print(root, debug);
    
    
/*
    int ret;
 
    ret = assetbundle_diff(
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled1.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled2.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\diff.asset"
    );
    assert(ret == 0);
    
    ret = assetbundle_merge(
		"D:\\OneDrive\\Code\\assetbundleparser\\Untitled1.asset",
		"D:\\OneDrive\\Code\\assetbundleparser\\Untitled3.asset",
		"D:\\OneDrive\\Code\\assetbundleparser\\diff.asset"
    );
    assert(ret == 0);
*/
    
    return 0;
}
