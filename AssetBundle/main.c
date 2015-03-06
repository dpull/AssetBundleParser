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

int main(int argc, const char * argv[]) {   
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
    
    
    return 0;
}
