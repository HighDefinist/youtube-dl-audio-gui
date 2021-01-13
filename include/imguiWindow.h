#pragma once
#include <string>

class TImGuiWindow {
public:
  struct TRect {
    int x, y, xSz, ySz;
  };

  // Should be called once at the beginning of every frame. Creates a window titled WindowName at the location (x,y), with width xSz and height ySz.
  virtual bool Init(std::string WindowName, int x, int y, int xSz, int ySz) = 0;

  // Should be called once, at the beginning of a frame, to process any basic window messages such as resizing or closing the window. When the window is closed, this function returns TRUE, otherwise FALSE. WaitForEvent=true means that the function waits until the user performs an action in the window. Otherwise it returns immediately.
  virtual bool ProcessMessagesAndCheckIfQuit(bool WaitForEvent) = 0;

  // Should be called during the frame when starting the GUI drawing operations (always after ProcessMessagesAndCheckIfQuit)
  virtual void NewFrame() = 0;

  // Called at some point after NewFrame() and any ImGUI-calls for the current frame, to render the result to the current backbuffer. If ClearAll, this function also overrides the current backbuffer contents with black before rendering the ImGUI result.
  virtual void Render(bool ClearAll) = 0;

  // Return the current window location and size (more specifcally: The client area of the window, so the border and title of the window are not included)
  virtual TRect GetWindowLocation() = 0;

  // If ProcessMessagesAndCheckIfQuit is currently waiting for user input (WaitForEvent == true), it returns immediately, or the next time it is called. ScheduleRedraw can be called from a different thread than the render thread.
  virtual void ScheduleRedraw() = 0;

  // Should be called once in the end. This frees up any reserved resources.
  virtual void Shutdown() = 0;

  virtual ~TImGuiWindow() {};
};