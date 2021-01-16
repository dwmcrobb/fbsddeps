//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2016, 2021
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The names of the authors and copyright holders may not be used to
//     endorse or promote products derived from this software without
//     specific prior written permission.
//
//  IN NO EVENT SHALL DANIEL W. MCROBB BE LIABLE TO ANY PARTY FOR
//  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
//  INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF DANIEL W. MCROBB HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
//  THE SOFTWARE PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND
//  DANIEL W. MCROBB HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
//  UPDATES, ENHANCEMENTS, OR MODIFICATIONS. DANIEL W. MCROBB MAKES NO
//  REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER
//  IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
//  OR THAT THE USE OF THIS SOFTWARE WILL NOT INFRINGE ANY PATENT,
//  TRADEMARK OR OTHER RIGHTS.
//===========================================================================

//---------------------------------------------------------------------------
//!  \file fbsddeps.cc
//!  \brief trivial program to get package dependencies for a binary that
//!  is linked to shared libraries
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/types.h>
  #include <fcntl.h>
  #include <fts.h>
  #include <unistd.h>
  #include <sqlite3.h>
}
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <set>
#include <string>

using namespace std;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void GetSharedLibs(const string & filename, set<string> & libs,
                          set<string> & libsNotFound)
{
  string  lddcmd("ldd " + filename);
  FILE    *lddpipe = popen(lddcmd.c_str(), "r");
  if (lddpipe) {
    regex   rgx("[ \\t]+([^ \\t]+)[ \\t]+[=][>][ \\t]+(\\/[^ \\t]+|not found)[ \\t]*",
                regex::ECMAScript|regex::optimize);
    smatch  sm;
    char    line[4096];
    while (fgets(line, 4096, lddpipe) != NULL) {
      string  s(line);
      if (regex_search(s, sm, rgx)) {
        if (sm.size() == 3) {
          string  lib(sm[2].str());
          if (lib == "not found") {
            libsNotFound.insert(sm[1].str());
          }
          else {
            libs.insert(lib);
          }
        }
      }
    }
    pclose(lddpipe);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
void GetPackageDeps(const set<string> & libs, set<string> & packages)
{
  string  queryPreamble("select packages.name, packages.version, packages.id,"
                        " files.package_id, files.path"
                        " from packages, files where"
                        " packages.id = files.package_id"
                        " and files.path = ");
  sqlite3  *ppdb;
  if (sqlite3_open_v2("/var/db/pkg/local.sqlite", &ppdb,
                      SQLITE_OPEN_READONLY, 0)
      == SQLITE_OK) {
    for (auto lib : libs) {
      string  qrystr(queryPreamble + '"' + lib + '"');
      sqlite3_stmt *ppStmt;
      if (sqlite3_prepare(ppdb, qrystr.c_str(), -1, &ppStmt, 0) == SQLITE_OK) {
        while (sqlite3_step(ppStmt) == SQLITE_ROW) {
          string  pkgName((const char *)sqlite3_column_text(ppStmt, 0));
          pkgName += '-';
          pkgName += (const char *)sqlite3_column_text(ppStmt, 1);
          packages.insert(pkgName);
        }
      }
      sqlite3_finalize(ppStmt);
    }
    sqlite3_close_v2(ppdb);
  }
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  cerr << "usage: " << argv0 << " file(s)...\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  if (argc < 2) {
    Usage(argv[0]);
    return 1;
  }

  for (int arg = 1; arg < argc; ++arg) {
    set<string>  sharedLibs, libsNotFound;
    GetSharedLibs(argv[arg], sharedLibs, libsNotFound);
    if (! sharedLibs.empty()) {
      set<string>  packages;
      GetPackageDeps(sharedLibs, packages);
      if (! packages.empty()) {
        cout << argv[arg] << ":\n";
        for (auto pkg : packages) {
          cout << "  " << pkg << '\n';
        }
      }
      if (! libsNotFound.empty()) {
        cout << "\n  libs not found:\n";
        for (auto lib : libsNotFound) {
          cout << "    " << lib << '\n';
        }
      }
    }
  }
  return 0;
}

