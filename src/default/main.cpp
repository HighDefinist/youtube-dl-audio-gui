// dear imgui - standalone example application for DirectX 11
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <string>
#include <regex>
#include <thread>
#include <vector>
#include <mutex>
#include "range.hpp"
#include "mz/ystring.h"

using namespace std;
using namespace std::mz;

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
static HWND hwndNextViewer = NULL;

regex regVideoID("v=(.*?)&");
regex regPlaylistID("list=(.*?)&");

vector<thread> Actions;
int nActionsDone = 0;
mutex ActionMutex;

void CreateRenderTarget() {
  ID3D11Texture2D* pBackBuffer;
  g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = NULL;
  }
}

HRESULT CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
  if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)!=S_OK)
    return E_FAIL;

  CreateRenderTarget();

  return S_OK;
}

void CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
  if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
  if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

  switch (msg) {
  case WM_CREATE:
    hwndNextViewer = SetClipboardViewer(hWnd);
    return 0;
  case WM_DRAWCLIPBOARD:
    if (hwndNextViewer) SendMessage(hwndNextViewer, msg, wParam, lParam);
    InvalidateRect(hWnd, NULL, TRUE);
    return 0;
  case WM_CHANGECBCHAIN:
    if ((HWND)wParam==hwndNextViewer)
      hwndNextViewer = (HWND)lParam;
    else if (hwndNextViewer!=NULL)
      SendMessage(hwndNextViewer, msg, wParam, lParam);
    return 0;
  case WM_SIZE:
    if (g_pd3dDevice!=NULL&&wParam!=SIZE_MINIMIZED) {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
      CreateRenderTarget();
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam&0xfff0)==SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ChangeClipboardChain(hWnd, hwndNextViewer);
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

string GetClipboardText() {
  // Try opening the clipboard
  if (!OpenClipboard(nullptr)) return "<Error 1>"; // error

  // Get handle of clipboard object for ANSI text
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData==nullptr) return "<Error 2>";// error

// Lock the handle to get the actual text pointer
  char * pszText = static_cast<char*>(GlobalLock(hData));
  if (pszText==nullptr) return "<Error 3>"; // error

  // Save text in a string class instance
  string text(pszText);

  // Release the lock
  GlobalUnlock(hData);

  // Release the clipboard
  CloseClipboard();

  return text;
}

string RegexGetFirstMatch(regex Reg, string Str) {
  smatch Match;

  if (regex_search(Str, Match, Reg)) {
    if (Match.size()>=2) return Match[1].str();
  }

  return "";
}

bool ProcessMessagesAndCheckIfQuit() {
  MSG msg;
  if (GetMessage(&msg, NULL, 0U, 0U)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message==WM_QUIT) return true;
    while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message==WM_QUIT) return true;
    }
  }
  return false;
}

auto MakeAction(HWND hwnd, string Command) {
  return [Command, hwnd] {
    lock_guard<mutex> guard(ActionMutex);
    system(Command.c_str());
    InvalidateRect(hwnd, NULL, TRUE);
    nActionsDone++;
  };
}

void DoMainWindow(HWND hwnd) {
  ImGui::Begin("Invisible title", nullptr, ImGuiWindowFlags_NoTitleBar+ImGuiWindowFlags_NoResize+ImGuiWindowFlags_NoMove+ImGuiWindowFlags_NoSavedSettings);
  {
    auto ClipStr = GetClipboardText();
    if (ClipStr.size()>2000) ClipStr = ""; else ClipStr += "&";
    auto VideoID = RegexGetFirstMatch(regVideoID, ClipStr);
    auto ListID = RegexGetFirstMatch(regPlaylistID, ClipStr);

    ImGui::Text("Video-ID: %s", VideoID.c_str());
    ImGui::Text("Playlist-ID: %s", ListID.c_str());

    if (ImGui::Button("Download single")) {
      if (VideoID!="") Actions.emplace_back(MakeAction(hwnd, ystr("youtube-dl.exe --audio-format best -x -i -o '%%(title)s.%%(ext)s' https://www.youtube.com/watch?v=%", VideoID)));
    }
    ImGui::SameLine();
    if (ImGui::Button("Download playlist")) {
      if (ListID!="") Actions.emplace_back(MakeAction(hwnd, ystr("youtube-dl.exe --audio-format best -x -i -o ""%%(playlist)s/%%(playlist_index)s_%%(title)s.%%(ext)s"" https://www.youtube.com/playlist?list=%", ListID)));
    }

    ImGui::Text("Actions pending: %i", Actions.size()-nActionsDone);
  }
  ImGui::End();
}

int main(int, char**) {
  // Create application window
  WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("youtube-dl-audio-gui"), NULL};
  RegisterClassEx(&wc);
  HWND hwnd = CreateWindow(wc.lpszClassName, _T("youtube-dl-audio-gui"), WS_OVERLAPPEDWINDOW, 100, 100, 600, 200, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (CreateDeviceD3D(hwnd)<0) {
    CleanupDeviceD3D();
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // Show the window
  ShowWindow(hwnd, SW_SHOWDEFAULT);
  UpdateWindow(hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 24.0f);
  ImGui::GetStyle().ScaleAllSizes(1.5);

  // Setup Platform/Renderer bindings
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  // Main loop
  while (!ProcessMessagesAndCheckIfQuit()) {
    // Start the ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RECT WindowLoc;
    GetClientRect(hwnd, &WindowLoc);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)(WindowLoc.right-WindowLoc.left), (float)(WindowLoc.bottom-WindowLoc.top)));
    DoMainWindow(hwnd);

    // Rendering
    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
  }

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  DestroyWindow(hwnd);
  UnregisterClass(wc.lpszClassName, wc.hInstance);

  return 0;
}
