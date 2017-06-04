#include "Utils.h"

#include <fstream>
#include <string>
#include <vector>
#include <iterator>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"

#include "DataStructures.h"

namespace arcextractor
{

uint8_t numBytesPerSymbolInArchiveStrings(std::ifstream& archive)
{
  //read directory description offset (0xC offset in file)
  uint32_t dirDescOffset;
  archive.seekg(0xC);
  archive.read(reinterpret_cast<char*>(&dirDescOffset), sizeof(dirDescOffset));

  //read global XOR cipher key (4 bytes at the end of archive)
  uint32_t globalXorKey;
  size_t globalXorKeySize = sizeof(globalXorKey);
  archive.seekg(-static_cast<int>(globalXorKeySize), archive.end);
  archive.read(reinterpret_cast<char*>(&globalXorKey), globalXorKeySize);

  //Decrypt first 4 bytes of file description table (related to 1st filename) to
  //determine string format (1 byte per symbol or 2 bytes). In that archives
  //filename can't be 1 symbol long, also the first symbol should be a latin
  //character. So if 2nd byte is zero, then the string format is 2 bytes
  //per symbol, otherwise 1 byte per symbol.
  uint32_t first4Bytes;
  archive.seekg(dirDescOffset);
  archive.read(reinterpret_cast<char*>(&first4Bytes), sizeof(first4Bytes));
  first4Bytes ^= globalXorKey;
  char secondByte = (reinterpret_cast<char*>(&first4Bytes))[1];
  return (secondByte == 0) ? 2 : 1;
}

void xorDecrypt(uint32_t* encryptedData, uint32_t dataLength, uint32_t key)
{
  for (uint32_t i = 0; i < dataLength; i++)
  {
    encryptedData[i] ^= key;
  }
}
void xorDecrypt(uint32_t* encryptedData, uint32_t dataLength, uint32_t key, uint8_t& percent)
{
  uint32_t decryptedDataCounter = 0;
  for (uint32_t i = 0; i < dataLength; i++)
  {
    encryptedData[i] ^= key;
    percent = static_cast<uint8_t>(static_cast<double>(decryptedDataCounter) / dataLength * 100);
  }
}

std::wstring* filePathToWstring(char* filePath)
{
  size_t pathSize = strlen(filePath) + 1;//0 terminated
  wchar_t* wcharPath = new wchar_t[pathSize];
  size_t outSize;
  mbstowcs_s(&outSize, wcharPath, pathSize, filePath, pathSize - 1);
  std::wstring* convertedPath = new std::wstring(wcharPath);

  delete[] wcharPath;
  return convertedPath;
}
std::wstring* filePathToWstring(wchar_t* filePath)
{
  return new std::wstring(filePath);
}

void createDirectories(std::wstring* filePath)
{
  //split file path by '\' to determine folder structure
  std::vector<std::wstring> tokens;
  split(filePath, L'\\', std::back_inserter(tokens));

  std::wstring currentPath = L"";
  int32_t numTokens = static_cast<int32_t>(tokens.size());
  //last token is a file name, not directory name
  for (int32_t i = 0; i < numTokens - 1; i++)
  {
    currentPath += tokens[i];
    CreateDirectory(currentPath.c_str(), 0);
    currentPath.append(L"\\");
  }
}

void indicateProgress(ArchiveUnpackState& archiveUnpackState)
{
  //decryption
  UnpackStepState& decryptionState = archiveUnpackState.decryptionState;
  while (!decryptionState.isComplete)
  {
    //cast to int because otherwise percent will be treated as character
    std::cout << "\rDecrypting..." << static_cast<int>(decryptionState.percent) << "%";
    if (decryptionState.isError)
    {
      std::cout << " ERROR!" << std::endl;
      return;
    }
  }
  std::cout << "\rDecrypting...100%" << std::endl;

  //extraction
  UnpackStepState& extractionState = archiveUnpackState.extractionState;
  while (!extractionState.isComplete)
  {
    //cast to int because otherwise percent will be treated as character
    std::cout << "\rExtracing...." << static_cast<int>(extractionState.percent) << "%";
    if (extractionState.isError)
    {
      std::cout << " ERROR!" << std::endl;
      return;
    }
  }
  std::cout << "\rExtracing....100%" << std::endl;
}

} // namespace arcextractor
