#include "imguiWindow_dx11.h"
#include <string>
#include <regex>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <filesystem>
#include "range.hpp"
#include "mz/wstring.h"
#include "mz/ystring.h"
#include "mz/basics.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace std;
using namespace std::mz;
namespace rj = rapidjson;
namespace fs = std::filesystem;

// Data
regex regVideoID("v=(.*?)&");
regex regPlaylistID("list=(.*?)&");

vector<thread> Actions;
int nActionsDone = 0;
mutex ActionMutex;

string YoutubeDlPath = "youtube-dl.exe";
string MkvExtractPath = "mkvextract.exe";

string GetClipboardText() {
  if (!IsClipboardFormatAvailable(CF_TEXT)) return "";
  string text = "";
  if (OpenClipboard(nullptr)) {
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData!=nullptr) {
      char *pszText = static_cast<char *>(GlobalLock(hData));
      if (pszText!=nullptr) {
        text = string(pszText);
        GlobalUnlock(hData);
      }
    }
    CloseClipboard();
  }
  return text;
}

string RegexGetFirstMatch(regex Reg, string Str) {
  smatch Match;
  if (regex_search(Str, Match, Reg)) if (Match.size()>=2) return Match[1].str();
  return "";
}


auto MakeAction(TImGuiWindowDX11 &ImGuiWindow, string Command) {
  return [Command, &ImGuiWindow] {
    lock_guard<mutex> guard(ActionMutex);
    system(Command.c_str());
    ImGuiWindow.ScheduleRedraw();
    nActionsDone++;
  };
}

auto MakeAction(TImGuiWindowDX11 &ImGuiWindow, function<void()> Command) {
  return [Command, &ImGuiWindow] {
    lock_guard<mutex> guard(ActionMutex);
    Command();
    ImGuiWindow.ScheduleRedraw();
    nActionsDone++;
  };
}

