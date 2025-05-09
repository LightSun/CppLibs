// //////////////////////////////////////////////////////////
// MemoryMapped.cpp
// Copyright (c) 2013 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#include "MemoryMapped.h"

#include <stdexcept>
#include <cstdio>

// OS-specific
#ifdef _WIN32
// Windows
#include <windows.h>
#else
// Linux
// enable large file support on 32 bit systems
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifdef  _FILE_OFFSET_BITS
#undef  _FILE_OFFSET_BITS
#endif
#define _FILE_OFFSET_BITS 64
// and include needed headers
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

using namespace h7;

/// do nothing, must use open()
MemoryMapped::MemoryMapped(int op)
: _filename   (),
  _filesize   (0),
  _hint       (Normal),
  _mappedBytes(0),
  _file       (0),
#ifdef _WIN32
  _mappedFile (NULL),
#endif
  _mappedView (NULL),
  _op(op)
{
}


/// open file, mappedBytes = 0 maps the whole file
MemoryMapped::MemoryMapped(const std::string& filename, size_t mappedBytes, CacheHint hint)
: _filename   (filename),
  _filesize   (0),
  _hint       (hint),
  _mappedBytes(mappedBytes),
  _file       (0),
#ifdef _WIN32
  _mappedFile (NULL),
#endif
  _mappedView (NULL)
{
  open(filename, mappedBytes, hint);
}


/// close file (see close() )
MemoryMapped::~MemoryMapped()
{
  close();
}


/// open file
bool MemoryMapped::open(const std::string& filename, size_t mappedBytes, CacheHint hint)
{
  // already open ?
  if (isValid())
    return false;

  _file       = 0;
  _filesize   = 0;
  _hint       = hint;
#ifdef _WIN32
  _mappedFile = NULL;
#endif
  _mappedView = NULL;

#ifdef _WIN32
  // Windows

  DWORD winHint = 0;
  switch (_hint)
  {
  case Normal:         winHint = FILE_ATTRIBUTE_NORMAL;     break;
  case SequentialScan: winHint = FILE_FLAG_SEQUENTIAL_SCAN; break;
  case RandomAccess:   winHint = FILE_FLAG_RANDOM_ACCESS;   break;
  default: break;
  }

  // open file
  switch (_op) {
  default:
  case kOP_READ:{
      _file = ::CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, winHint, NULL);
  }break;
  case kOP_WRITE:{
      _file = ::CreateFileA(filename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, winHint, NULL);
  }break;
  case kOP_READ_WRITE:{
      _file = ::CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, winHint, NULL);
  }break;
  }
  if (!_file)
    return false;

  // file size
  LARGE_INTEGER result;
  if (!GetFileSizeEx(_file, &result))
    return false;
  _filesize = static_cast<uint64_t>(result.QuadPart);

  // convert to mapped mode
  switch (_op) {
  default:
  case kOP_READ:{
      _mappedFile = ::CreateFileMapping(_file, NULL, PAGE_READONLY, 0, 0, NULL);
  }break;
  case kOP_WRITE:{
      _mappedFile = ::CreateFileMapping(_file, NULL, PAGE_READWRITE, 0, 0, NULL);
  }break;
  case kOP_READ_WRITE:{
      _mappedFile = ::CreateFileMapping(_file, NULL, PAGE_READWRITE, 0, 0, NULL);
  }break;
  }
  if (!_mappedFile)
    return false;

#else

  // Linux

  // open file
  switch (_op) {
  default:
  case kOP_READ:{
       _file = ::open(filename.c_str(), O_RDONLY | O_LARGEFILE);
  }break;
  case kOP_WRITE:{
       _file = ::open(filename.c_str(), O_WRONLY | O_LARGEFILE);
  }break;
  case kOP_READ_WRITE:{
       _file = ::open(filename.c_str(), O_RDWR | O_LARGEFILE);
  }break;
  }
  if (_file == -1)
  {
    _file = 0;
    return false;
  }

  // file size
  struct stat64 statInfo;
  if (fstat64(_file, &statInfo) < 0)
    return false;

  _filesize = statInfo.st_size;
#endif

  // initial mapping
  remap(0, mappedBytes);

  if (!_mappedView)
    return false;

  // everything's fine
  return true;
}


