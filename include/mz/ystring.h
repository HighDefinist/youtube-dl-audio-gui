// Contains various macros and templates which are frequently used, related to strings, tuples and functional programming. Unlike the macros in base.h, they require additional, but common, includes

#pragma once

#include <codecvt>
#include <locale>
#include <string>
#include <stdio.h>

/////////////
// ystring //
///////////// 

namespace std {
  namespace mz {
    inline void yadd_str(string& s, const wstring& value) {
      wstring_convert<codecvt_utf8_utf16<wchar_t>> str_convert;
      s += str_convert.to_bytes(value);
    }

    inline void yadd_str(string& s, const string& value) {
      s += value;
    }

    template<typename T> typename enable_if<is_integral<T>::value, void>::type yadd_str(string& s, const T& value) {
      s += to_string(value);
    }

    inline void yadd_str(string& s, const float& value) {
      char cc[20];
      auto i = snprintf(&cc[0], 20, "%.7g", static_cast<double> (value));
      (void)i;
      s += cc;
    }

    inline void yadd_str(string& s, const double& value) {
      char cc[30];
      auto i = snprintf(&cc[0], 30, "%.15g", value);
      (void)i;
      s += cc;
    }

    inline void xstr(string& result, const char *s) {
      while (*s) {
        if (*s=='%') {
          if (*(s+1)=='%') ++s;
          else throw runtime_error("invalid format string: missing arguments");
        }
        result += *s++;
      }
    }

    template<typename... Args> void yadd_str(string& s, tuple<Args...> value);
    template<size_t i, typename... Args> typename enable_if<i==0, void>::type ytuple_str(string& str, tuple<Args...> &args) {
      str += "[";
      yadd_str(str, get<i>(args));
    }
    template<size_t i, typename... Args> typename enable_if<i!=0, void>::type ytuple_str(string& str, tuple<Args...> &args) {
      ytuple_str<i-1, Args...>(str, args);
      str += ";";
      yadd_str(str, get<i>(args));
    }
    template<typename... Args> void yadd_str(string& s, tuple<Args...> value) {
      ytuple_str<sizeof...(Args)-1, Args...>(s, value);
      s += "]";
    }

    template<typename T, typename... Args> void xstr(string& result, const char *s, const T& value, Args... args) {
      while (*s) {
        if (*s=='%') {
          if (*(s+1)=='%') {
            ++s;
          } else {
            yadd_str(result, value);
            xstr(result, s+1, args...); // call even when *s == 0 to throw exception on extra arguments
            return;
          }
        }
        result += *s++;
      }
      throw logic_error("extra arguments provided to xstr");
    }

    template<typename... Args> string ystr(string s, Args... args) {
      string str;
      str.reserve(s.length());
      xstr(str, s.c_str(), args...);
      return str;
    }

    template<typename... Args> void yprintf(string s, Args... args) {
      auto ll = ystr(s, args...);
      printf("%s", ll.c_str());
    }

    template<typename T>string _str(const T& value) {
      return ystr("%", value);
    }
  }
}