void DoMainWindow(TImGuiWindowDX11 &ImGuiWindow) {
  ImGui::Begin("Invisible title", nullptr, ImGuiWindowFlags_NoTitleBar+ImGuiWindowFlags_NoResize+ImGuiWindowFlags_NoMove+ImGuiWindowFlags_NoSavedSettings);
  {
    auto ClipStr = GetClipboardText();
    if (ClipStr.size()>2000) ClipStr = ""; else ClipStr += "&";
    auto VideoID = RegexGetFirstMatch(regVideoID, ClipStr);
    auto ListID = RegexGetFirstMatch(regPlaylistID, ClipStr);

    ImGui::Text("Video-ID: %s", VideoID.c_str());
    ImGui::Text("Playlist-ID: %s", ListID.c_str());

    if (ImGui::Button("Download single")) {
      if (VideoID!="") Actions.emplace_back(MakeAction(ImGuiWindow, YoutubeDlPath+ystr(" --audio-format best -x -i -o ""_Singles/%%(title)s.%%(ext)s"" https://www.youtube.com/watch?v=%", VideoID)));
    }
    ImGui::SameLine();
    if (ImGui::Button("Download playlist")) {
      if (ListID!="") Actions.emplace_back(MakeAction(ImGuiWindow, YoutubeDlPath+ystr(" --audio-format best -x -i -o ""%%(playlist)s/%%(playlist_index)s_%%(title)s.%%(ext)s"" https://www.youtube.com/playlist?list=%", ListID)));
    }

    if (ImGui::Button("Download anything")) {
      Actions.emplace_back(MakeAction(ImGuiWindow, YoutubeDlPath+ystr(" --audio-format best -x -i -o ""_Anything/%%(title)s.%%(ext)s"" %", ClipStr)));
    }

    if (ImGui::Button("To Ogg")) {
      Actions.emplace_back(MakeAction(ImGuiWindow, [&] {
        for (auto &x:fs::recursive_directory_iterator(".")) {
          if (x.is_regular_file()&&x.path().extension()==".webm") {
            auto y = x.path();
            auto OldFileName = y.wstring();
            auto NewFileName = y.replace_extension(L""".ogg").wstring();
            auto Path = _wstring(MkvExtractPath)+L" \""+OldFileName+L"\" tracks 0:\""+NewFileName+L"\"";
            _wsystem(Path.c_str());
            if (fs::exists(y)) fs::remove(x);
          }
        }
      }));
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Download Folder")) {
      system("start .");
    }

    ImGui::Text("Actions pending: %i", Actions.size()-nActionsDone);
  }
  ImGui::End();
}

// Tries to a read a value out of an object. If that fails, return the default instead, and also create a new value in the object, set to the default.
template <typename T> void JsonGetOrDefault(rj::Document &Doc, rj::Document::Object &Obj, const string &Name, T &Value) {
  auto itr = Obj.FindMember(Name.c_str());
  if (itr!=Obj.MemberEnd()&&itr->value.Is<T>()) {
    Value = itr->value.Get<T>();
  } else {
    rj::Value NewName(Name.c_str(), (rj::SizeType)Name.size(), Doc.GetAllocator());
    if constexpr (is_same_v<char *, T>||is_same_v<const char *, T>) {
      Obj.AddMember(NewName, rj::Value(Value, Doc.GetAllocator()), Doc.GetAllocator());
    } else {
      Obj.AddMember(NewName, rj::Value(Value), Doc.GetAllocator());
      ;
    }
  }
}

void JsonGetOrDefault(rj::Document &Doc, rj::Document::Object &Obj, const string &Name, string &Output) {
  const char *Str = Output.c_str();
  JsonGetOrDefault(Doc, Obj, Name, Str);
  Output = Str;
}

// Return a subobject with the corresponding Name. If that subobject does not exist (or it has the wrong type), create a new empty subobject instead and return that.
rj::Document::Object JsonGetSubObject(rj::Document &Doc, rj::Document::Object &Obj, const string &Name) {
  auto itr = Obj.FindMember(Name.c_str());
  if (itr==Obj.MemberEnd()) {
    rj::Value NewName(Name.c_str(), (rj::SizeType)Name.size(), Doc.GetAllocator());
    Obj.AddMember(NewName, rj::Value(rj::Type::kObjectType), Doc.GetAllocator());
    itr = Obj.FindMember(Name.c_str());
  } else if (!itr->value.IsObject()) itr->value.SetObject();
  return itr->value.GetObjectA();
}

// Parses the Json string into a Json object, and returns that. If this fails somehow, returns an empty object instead.
rj::Document::Object JsonObjectOfString(rj::Document &JsonDoc, const string &ParseString) {
  JsonDoc.Parse(ParseString.c_str());
  if (!JsonDoc.IsObject()) JsonDoc.SetObject();
  return JsonDoc.GetObjectA();
}

void StringOfJson(string &JsonString, rj::Document &Doc) {
  rj::StringBuffer buffer;
  rj::PrettyWriter<rj::StringBuffer> writer(buffer);
  Doc.Accept(writer);
  JsonString.assign(buffer.GetString(), buffer.GetLength());
}

void Init() {
  string JsonText;
  if (!StringOfFile(JsonText, "settings.json")) yprintf("settings.json not found! Created a new file.\n");
  rapidjson::Document JsonDoc;
  auto JsonObj = JsonObjectOfString(JsonDoc, JsonText);
  JsonGetOrDefault(JsonDoc, JsonObj, "YoutubeDlPath", YoutubeDlPath);
  JsonGetOrDefault(JsonDoc, JsonObj, "MkvExtractPath", MkvExtractPath);
  StringOfJson(JsonText, JsonDoc);
  FileOfString("settings.json", JsonText);
}

void ResetWindowSize(TImGuiWindowDX11 &ImDx) {
  auto locWindow = ImDx.GetWindowLocation();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2((float)locWindow.xSz, (float)locWindow.ySz));
}

int main(int, char **) {
  Init();

  TImGuiWindowDX11 ImDx;
  ImDx.SetWndProcExtra([](HWND hWndLoc, UINT msg, WPARAM, LPARAM)->LRESULT {
    switch (msg) {
    case WM_CREATE:
      AddClipboardFormatListener(hWndLoc);
      return 0;
    case WM_DESTROY:
      RemoveClipboardFormatListener(hWndLoc);
      return 0;
    }
    return 0;
  });
  ImDx.Init("youtube-dl-audio-gui", 100, 100, 300, 188);

  while (!ImDx.ProcessMessagesAndCheckIfQuit(false)) {
    ImDx.NewFrame();
    ResetWindowSize(ImDx);
    DoMainWindow(ImDx);
    ImDx.Render(true);
  }
  ImDx.Shutdown();

  return 0;
}