/// close file
void MemoryMapped::close()
{
  // kill pointer
  if (_mappedView)
  {
#ifdef _WIN32
    ::UnmapViewOfFile(_mappedView);
#else
    ::munmap(_mappedView, _filesize);
#endif
    _mappedView = NULL;
  }

#ifdef _WIN32
  if (_mappedFile)
  {
    ::CloseHandle(_mappedFile);
    _mappedFile = NULL;
  }
#endif

  // close underlying file
  if (_file)
  {
#ifdef _WIN32
    ::CloseHandle(_file);
#else
    ::close(_file);
#endif
    _file = 0;
  }

  _filesize = 0;
}


/// access position, no range checking (faster)
unsigned char MemoryMapped::operator[](size_t offset) const
{
  return ((unsigned char*)_mappedView)[offset];
}


/// access position, including range checking
unsigned char MemoryMapped::at(size_t offset) const
{
  // checks
  if (!_mappedView)
    throw std::invalid_argument("No view mapped");
  if (offset >= _filesize)
    throw std::out_of_range("View is not large enough");

  return operator[](offset);
}


/// raw access
const char* MemoryMapped::getData() const
{
  return (const char*)_mappedView;
}


/// true, if file successfully opened
bool MemoryMapped::isValid() const
{
  return _mappedView != NULL;
}


/// get file size
uint64_t MemoryMapped::size() const
{
  return _filesize;
}


/// get number of actually mapped bytes
size_t MemoryMapped::mappedSize() const
{
  return _mappedBytes;
}


/// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
bool MemoryMapped::remap(uint64_t offset, size_t mappedBytes)
{
  if (!_file)
    return false;

  if (mappedBytes == WholeFile)
    mappedBytes = _filesize;

  // close old mapping
  if (_mappedView)
  {
#ifdef _WIN32
    ::UnmapViewOfFile(_mappedView);
#else
    ::munmap(_mappedView, _mappedBytes);
#endif
    _mappedView = NULL;
  }

  // don't go further than end of file
  if (offset > _filesize)
    return false;
  if (offset + mappedBytes > _filesize)
    mappedBytes = size_t(_filesize - offset);

#ifdef _WIN32
  // Windows

  DWORD offsetLow  = DWORD(offset & 0xFFFFFFFF);
  DWORD offsetHigh = DWORD(offset >> 32);
  _mappedBytes = mappedBytes;

  // get memory address
  switch (_op) {
  default:
  case kOP_READ:{
      _mappedView = ::MapViewOfFile(_mappedFile, FILE_MAP_READ,
                                    offsetHigh, offsetLow, mappedBytes);
  }break;

  case kOP_WRITE:{
      _mappedView = ::MapViewOfFile(_mappedFile, FILE_MAP_WRITE,
                                    offsetHigh, offsetLow, mappedBytes);

  }break;

  case kOP_READ_WRITE:{
      _mappedView = ::MapViewOfFile(_mappedFile, FILE_MAP_ALL_ACCESS,
                                    offsetHigh, offsetLow, mappedBytes);
  }break;
  }
  if (_mappedView == NULL)
  {
    _mappedBytes = 0;
    _mappedView  = NULL;
    return false;
  }

  return true;

#else

  // Linux
  // new mapping
  switch (_op) {
  default:
  case kOP_READ:{
       _mappedView = ::mmap64(NULL, mappedBytes, PROT_READ, MAP_SHARED, _file, offset);
  }break;
  case kOP_WRITE:{
       _mappedView = ::mmap64(NULL, mappedBytes, PROT_WRITE, MAP_SHARED, _file, offset);
  }break;
  case kOP_READ_WRITE:{
       _mappedView = ::mmap64(NULL, mappedBytes, PROT_READ | PROT_WRITE, MAP_SHARED,
                              _file, offset);
  }break;
  }
  if (_mappedView == MAP_FAILED)
  {
    _mappedBytes = 0;
    _mappedView  = NULL;
    return false;
  }

  _mappedBytes = mappedBytes;

  // tweak performance
  int linuxHint = 0;
  switch (_hint)
  {
  case Normal:         linuxHint = MADV_NORMAL;     break;
  case SequentialScan: linuxHint = MADV_SEQUENTIAL; break;
  case RandomAccess:   linuxHint = MADV_RANDOM;     break;
  default: break;
  }
  // assume that file will be accessed soon
  //linuxHint |= MADV_WILLNEED;
  // assume that file will be large
  //linuxHint |= MADV_HUGEPAGE;

  ::madvise(_mappedView, _mappedBytes, linuxHint);

  return true;
#endif
}


/// get OS page size (for remap)
int MemoryMapped::getpagesize()
{
#ifdef _WIN32
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return sysInfo.dwAllocationGranularity;
#else
  return sysconf(_SC_PAGESIZE); //::getpagesize();
#endif
}
