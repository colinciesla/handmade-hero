#include <windows.h>
#include <cstdint>
#include <xinput.h>

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

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)

typedef X_INPUT_GET_STATE(x_input_get_state);

X_INPUT_GET_STATE(XInputGetStateStub) {
    return (0);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)

typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_SET_STATE(XInputSetStateStub) {
    return (0);
}

global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputSetState XInputSetState_

internal void Win32LoadXInput() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

    if (XInputLibrary) {
        XInputGetState = (x_input_get_state *) GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *) GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

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


internal void renderGradient(WIN32_OFFSCREEN_BUFFER *Buffer, int blueOffset, int greenOffset) {
    auto *row = (uint8_t *) Buffer->memory;

    for (int y = 0; y < Buffer->height; ++y) {
        auto *pixel = (uint32_t *) row;

        for (int x = 0; x < Buffer->width; ++x) {
            uint8_t blue = (x + blueOffset);
            uint8_t green = (y + greenOffset);

            *pixel++ = ((green << 8) | blue);
        }

        row += Buffer->pitch;
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
win32CopyBufferToWindow(WIN32_OFFSCREEN_BUFFER *Buffer, HDC DeviceContext, int windowWidth, int windowHeight) {
    StretchDIBits(
            DeviceContext,
            0, 0, windowWidth, windowHeight,
            0, 0, Buffer->width, Buffer->height,
            Buffer->memory,
            &Buffer->Info,
            DIB_RGB_COLORS,
            SRCCOPY
    );
}

LRESULT CALLBACK win32MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
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

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            uint32_t VKCode = wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);
            bool isDown = ((lParam & (1 << 31)) == 0);

            if (wasDown != isDown) {
                if (VKCode == 'W') {

                } else if (VKCode == 'A') {

                } else if (VKCode == 'S') {

                } else if (VKCode == 'D') {

                } else if (VKCode == 'Q') {

                } else if (VKCode == 'E') {

                } else if (VKCode == VK_UP) {

                } else if (VKCode == VK_LEFT) {

                } else if (VKCode == VK_RIGHT) {

                } else if (VKCode == VK_DOWN) {

                } else if (VKCode == VK_ESCAPE) {
                    OutputDebugStringA("ESCAPE: ");
                    if (isDown) {
                        OutputDebugStringA("WEOW");
                    }

                    if (wasDown) {
                        OutputDebugStringA("WEW");
                    }

                    OutputDebugStringA("\n");
                } else if (VKCode == VK_SPACE) {

                }
            }
        }
            break;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC DeviceContext = BeginPaint(Window, &paint);
            WIN32_WINDOW_DIMENSION Dimension = Win32GetWindowDimension(Window);

            win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension.width, Dimension.height);
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
    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

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

                for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                        XINPUT_GAMEPAD *Gamepad = &ControllerState.Gamepad;

                        bool Up = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Gamepad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Gamepad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Gamepad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Gamepad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Gamepad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Gamepad->wButtons & XINPUT_GAMEPAD_Y);

                        int16_t StickX = Gamepad->sThumbLX;
                        int16_t StickY = Gamepad->sThumbLY;

                        if (AButton) {
                            yOffset += 2;
                        }
                    } else {

                    }
                }

                renderGradient(&GlobalBackBuffer, xOffset, yOffset);

                HDC DeviceContext = GetDC(Window);

                WIN32_WINDOW_DIMENSION Dimension = Win32GetWindowDimension(Window);

                win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension.width, Dimension.height);

                ReleaseDC(Window, DeviceContext);

                ++xOffset;
            }
        } else {

        }
    } else {

    }

    return 0;
}