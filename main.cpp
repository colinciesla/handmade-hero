#include <windows.h>
#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

struct WIN32_OFFSCREEN_BUFFER {
    BITMAPINFO Info{};
    void *memory{};
    int width{};
    int height{};
    int pitch{};
    int bytesPerPixel{};
};

struct WIN32_WINDOW_DIMENSION {
    int width{};
    int height{};
};

WIN32_WINDOW_DIMENSION Win32GetWindowDimension(HWND Window) {
    WIN32_WINDOW_DIMENSION Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.height = ClientRect.bottom - ClientRect.top;
    Result.width = ClientRect.right - ClientRect.left;

    return Result;
}

global_variable bool running;
global_variable WIN32_OFFSCREEN_BUFFER GlobalBackBuffer;

internal void renderGradient(WIN32_OFFSCREEN_BUFFER Buffer, int blueOffset, int greenOffset) {
    auto *row = (uint8_t *) Buffer.memory;

    for (int y = 0; y < Buffer.height; ++y) {
        auto *pixel = (uint32_t *) row;

        for (int x = 0; x < Buffer.width; ++x) {
            uint8_t blue = (x + blueOffset);
            uint8_t green = (y + greenOffset);

            *pixel++ = ((green << 8) | blue);
        }

        row += Buffer.pitch;
    }
}

internal void win32resizeDIBSection(WIN32_OFFSCREEN_BUFFER *Buffer, int width, int height) {
    if (Buffer->memory) {
        VirtualFree(Buffer->memory, 0, MEM_RELEASE);
    }

    Buffer->width = width;
    Buffer->height = height;
    Buffer->bytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = (Buffer->width * Buffer->height) * Buffer->bytesPerPixel;
    Buffer->memory = VirtualAlloc(nullptr, bitmapMemorySize,
                                  MEM_COMMIT, PAGE_READWRITE);

    Buffer->pitch = width * Buffer->bytesPerPixel;
}

internal void
win32CopyBufferToWindow(WIN32_OFFSCREEN_BUFFER Buffer, HDC DeviceContext, int windowWidth, int windowHeight) {
    StretchDIBits(
            DeviceContext,
            0, 0, windowWidth, windowHeight,
            0, 0, Buffer.width, Buffer.height,
            Buffer.memory,
            &Buffer.Info,
            DIB_RGB_COLORS,
            SRCCOPY
    );
}

LRESULT CALLBACK win32MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
        case WM_SIZE: {

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
            HDC DeviceContext = BeginPaint(Window, &paint);
            WIN32_WINDOW_DIMENSION Dimension = Win32GetWindowDimension(Window);

            win32CopyBufferToWindow(GlobalBackBuffer, DeviceContext, Dimension.width, Dimension.height);
            EndPaint(Window, &paint);
        }
            break;

        default: {
            // OutputDebugStringA("default\n");
            result = DefWindowProc(Window, message, wParam, lParam);
        }
            break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
    WNDCLASS WindowClass = {};

    win32resizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = win32MainWindowCallback;
    WindowClass.hInstance = instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass)) {
        HWND Window = CreateWindowExA(
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

                renderGradient(GlobalBackBuffer, xOffset, yOffset);

                HDC DeviceContext = GetDC(Window);

                WIN32_WINDOW_DIMENSION Dimension = Win32GetWindowDimension(Window);

                win32CopyBufferToWindow(GlobalBackBuffer, DeviceContext, Dimension.width, Dimension.height);

                ReleaseDC(Window, DeviceContext);

                ++xOffset;
                yOffset += 2;
            }
        } else {

        }
    } else {

    }

    return 0;
}