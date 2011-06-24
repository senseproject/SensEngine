// Copyright 2011 Branan Purvine-Riley and Adam Johnson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SENSE_UTIL_UTIL_HPP
#define SENSE_UTIL_UTIL_HPP

// Please don't put anything in here unless it really is a utility macro/inline.
// I'm serious.

// Make automatic exceptions easier to debug
#define STRINGIFICATION(X) #X
#define STRINGIFICATE(X) STRINGIFICATION(X)
#define FILE_LINE " @ " __FILE__ ":" STRINGIFICATE(__LINE__)

// Selectively disable warnings on GCC
#ifdef __GNUC__
#define GCC_DO_PRAGMA(prag) _Pragma(#prag)
#if __GNUC_MINOR__ >= 6
#define GCC_DIAGNOSTIC_PUSH  GCC_DO_PRAGMA(GCC diagnostic push)
#define GCC_DIAGNOSTIC_POP   GCC_DO_PRAGMA(GCC diagnostic pop)
#else
#define GCC_DIAGNOSTIC_PUSH
#define GCC_DIAGNOSTIC_POP
#endif
#define GCC_DISABLE_WARNING(warn) GCC_DO_PRAGMA(GCC diagnostic ignored warn)
#define MSC_DIAGNOSTIC_PUSH
#define MSC_DIAGNOSTIC_POP
#define MSC_DISABLE_WARNING(warn)
#endif

// Selectively disable warnings on MSC
#ifdef _MSC_VER
#define MSC_DIAGNOSTIC_PUSH __pragma warning(push)
#define MSC_DIAGNOSTIC_POP  __pragma warning(pop)
#define MSC_DISABLE_WARNING(warn) __pragma warning(disable: warn)
#define GCC_DIAGNOSTIC_PUSH
#define GCC_DIAGNOSTIC_POP
#define GCC_DISABLE_WARNING(warn)
#endif

// use _wfopen on win32 and fopen on Unix
#ifdef _WIN32
#define openFile _wfopen
#define openFile_R L"r"
#define openFile_RB L"rb"
#define openFile_W L"w"
#define openFile_WB L"wb"
#define openfile_WR L"wr"
#define openFile_WRB L"wrb"
#else _WIN32
#define openFile fopen
#define openFile_R "r"
#define openFile_RB "rb"
#define openFile_W "w"
#define openFile_WB "wb"
#define openfile_WR "wr"
#define openFile_WRB "wrb"
#endif // _WIN32
#endif // SENSE_UTIL_UTIL_HPP
