#pragma once

#include "imguiWindow.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <functional>
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class TImGuiWindowDX11:public TImGuiWindow {
  ID3D11Device*            g_pd3dDevice = NULL;
  ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
  IDXGISwapChain*          g_pSwapChain = NULL;
  ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
  HWND hWndGUI;
  WNDCLASSEX wc;

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


  HRESULT CreateDeviceD3D() {
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
    sd.OutputWindow = hWndGUI;
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

  LRESULT WINAPI WndProcActual(HWND hWndLoc, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWndLoc, msg, wParam, lParam)!=0) return 1;

    if (fWndProcExtra!=nullptr) {
      auto LresultExtra = fWndProcExtra(hWndLoc, msg, wParam, lParam);
      if (LresultExtra!=0) return LresultExtra;
    }

    switch (msg) {
    case WM_SIZE:
      if (g_pd3dDevice!=NULL&&wParam!=SIZE_MINIMIZED) {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
      }
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
    return DefWindowProc(hWndLoc, msg, wParam, lParam);
  }

  static LRESULT WINAPI WndProcStatic(HWND hWndLoc, UINT msg, WPARAM wParam, LPARAM lParam) {
    TImGuiWindowDX11 *pThis;

    if (msg==WM_NCCREATE) {
      pThis = static_cast<TImGuiWindowDX11*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

      SetLastError(0);
      if (!SetWindowLongPtr(hWndLoc, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis))) {
        if (GetLastError()!=0)
          return FALSE;
      }
    } else {
      pThis = reinterpret_cast<TImGuiWindowDX11*>(GetWindowLongPtr(hWndLoc, GWLP_USERDATA));
    }

    if (pThis) {
      return pThis->WndProcActual(hWndLoc, msg, wParam, lParam);
      // use pThis->member as needed...
    } else return DefWindowProc(hWndLoc, msg, wParam, lParam);
  }

public:
  virtual bool Init(std::string WindowName, int x, int y, int xSz, int ySz) override {
    // Create application window

    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProcStatic, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T(WindowName.c_str()), NULL};
    RegisterClassEx(&wc);
    hWndGUI = CreateWindow(wc.lpszClassName, _T(WindowName.c_str()), WS_OVERLAPPEDWINDOW, x, y, xSz, ySz, NULL, NULL, wc.hInstance, this);

    // Initialize Direct3D
    if (CreateDeviceD3D()<0) {
      CleanupDeviceD3D();
      UnregisterClass(wc.lpszClassName, wc.hInstance);
      return false;
    }

    // Show the window
    ShowWindow(hWndGUI, SW_SHOWDEFAULT);
    UpdateWindow(hWndGUI);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 24.0f);
    ImGui::GetStyle().ScaleAllSizes(1.5);

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hWndGUI);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
  }
  virtual bool ProcessMessagesAndCheckIfQuit(bool WaitForEvent) override {
    MSG msg;
    if (WaitForEvent) {
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
    } else {
      while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      auto locWindow = GetWindowLocation();
      if (locWindow.xSz==0&&locWindow.ySz==0) {
        Sleep(50);
      }
    }
    return false;
  }
  virtual void Shutdown() override {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hWndGUI);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
  void NewFrame() override {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
  }
  TRect GetWindowLocation() override {
    RECT WindowLoc;
    GetClientRect(hWndGUI, &WindowLoc);
    return TRect{WindowLoc.left,WindowLoc.top,WindowLoc.right-WindowLoc.left,WindowLoc.bottom-WindowLoc.top};
  }
  void ScheduleRedraw() override {
    InvalidateRect(hWndGUI, NULL, TRUE);
  }
  void Render(bool ClearAll) override {
    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    if (ClearAll) {
      ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
      g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
    }
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
  }
public:
  void SetWndProcExtra(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fWndProcExtra_) {
    fWndProcExtra = fWndProcExtra_;
  };
private:
  std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fWndProcExtra = nullptr;
};
