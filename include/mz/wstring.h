#pragma once
#include <string>
#include <locale>
#include <codecvt>
#define _Z_WSTRING
using namespace std;

__inline wstring _wstring(const string& s){
  wstring_convert<codecvt_utf8_utf16<wchar_t>> str_convert;
  return str_convert.from_bytes(s);
}
__inline wstring _wstring(const wstring& s){ return s; }
__inline string _string(const string& s){ return s; }
__inline string _string(const wstring& s){
  wstring_convert<codecvt_utf8_utf16<wchar_t>> str_convert;
  return str_convert.to_bytes(s);
}


#ifdef _UNICODE
__inline wstring _tstring(const wstring& s){ return s; }
__inline wstring _tstring(const string& s){
  wstring_convert<codecvt_utf8_utf16<wchar_t>> str_convert;
  return str_convert.from_bytes(s);
}
#define tmain wmain
#define tchar wchar_t
#define tcout wcout
#else
__inline string _tstring(const string& s){ return s; }
__inline string _tstring(const wstring& s){
  wstring_convert<codecvt_utf8_utf16<wchar_t>> str_convert;
  return str_convert.to_bytes(s);
}
#define tmain main
#define tchar char
#define tcout cout
#endif

