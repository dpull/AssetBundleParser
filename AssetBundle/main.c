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
#include "filemaping.h"
#include "field_type.h"
#include "traversedir.h"

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
    
    struct filemaping* filemaping = NULL;
    const char* split0 = strstr(fullpath, ".split0");
    if (split0 == NULL) {
        filemaping = filemaping_create_readonly(fullpath);
        if (!filemaping)
            return true;
    } else {
        char asset_path[512];
        memset(asset_path, 0, 512);
        strncpy(asset_path, fullpath, split0 - fullpath);
        
        char temp_file[512];
        strcpy(temp_file, "/tmp/temp.XXXXXX");
        mktemp(temp_file);
        filemaping = filemaping_create_readwrite(temp_file, 1024 * 1024 * 100);
        unsigned char* data = filemaping_getdata(filemaping);
        size_t offset = 0;
        
        for (int i = 0; i < 100; ++i) {
            char split_asset_path[512];
            sprintf(split_asset_path, "%s.split%d", asset_path, i);
            struct filemaping* split_filemaping = filemaping_create_readonly(split_asset_path);
            if (!split_filemaping)
                break;
            
            unsigned char* split_data = filemaping_getdata(split_filemaping);
            size_t split_length = filemaping_getlength(split_filemaping);
            memcpy(data + offset, split_data, split_length);
            offset += split_length;
            
            filemaping_destory(split_filemaping);
        }
    }
    
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
     
    struct field_type_db* db = field_type_db_load("/Users/dpull/Documents/AssetBundle/types.dat");
    struct debug_tree* root = debug_tree_create(NULL, "*");
    field_type_db_print(db, root);
    // debug_tree_print(root, stdout);
    field_type_db_destory(db);

    struct file_asset* file_asset = (struct file_asset*)malloc(sizeof(*file_asset));
    memset(file_asset, 0, sizeof(*file_asset));
    
    traversedir("/Users/dpull/Documents/AssetBundle/Untitled", load_assetfile, file_asset, true);
    
    struct assetbundle* p1 = assetbundle_load("/Users/dpull/Documents/AssetBundle/Untitled.asset");
    assert(p1);

    assetbundle_diff_1(file_asset->assets, file_asset->count, p1, "/Users/dpull/Documents/AssetBundle/diff.asset ");
    
   
    root = debug_tree_create(NULL, "*");
    assetbundle_print(p1, root);

    FILE* debug1 = fopen("/Users/dpull/Documents/AssetBundle/debug1.txt", "w+");
    debug_tree_print(root, debug1);

    debug_tree_destory(root);


    root = debug_tree_create(NULL, "*");

    for (int i = 0; i < file_asset->count; ++i)
    {
        struct debug_tree* file = debug_tree_create(root, "%s", file_asset->names[i]);
        assetfile_print(file_asset->assets[i], file);
    }
    
    FILE* debug2 = fopen("/Users/dpull/Documents/AssetBundle/debug2.txt", "w+");
    debug_tree_print(root, debug2);
    debug_tree_destory(root);


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
