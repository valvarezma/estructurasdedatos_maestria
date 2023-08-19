#include <iostream>
#include <windows.h>

using namespace std;

int main() {
  // Get the current mouse position.
  POINT cursorPos;
  GetCursorPos(&cursorPos);

  // Create an infinite loop.
  while (true) {
    // Move the mouse to a random position on the screen.
    cursorPos.x = rand() % GetSystemMetrics(SM_CXSCREEN);
    cursorPos.y = rand() % GetSystemMetrics(SM_CYSCREEN);

    // Move the mouse to the new position.
    SetCursorPos(cursorPos.x, cursorPos.y);

    // Sleep for 10 seconds.
    Sleep(10000);
  }

  return 0;
}