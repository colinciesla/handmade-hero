#include <windows.h>

LRESULT CALLBACK mainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
        case WM_SIZE: {
            OutputDebugStringA("WM_SIZE\n");
        }
            break;

        case WM_DESTROY: {
            OutputDebugStringA("WM_DESTROY\n");
        }
            break;

        case WM_CLOSE: {
            OutputDebugStringA("WM_CLOSE\n");
        }
            break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
            break;

        default: {
            // OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, wParam, lParam);
        }
            break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
    WNDCLASS windowClass = {};

    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = mainWindowCallback;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&windowClass)) {
        HWND windowHandle = CreateWindowEx(
                0,
                windowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                nullptr,
                nullptr,
                instance,
                nullptr);

        if (windowHandle) {
            MSG message;

            for (;;) {
                BOOL messageResult = GetMessage(&message, nullptr, 0, 0);
                if (messageResult > 0) {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                } else {
                    break;
                }
            }
        } else {

        }
    } else {

    }

    return 0;
}