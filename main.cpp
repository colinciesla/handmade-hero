#include <windows.h>
#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;

internal void renderGradient(int xOffset, int yOffset) {
    int width = bitmapWidth;
    int height = bitmapHeight;

    int pitch = width * bytesPerPixel;
    auto *row = (uint8_t *) bitmapMemory;

    for (int y = 0; y < bitmapHeight; ++y) {
        auto *pixel = (uint8_t *) row;

        for (int x = 0; x < bitmapWidth; ++x) {
            *pixel = 0;
            ++pixel;

            *pixel = (uint8_t) (x + xOffset);
            ++pixel;

            *pixel = (uint8_t) (y + yOffset);
            ++pixel;

            *pixel = 0;
            ++pixel;
        }

        row += pitch;
    }
};

internal void win32resizeDIBSection(int width, int height) {
    if (bitmapMemory) {
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

    bitmapWidth = width;
    bitmapHeight = height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = bitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -bitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
    bitmapMemory = VirtualAlloc(nullptr, bitmapMemorySize,
                                MEM_COMMIT, PAGE_READWRITE);

    renderGradient(128, 0);
}

internal void win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int x, int y, int width, int height) {
    int windowWidth = ClientRect->right - ClientRect->left;
    int windowHeight = ClientRect->bottom - ClientRect->top;

    StretchDIBits(
            DeviceContext,
            0, 0, bitmapWidth, bitmapHeight,
            0, 0, windowWidth, windowHeight,
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

            RECT ClientRect;
            GetClientRect(window, &ClientRect);

            win32UpdateWindow(DeviceContext, &ClientRect, x, y, width, height);
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
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = win32MainWindowCallback;
    WindowClass.hInstance = instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass)) {
        HWND Window = CreateWindowEx(
                0,
                WindowClass.lpszClassName,
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

        if (Window) {
            int xOffset = 0;
            int yOffset = 0;
            
            running = true;

            while (running) {
                MSG message;

                while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        running = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                renderGradient(xOffset, yOffset);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);

                int windowWidth = ClientRect.right - ClientRect.left;
                int windowHeight = ClientRect.bottom - ClientRect.top;

                win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, windowWidth, windowHeight);

                ReleaseDC(Window, DeviceContext);

                ++xOffset;
            }
        } else {

        }
    } else {

    }

    return 0;
}