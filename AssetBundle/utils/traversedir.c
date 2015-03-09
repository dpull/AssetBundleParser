// 2010 @kingsoft.com by zhaochunfeng
// 2015 @dpull.com by acai

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "platform.h"
#include "traversedir.h"

static void PathCombine(char szOutPath[], size_t uLength, const char szPath1[], const char szPath2[])
{
    char* pszPos = szOutPath;
    char* pszEnd = szOutPath + uLength;
    
    while (pszPos < pszEnd && *szPath1)
    {
        *pszPos = *szPath1;
        pszPos++;
        szPath1++;
    }
    
    if (pszPos < pszEnd && pszPos != szOutPath)
    {
        *pszPos = '/';
        pszPos++;
    }
    
    while (pszPos < pszEnd && *szPath2)
    {
        *pszPos = *szPath2;
        pszPos++;
        szPath2++;
    }
    
    if (pszPos < pszEnd)
    {
        *pszPos = '\0';
        pszPos++;
    }
    
    szOutPath[uLength - 1] = '\0';
}

#ifdef _MSC_VER
#include<Windows.h>
bool TraverseDirFiles(const char szDir[], traversedir_callback* pFunc, void* pvUsrData, bool bIgnoreHideFile)
{
    bool                bResult     = false;
    bool			    bRetCode    = false;
    HANDLE              hFind       = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA    FindFileData;
    char                szPath[512];
    
    PathCombine(szPath, sizeof(szPath), szDir, "*");
    
    hFind = FindFirstFileA(wstrPath.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        goto Exit0;
    
    do
    {
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && bIgnoreHideFile)
            continue;
        
        if (FindFileData.cFileName[0] == '.' && bIgnoreHideFile)
            continue;
        
        if (strcmp(FindFileData.cFileName, ".") == 0)
            continue;
        
        if (strcmp(FindFileData.cFileName, "..") == 0)
            continue;
        
        PathCombine(szPath, sizeof(szPath), szDir, FindFileData.cFileName);
        
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            bRetCode = TraverseDirFiles(wstrPath.c_str(), pFunc, pvUsrData, bIgnoreHideFile);
            if (!bRetCode)
                goto Exit0;
        }
        else
        {
            bRetCode = (*pFunc)(szPath, FindFileData.cFileName, pvUsrData);
            if (!bRetCode)
                goto Exit0;
        }
    } while (FindNextFileA(hFind, &FindFileData));
    
    bResult = true;
Exit0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    return bResult;
}
#else
#include <dirent.h>
bool TraverseDirFiles(const char szDir[], traversedir_callback* pFunc, void* pvUsrData, bool bIgnoreHideFile)
{
    bool            bResult     = false;
    bool			bRetCode    = false;
    DIR*            pDir        = NULL;
    struct dirent*  pDirNode    = NULL;
    char            szPath[512];
    
    pDir = opendir(szDir);
    if (!pDir)
        goto Exit0;
    
    while ((pDirNode = readdir(pDir)) != NULL)
    {
        if (strcmp(pDirNode->d_name, ".") == 0)
            continue;
        
        if (strcmp(pDirNode->d_name, "..") == 0)
            continue;
        
        if (pDirNode->d_name[0] == '.' && bIgnoreHideFile)
            continue;
        
        PathCombine(szPath, sizeof(szPath), szDir, pDirNode->d_name);
        if (pDirNode->d_type == DT_DIR)
        {
            bRetCode = TraverseDirFiles(szPath, pFunc, pvUsrData, bIgnoreHideFile);
            if (!bRetCode)
                goto Exit0;
        }
        else
        {
            bRetCode = (*pFunc)(szPath, pDirNode->d_name, pvUsrData);
            if (!bRetCode)
                goto Exit0;
        }
    }
    
    bResult = true;
Exit0:
    if (pDir != NULL)
    {
        closedir(pDir);
        pDir = NULL;
    }
    return bResult;
}
#endif

void traversedir(const char dir[], traversedir_callback* callback, void* userdata, bool ignore_hidefile)
{
    char* dup = strdup(dir);
    char* pos = dup;
    
    while (*pos != '\0')
    {
        if (*pos == '\\')
            *pos = '/';
        pos++;
    }
    
    TraverseDirFiles(dup, callback, userdata, ignore_hidefile);
    
    if (dup)
        free(dup);
}
