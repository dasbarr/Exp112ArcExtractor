#ifndef ARC_EXTRACTOR_UNPACKER_THREAD_H
#define ARC_EXTRACTOR_UNPACKER_THREAD_H

#include <string>
#include <iostream>
#include <fstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"

#include "DataStructures.h"
#include "Utils.h"

namespace arcextractor
{

template <typename T>
void extractAndWriteFiles(ArchiveData<T>& archiveData, UnpackStepState& extractionState)
{
  uint32_t numFiles = archiveData.numFiles;
  uint32_t numProcessedFiles = 0;
  for (uint32_t i = 0; i < numFiles; i++)
  {
    FileDescriptionEntry<T> currentFileDescription = archiveData.fileDescriptionEntries[i];
    char *fileData = archiveData.rawFilesData + currentFileDescription.fileDataOffset;

    //Determine encrypted part size. Non-grouped part at the end of current file
    //isn't encrypted.
    uint32_t encryptedPartSize = currentFileDescription.fileSize / sizeof(uint32_t);

    //decrypt current file
    xorDecrypt(reinterpret_cast<uint32_t*>(fileData), encryptedPartSize, 
               currentFileDescription.xorKey);

    //create directories for current file
    std::wstring* convertedFilePath = filePathToWstring(currentFileDescription.filePath);
    createDirectories(convertedFilePath);
    delete convertedFilePath;

    //write unpacked file
    std::ofstream unpackedFile;
    unpackedFile.open(currentFileDescription.filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (unpackedFile.fail())
    {
      //end unpacking immediately and indicate an error
      extractionState.isError = true;
      return;
    }
    else
    {
      unpackedFile.write(fileData, currentFileDescription.fileSize);
      unpackedFile.flush();
    }
    unpackedFile.close();

    numProcessedFiles++;
    extractionState.percent = static_cast<uint8_t>(static_cast<double>(numProcessedFiles) / numFiles * 100);
  }

  extractionState.isComplete = true;
}

template <typename T>
DWORD WINAPI unpackerThread(LPVOID threadParameter)
{
  ThreadData<T>* threadData = static_cast<ThreadData<T>*>(threadParameter);
  ArchiveData<T>& archiveData = *(threadData->archiveData);
  ArchiveUnpackState& archiveUnpackState = threadData->archiveUnpackState;

  //decrypt file description table
  UnpackStepState& decryptionState = archiveUnpackState.decryptionState;
  xorDecrypt(reinterpret_cast<uint32_t*>(archiveData.fileDescriptionEntries),
             archiveData.fileDescriptionTableSize / sizeof(uint32_t),
             archiveData.globalXorKey,
             decryptionState.percent);

  //assume that nothing can go wrong during decrypting =)
  decryptionState.isComplete = true;

  //process files in archive
  UnpackStepState& extractionState = archiveUnpackState.extractionState;
  extractAndWriteFiles(archiveData, extractionState);
  
  if (extractionState.isError)
    return 1;

  return 0;
}

} // namespace arcextractor

#endif // ARC_EXTRACTOR_UNPACKER_THREAD_H
