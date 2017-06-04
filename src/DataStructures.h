#ifndef ARC_EXTRACTOR_DATA_STRUCTURES_H
#define ARC_EXTRACTOR_DATA_STRUCTURES_H

#include <fstream>
#include <memory>

namespace arcextractor
{

template <typename T>
struct FileDescriptionEntry
{
  //Max file name field size is always 128 symbols
  //(including 0 symbol at the end).
  T fileName[128];
  //Max file path field size is always 256 symbols 
  //(including 0 symbol at the end).
  T filePath[256];
  //file data offset from the archive's beginning
  uint32_t fileDataOffset;
  //file size in bytes
  uint32_t fileSize;
  //each file is encrypted with their own xor cipher key
  uint32_t xorKey;
};

template <typename T>
class ArchiveData
{
public:
  //raw data includes header (to keep offsets valid) and encrypted files
  char* rawFilesData;
  uint32_t numFiles;
  uint32_t fileDescriptionTableSize;
  //file description table is encrypted with global xor cipher key
  uint32_t globalXorKey;
  FileDescriptionEntry<T>* fileDescriptionEntries;

  ArchiveData(std::ifstream& archive)
  {
    //read number of files in archive (0x8 offset)
    archive.seekg(0x8);
    archive.read(reinterpret_cast<char*>(&numFiles), sizeof(numFiles));

    //Read file description table offset (0xC offset). Seek isn't necessary 
    //because that value is right after number of files.
    uint32_t fileDescriptionTableOffset;
    archive.read(reinterpret_cast<char*>(&fileDescriptionTableOffset),
      sizeof(fileDescriptionTableOffset));

    //read global XOR cipher key (4 bytes at the end of archive)
    size_t globalXorKeySize = sizeof(globalXorKey);
    archive.seekg(-static_cast<int>(globalXorKeySize), archive.end);
    archive.read(reinterpret_cast<char*>(&globalXorKey), globalXorKeySize);

    //Read header and files data. Size equals to file description table offset.
    uint32_t rawFilesDataSize = fileDescriptionTableOffset;
    rawFilesData = new char[rawFilesDataSize];
    archive.seekg(0);
    archive.read(rawFilesData, fileDescriptionTableOffset);

    //determine file description table size (in bytes)
    archive.seekg(0);
    archive.ignore(std::numeric_limits<std::streamsize>::max());
    //Archive size in bytes. Assume that game archives are smaller than 4 GB
    //(to fit uint32_t).
    uint32_t archiveSize = static_cast<uint32_t>(archive.gcount());
    //don't forget that 4 bytes at the end of archive - global XOR key
    fileDescriptionTableSize = archiveSize - fileDescriptionTableOffset -
      sizeof(globalXorKey);

    //read encrypted directory description table
    fileDescriptionEntries = new FileDescriptionEntry<T>[numFiles];
    archive.seekg(fileDescriptionTableOffset);
    archive.read(reinterpret_cast<char*>(fileDescriptionEntries),
      fileDescriptionTableSize);
  }
  ~ArchiveData()
  {
    delete[] rawFilesData;
    delete[] fileDescriptionEntries;
  }
};

struct UnpackStepState
{
  uint8_t percent = 0;
  bool isComplete = false;
  bool isError = false;
};

struct ArchiveUnpackState
{
  UnpackStepState decryptionState;
  UnpackStepState extractionState;
};

template <typename T>
struct ThreadData
{
  ArchiveUnpackState archiveUnpackState;
  std::shared_ptr<ArchiveData<T>> archiveData;
};

} // namespace arcextractor

#endif // ARC_EXTRACTOR_DATA_STRUCTURES_H
