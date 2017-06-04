#include <iostream>
#include <fstream>

#include "Utils.h"

int main(int argc, char* argv[])
{
  //first parameter should be an archive name
  if (argc < 2)
  {
    std::cout << "Syntax: " << argv[0] << " <archiveName.arc>" << std::endl;
    return 0;
  }

  //open archive
  std::ifstream archive;
  archive.open(argv[1], std::ios::in | std::ios::binary);
  if (archive.fail())
  {
    archive.close();
    std::cout << "Error: Cannot open archive." << std::endl;
    return 1;
  }

  //determine string format and unpack archive
  uint8_t numBytesPerSymbol = arcextractor::numBytesPerSymbolInArchiveStrings(archive);
  if (numBytesPerSymbol == 1)
  {
    arcextractor::unpackArchive<char>(archive);
  }
  else
  {
    arcextractor::unpackArchive<wchar_t>(archive);
  }

  std::cout << "Done." << std::endl;

  return 0;
}
