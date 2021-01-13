// This code is modified, but originally from the "The CUDA Handbook". It was published with the 2-clause BSD license.

#pragma once
#include <fstream>
#include <string>

#define Ctypecopy(name) typedef const name C##name
#define Ctypedef(type,name) typedef type name; Ctypecopy(name)

Ctypedef(char, i8);
Ctypedef(short, i16);
Ctypedef(int, i32);
Ctypedef(long long, i64);
Ctypedef(unsigned char, ui8);
Ctypedef(unsigned short, ui16);
Ctypedef(unsigned int, ui32);
Ctypedef(unsigned long long, ui64);
Ctypedef(float, fl);
Ctypedef(double, db);
Ctypecopy(bool);
Ctypecopy(int);

#define mt make_tuple
#define gt(A,B) get<B>(A)

namespace std {
  namespace mz {
    bool StringOfFile(string& Data, string FileName) {
      ifstream FileStream(FileName, ifstream::binary);
      if (!FileStream.is_open()) {
        Data = "";
        return false;
      }
      FileStream.seekg(0, FileStream.end);
      size_t FileSize = (size_t)FileStream.tellg();
      FileStream.seekg(0, FileStream.beg);
      Data.resize(FileSize);
      FileStream.read(&Data[0], FileSize);
      FileStream.close();
      return true;
    }
    bool FileOfString(string FileName, string& Data) {
      ofstream FileStream(FileName, ofstream::binary);
      if (!FileStream.is_open()) return false;
      FileStream.write(&Data[0], Data.size());
      FileStream.close();
      return true;
    }
  }
}