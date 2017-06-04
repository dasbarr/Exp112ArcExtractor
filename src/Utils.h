#ifndef ARC_EXTRACTOR_UTILS_H
#define ARC_EXTRACTOR_UTILS_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"

#include "DataStructures.h"
#include "UnpackerThread.h"

namespace arcextractor
{

//Returns number of bytes per symbol in strings inside archive (file names and
//file paths).
uint8_t numBytesPerSymbolInArchiveStrings(std::ifstream& archive);

void xorDecrypt(uint32_t* encryptedData, uint32_t dataLength, uint32_t key);
//xor decryption with progress indication
void xorDecrypt(uint32_t* encryptedData, uint32_t dataLength, uint32_t key, uint8_t& percent);

std::wstring* filePathToWstring(char* filePath);
std::wstring* filePathToWstring(wchar_t* filePath);

//creates directory hierarchy by file path specified
void createDirectories(std::wstring* filePath);

void indicateProgress(ArchiveUnpackState& archiveUnpackState);

//splits wstring by delimiter
template <typename T>
void split(const std::wstring* ws, wchar_t delimiter, T result)
{
  std::wstringstream wstrStream;
  wstrStream.str(*ws);
  std::wstring item;
  while (getline(wstrStream, item, delimiter))
  {
    *(result++) = item;
  }
}

//unpack archive using another thread and indicate unpacking progress
template <typename T>
void unpackArchive(std::ifstream& archive)
{
  std::shared_ptr<ArchiveData<T>> archiveData = std::make_shared<ArchiveData<T>>(archive);
  ThreadData<T> threadData;
  threadData.archiveData = archiveData;

  DWORD unpackerThreadID;
  HANDLE unpackerThreadHandle = CreateThread(0, 0, unpackerThread<T>, &threadData, 0, &unpackerThreadID);

  std::wcout << std::endl;
  indicateProgress(threadData.archiveUnpackState);

  CloseHandle(unpackerThreadHandle);
}

} // namespace arcextractor

#endif // ARC_EXTRACTOR_UTILS_H
