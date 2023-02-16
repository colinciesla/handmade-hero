#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *bitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void win32resizeDIBSection(int width, int height) {
    if (BitmapHandle) {
        DeleteObject(BitmapHandle);
    }

    if (!BitmapDeviceContext) {
        BitmapDeviceContext = CreateCompatibleDC(nullptr);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = width;
    BitmapInfo.bmiHeader.biHeight = height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(
            BitmapDeviceContext,
            &BitmapInfo,
            DIB_RGB_COLORS,
            &bitmapMemory,
            0,
            0
    );
}

internal void win32UpdateWindow(HDC DeviceContext, int x, int y, int width, int height) {
    StretchDIBits(
            DeviceContext,
            x,
            y,
            width,
            height,
            x,
            y,
            width,
            height,
            bitmapMemory,
            &BitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY
    );
}

LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(window, &ClientRect);
            int height = ClientRect.bottom - ClientRect.top;
            int width = ClientRect.right - ClientRect.left;
            win32resizeDIBSection(width, height);
        }
            break;

        case WM_DESTROY: {
            running = false;
        }
            break;

        case WM_CLOSE: {
            running = false;
        }
            break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
            break;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC DeviceContext = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            win32UpdateWindow(DeviceContext, x, y, width, height);
            EndPaint(window, &paint);
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
    windowClass.lpfnWndProc = win32MainWindowCallback;
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
            running = true;

            while (running) {
                MSG message;
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