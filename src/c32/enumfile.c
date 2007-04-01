/* Copyright 2007 TeX Users Group

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */

#include "enumfile.h"
#include <string.h>

static int IncludeDirs = TRUE;

static int Recurse(HANDLE CurFile, void* FuncData,
                   ENUMFILECALLBACK Callback,
                   WIN32_FIND_DATA* CurData, char* Path)
    {
    int             Status = 1;

    while(CurFile != INVALID_HANDLE_VALUE)
        {
        int     IsDir, SkipDir;
        char*   FileName    = CurData->cFileName;
        char*   EndDir      = Path + strlen(Path);

        IsDir = CurData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        SkipDir = !strcmp(FileName, "..") || !strcmp(FileName, ".");
        
        if(SkipDir || (IsDir && !IncludeDirs))
            Status  = 1;
        else
            {
            strcat(Path, CurData->cFileName);
            Status = Callback(FuncData, Path, CurData);
            *EndDir = '\0';
            }

        if(IsDir && !SkipDir && Status > 0)
            {
            HANDLE  CurFile;
            char*   NewEnd;

            strcat(Path, CurData->cFileName);
            NewEnd  = Path + strlen(Path);
            strcat(Path, "\\*.*");
            CurFile =  FindFirstFile(Path, CurData);
            NewEnd[1] = '\0';
            Status = Recurse(CurFile, FuncData, Callback,
                CurData, Path);
            *EndDir = '\0';
            }
        if(Status < 0)
            break;
        else if(!FindNextFile(CurFile, CurData))
            break;
        }

    if(CurFile != INVALID_HANDLE_VALUE)
        FindClose(CurFile);

    return Status;
    }


int EnumFile(const char* Pattern, void* FuncData,
                 ENUMFILECALLBACK Callback)
    {
    HANDLE          CurFile;
    WIN32_FIND_DATA CurData;
    char            Path[MAX_PATH+1];
    char*           EndDir;

    strcpy(Path, Pattern);
    EndDir  = strrchr(Path, '\\');
    /* could have used '/' instead of '\' */
    if(strrchr(Path, '/') > EndDir)
        EndDir  = strrchr(Path, '/');
    if(strrchr(Path, ':') > EndDir)
        EndDir  = strrchr(Path, ':');

    if(EndDir)
        {
        if(!strcmp(EndDir, ".") || !strcmp(EndDir, ".."))
            {
            strcat(Path, "\\");
            EndDir  += strlen(EndDir)-1;
            }
        strcat(Path, "*.*");
        ++EndDir;
        Pattern = Path;
        }
    else
        EndDir  = Path;

    if(!strcmp(EndDir, ".") || !strcmp(EndDir, ".."))
        {
        EndDir  += strlen(EndDir) + 1;
        strcat(Path, "\\*.*");
        Pattern = Path;
        }

    CurFile = FindFirstFile(Pattern, &CurData);
    EndDir[0]       = '\0';

    return Recurse(CurFile, FuncData, Callback, &CurData, Path);
    }

/*	typedef int (*ENUMFILECALLBACK)(void* Data, LPCSTR RelPath,
									WIN32_FIND_DATA* Found); */

/*	int EnumFile(LPCSTR Pattern, void* Data, ENUMFILECALLBACK Callback); */

/* Your callback function should return an integer value:
 *
 * >0   - to continue scanning for more files.
 *
 * =0   - to not descend the current directory.
 *
 * <0   - to terminate the enumeration and return to the caller.
 */


/* End of File */
