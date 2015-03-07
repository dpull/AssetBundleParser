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

struct file_asset
{
    size_t max_count;
    size_t count;
    char** names;
    struct assetfile** assets;
};

bool load_assetfile(const char* fullpath, const char* filename, void* userdata)
{
    if (strchr(filename, '.') != NULL)
        return true;
    
    if (strcmp(filename, "PlayerConnectionConfigFile") == 0)
        return true;
    
    if (strstr(fullpath, "/lib/") != NULL)
        return true;
    
    
    
    struct file_asset* file_asset = (struct file_asset*)userdata;
    if (file_asset->count == file_asset->max_count) {
        file_asset->max_count = 2 * ((file_asset->max_count == 0) ? 100 : file_asset->max_count);
        file_asset->names = (char**)realloc(file_asset->names, file_asset->max_count);
        file_asset->assets = (struct assetfile**)realloc(file_asset->assets, file_asset->max_count);
    }
    
    struct assetfile* asset = assetfile_loadfile(fullpath);
    if (asset) {
        file_asset->names[file_asset->count] = strdup(fullpath);
        file_asset->assets[file_asset->count] = asset;
        file_asset->count++;
    }
    
    return true;
}

int main(int argc, const char * argv[])
{
    struct file_asset* file_asset = (struct file_asset*)malloc(sizeof(*file_asset));
    memset(file_asset, 0, sizeof(*file_asset));
    
    traversedir("/Users/dpull/Documents/AssetBundle/Untitled", load_assetfile, file_asset, true);
    
    
    struct assetfile* p1 = assetfile_loadfile("/Users/dpull/Documents/AssetBundle/level11");
    assert(p1);
    
    struct assetfile* p2 = assetfile_loadfile("/Users/dpull/Documents/AssetBundle/other");
    assert(p2);
    
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
