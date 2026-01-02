#include "pch.h" // 必须是第一行
#include <Windows.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <stdint.h>

// 全局配置（对应Python中的FAILSAFE/PAUSE）
bool FAILSAFE = true;
const POINT FAILSAFE_POINTS[] = { {0, 0} };
const size_t FAILSAFE_POINT_COUNT = 1;
double PAUSE = 0.1; // 默认暂停0.1秒

// 鼠标事件常量（与Python一致）
#define MOUSEEVENTF_MOVE          0x0001
#define MOUSEEVENTF_ABSOLUTE      0x8000
#define MOUSEEVENTF_LEFTDOWN      0x0002
#define MOUSEEVENTF_LEFTUP        0x0004
#define MOUSEEVENTF_LEFTCLICK     (MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP)
#define MOUSEEVENTF_RIGHTDOWN     0x0008
#define MOUSEEVENTF_RIGHTUP       0x0010
#define MOUSEEVENTF_RIGHTCLICK    (MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP)
#define MOUSEEVENTF_MIDDLEDOWN    0x0020
#define MOUSEEVENTF_MIDDLEUP      0x0040
#define MOUSEEVENTF_MIDDLECLICK   (MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP)

// 键盘事件常量（与Python一致）
#define KEYEVENTF_EXTENDEDKEY     0x0001
#define KEYEVENTF_KEYUP           0x0002
#define KEYEVENTF_SCANCODE        0x0008
#define KEYEVENTF_UNICODE         0x0004

// 键盘扫描码映射表（完整移植Python中的KEYBOARD_MAPPING）
std::unordered_map<std::string, uint16_t> KEYBOARD_MAPPING = {
    {"escape", 0x01}, {"esc", 0x01},
    {"f1", 0x3B}, {"f2", 0x3C}, {"f3", 0x3D}, {"f4", 0x3E},
    {"f5", 0x3F}, {"f6", 0x40}, {"f7", 0x41}, {"f8", 0x42},
    {"f9", 0x43}, {"f10", 0x44}, {"f11", 0x57}, {"f12", 0x58},
    {"printscreen", 0xB7}, {"prntscrn", 0xB7}, {"prtsc", 0xB7}, {"prtscr", 0xB7},
    {"scrolllock", 0x46}, {"pause", 0xC5}, {"`", 0x29},
    {"1", 0x02}, {"2", 0x03}, {"3", 0x04}, {"4", 0x05}, {"5", 0x06},
    {"6", 0x07}, {"7", 0x08}, {"8", 0x09}, {"9", 0x0A}, {"0", 0x0B},
    {"-", 0x0C}, {"=", 0x0D}, {"backspace", 0x0E},
    {"insert", 0xD2 + 1024}, {"home", 0xC7 + 1024}, {"pageup", 0xC9 + 1024}, {"pagedown", 0xD1 + 1024},
    {"numlock", 0x45}, {"divide", 0xB5 + 1024}, {"multiply", 0x37},
    {"subtract", 0x4A}, {"add", 0x4E}, {"decimal", 0x53}, {"numpadenter", 0x9C + 1024},
    {"numpad1", 0x4F}, {"numpad2", 0x50}, {"numpad3", 0x51}, {"numpad4", 0x4B},
    {"numpad5", 0x4C}, {"numpad6", 0x4D}, {"numpad7", 0x47}, {"numpad8", 0x48},
    {"numpad9", 0x49}, {"numpad0", 0x52}, {"tab", 0x0F},
    {"q", 0x10}, {"w", 0x11}, {"e", 0x12}, {"r", 0x13}, {"t", 0x14},
    {"y", 0x15}, {"u", 0x16}, {"i", 0x17}, {"o", 0x18}, {"p", 0x19},
    {"[", 0x1A}, {"]", 0x1B}, {"\\", 0x2B}, {"del", 0xD3 + 1024}, {"delete", 0xD3 + 1024},
    {"end", 0xCF + 1024}, {"capslock", 0x3A},
    {"a", 0x1E}, {"s", 0x1F}, {"d", 0x20}, {"f", 0x21}, {"g", 0x22},
    {"h", 0x23}, {"j", 0x24}, {"k", 0x25}, {"l", 0x26}, {";", 0x27},
    {"'", 0x28}, {"enter", 0x1C}, {"return", 0x1C},
    {"shift", 0x2A}, {"shiftleft", 0x2A},
    {"z", 0x2C}, {"x", 0x2D}, {"c", 0x2E}, {"v", 0x2F}, {"b", 0x30},
    {"n", 0x31}, {"m", 0x32}, {",", 0x33}, {".", 0x34}, {"/", 0x35},
    {"shiftright", 0x36}, {"ctrl", 0x1D}, {"ctrlleft", 0x1D},
    {"win", 0xDB + 1024}, {"winleft", 0xDB + 1024},
    {"alt", 0x38}, {"altleft", 0x38}, {" ", 0x39}, {"space", 0x39},
    {"altright", 0xB8 + 1024}, {"winright", 0xDC + 1024},
    {"apps", 0xDD + 1024}, {"ctrlright", 0x9D + 1024}
};

// 初始化方向键扫描码（通过MapVirtualKey获取，与Python一致）
void InitDirectionKeyCodes() {
    KEYBOARD_MAPPING["up"] = MapVirtualKey(VK_UP, 0);
    KEYBOARD_MAPPING["left"] = MapVirtualKey(VK_LEFT, 0);
    KEYBOARD_MAPPING["down"] = MapVirtualKey(VK_DOWN, 0);
    KEYBOARD_MAPPING["right"] = MapVirtualKey(VK_RIGHT, 0);
}

// 安全检查（FAILSAFE，鼠标移到0,0触发）
bool FailSafeCheck() {
    if (!FAILSAFE) return true;

    POINT cursor;
    GetCursorPos(&cursor);
    for (size_t i = 0; i < FAILSAFE_POINT_COUNT; i++) {
        if (cursor.x == FAILSAFE_POINTS[i].x && cursor.y == FAILSAFE_POINTS[i].y) {
            return false; // 触发安全检查
        }
    }
    return true;
}

// 暂停函数（对应Python的_handlePause）
void HandlePause(bool pause) {
    if (pause) {
        std::this_thread::sleep_for(std::chrono::duration<double>(PAUSE));
    }
}
// 新增：导出设置PAUSE的函数（放在全局配置下方即可）
extern "C" __declspec(dllexport) void SetPause(double newPauseValue) {
    PAUSE = newPauseValue; // 直接修改全局的PAUSE变量
}
// 新增：导出设置PAUSE的函数（放在全局配置下方即可）
extern "C" __declspec(dllexport) void SetFailsafe(double newFailsafeValue) {
    FAILSAFE = newFailsafeValue; // 直接修改全局的FAILSAFE变量
}

// 获取鼠标当前位置（对应Python的position）
extern "C" __declspec(dllexport) void GetMousePosition(int* x, int* y) {
    POINT cursor;
    GetCursorPos(&cursor);
    *x = cursor.x;
    *y = cursor.y;
}

// 获取屏幕尺寸（对应Python的size）
extern "C" __declspec(dllexport) void GetScreenSize(int* width, int* height) {
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
}

// 转换为Windows绝对坐标（对应Python的_to_windows_coordinates）
void ToWindowsCoordinates(int x, int y, int* winX, int* winY) {
    int screenWidth, screenHeight;
    GetScreenSize(&screenWidth, &screenHeight);
    *winX = (x * 65536) / screenWidth + 1;
    *winY = (y * 65536) / screenHeight + 1;
}

// ===================== 鼠标操作函数 =====================
// 鼠标按下（button: "left"/"middle"/"right"）
extern "C" __declspec(dllexport) bool MouseDown(int x, int y, const char* button, bool pause) {
    if (!FailSafeCheck()) return false;

    // 如果指定坐标，先移动到目标位置
    if (x >= 0 && y >= 0) {
        int winX, winY;
        ToWindowsCoordinates(x, y, &winX, &winY);
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = winX;
        input.mi.dy = winY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        SendInput(1, &input, sizeof(INPUT));
    }

    // 确定鼠标按键事件
    DWORD mouseFlag = 0;
    std::string btn(button);
    if (btn == "left" || btn == "primary") mouseFlag = MOUSEEVENTF_LEFTDOWN;
    else if (btn == "middle") mouseFlag = MOUSEEVENTF_MIDDLEDOWN;
    else if (btn == "right" || btn == "secondary") mouseFlag = MOUSEEVENTF_RIGHTDOWN;
    else return false;

    // 发送鼠标按下事件
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = mouseFlag;
    SendInput(1, &input, sizeof(INPUT));

    HandlePause(pause);
    return true;
}

// 鼠标抬起
extern "C" __declspec(dllexport) bool MouseUp(int x, int y, const char* button, bool pause) {
    if (!FailSafeCheck()) return false;

    if (x >= 0 && y >= 0) {
        int winX, winY;
        ToWindowsCoordinates(x, y, &winX, &winY);
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = winX;
        input.mi.dy = winY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        SendInput(1, &input, sizeof(INPUT));
    }

    DWORD mouseFlag = 0;
    std::string btn(button);
    if (btn == "left" || btn == "primary") mouseFlag = MOUSEEVENTF_LEFTUP;
    else if (btn == "middle") mouseFlag = MOUSEEVENTF_MIDDLEUP;
    else if (btn == "right" || btn == "secondary") mouseFlag = MOUSEEVENTF_RIGHTUP;
    else return false;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = mouseFlag;
    SendInput(1, &input, sizeof(INPUT));

    HandlePause(pause);
    return true;
}

// 鼠标点击（clicks: 点击次数, interval: 间隔秒数）
extern "C" __declspec(dllexport) bool MouseClick(int x, int y, int clicks, double interval, const char* button, bool pause) {
    if (!FailSafeCheck() || clicks <= 0) return false;

    if (x >= 0 && y >= 0) {
        int winX, winY;
        ToWindowsCoordinates(x, y, &winX, &winY);
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = winX;
        input.mi.dy = winY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        SendInput(1, &input, sizeof(INPUT));
    }

    DWORD clickFlag = 0;
    std::string btn(button);
    if (btn == "left" || btn == "primary") clickFlag = MOUSEEVENTF_LEFTCLICK;
    else if (btn == "middle") clickFlag = MOUSEEVENTF_MIDDLECLICK;
    else if (btn == "right" || btn == "secondary") clickFlag = MOUSEEVENTF_RIGHTCLICK;
    else return false;

    for (int i = 0; i < clicks; i++) {
        if (!FailSafeCheck()) break;

        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = clickFlag;
        SendInput(1, &input, sizeof(INPUT));

        if (i < clicks - 1) {
            std::this_thread::sleep_for(std::chrono::duration<double>(interval));
        }
    }

    HandlePause(pause);
    return true;
}

// 移动鼠标到指定位置（relative: 是否相对移动）
extern "C" __declspec(dllexport) bool MoveMouseTo(int x, int y, bool relative, bool pause) {
    if (!FailSafeCheck()) return false;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;

    if (!relative) {
        int winX, winY;
        ToWindowsCoordinates(x, y, &winX, &winY);
        input.mi.dx = winX;
        input.mi.dy = winY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    }
    else {
        // 相对移动
        input.mi.dx = x;
        input.mi.dy = y;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
    }

    SendInput(1, &input, sizeof(INPUT));
    HandlePause(pause);
    return true;
}

// ===================== 键盘操作函数 =====================
// 键盘按键按下
extern "C" __declspec(dllexport) bool KeyDown(const char* key, bool pause) {
    if (!FailSafeCheck()) return false;

    std::string keyStr(key);
    if (KEYBOARD_MAPPING.find(keyStr) == KEYBOARD_MAPPING.end()) return false;
    uint16_t scanCode = KEYBOARD_MAPPING[keyStr];

    DWORD keyFlags = KEYEVENTF_SCANCODE;
    int insertedEvents = 0;
    int expectedEvents = 1;

    // 方向键特殊处理（NumLock开启时需额外发送0xE0）
    if (keyStr == "up" || keyStr == "left" || keyStr == "down" || keyStr == "right") {
        keyFlags |= KEYEVENTF_EXTENDEDKEY;
        if (GetKeyState(VK_NUMLOCK) & 0x0001) { // NumLock开启
            expectedEvents = 2;
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = 0xE0;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            insertedEvents += SendInput(1, &input, sizeof(INPUT));
        }
    }

    // 发送按键按下事件
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = keyFlags;
    insertedEvents += SendInput(1, &input, sizeof(INPUT));

    HandlePause(pause);
    return insertedEvents == expectedEvents;
}

// 键盘按键抬起
extern "C" __declspec(dllexport) bool KeyUp(const char* key, bool pause) {
    if (!FailSafeCheck()) return false;

    std::string keyStr(key);
    if (KEYBOARD_MAPPING.find(keyStr) == KEYBOARD_MAPPING.end()) return false;
    uint16_t scanCode = KEYBOARD_MAPPING[keyStr];

    DWORD keyFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    int insertedEvents = 0;
    int expectedEvents = 1;

    if (keyStr == "up" || keyStr == "left" || keyStr == "down" || keyStr == "right") {
        keyFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    // 发送按键抬起事件
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = keyFlags;
    insertedEvents += SendInput(1, &input, sizeof(INPUT));

    // 方向键NumLock额外处理
    if (keyStr == "up" || keyStr == "left" || keyStr == "down" || keyStr == "right") {
        if (GetKeyState(VK_NUMLOCK) & 0x0001) {
            expectedEvents = 2;
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = 0xE0;
            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            insertedEvents += SendInput(1, &input, sizeof(INPUT));
        }
    }

    HandlePause(pause);
    return insertedEvents == expectedEvents;
}

// 按键点击（presses: 次数, interval: 间隔秒数）
extern "C" __declspec(dllexport) bool PressKey(const char* key, int presses, double interval, bool pause) {
    if (!FailSafeCheck() || presses <= 0) return false;

    int completed = 0;
    for (int i = 0; i < presses; i++) {
        if (KeyDown(key, false) && KeyUp(key, false)) {
            completed++;
        }
        if (i < presses - 1) {
            std::this_thread::sleep_for(std::chrono::duration<double>(interval));
        }
    }

    HandlePause(pause);
    return completed == presses;
}

// 输入字符串（修复大小写和特殊符号问题）
extern "C" __declspec(dllexport) bool TypeWrite(const char* message, double interval, bool pause) {
    if (!FailSafeCheck()) return false;

    std::string msg(message);
    for (char c : msg) {
        bool needShift = false;
        std::string keyStr;

        // 处理大写字母（需要Shift）
        if (c >= 'A' && c <= 'Z') {
            needShift = true;
            keyStr = std::tolower(c); // 转为小写字母键名
        }
        // 处理需要Shift的特殊符号（对应键盘上方数字键的符号）
        else if (c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '^' || c == '&' || c == '*' || c == '(' || c == ')') {
            needShift = true;
            switch (c) {
            case '!': keyStr = "1"; break;
            case '@': keyStr = "2"; break;
            case '#': keyStr = "3"; break;
            case '$': keyStr = "4"; break;
            case '%': keyStr = "5"; break;
            case '^': keyStr = "6"; break;
            case '&': keyStr = "7"; break;
            case '*': keyStr = "8"; break;
            case '(': keyStr = "9"; break;
            case ')': keyStr = "0"; break;
            }
        }
        // 处理其他符号
        else if (c == '_') {
            needShift = true;
            keyStr = "-"; // 下划线是Shift+减号
        }
        else if (c == '+') {
            needShift = true;
            keyStr = "="; // 加号是Shift+等号
        }
        else if (c == '{') {
            needShift = true;
            keyStr = "["; // 左大括号是Shift+左中括号
        }
        else if (c == '}') {
            needShift = true;
            keyStr = "]"; // 右大括号是Shift+右中括号
        }
        else if (c == '|') {
            needShift = true;
            keyStr = "\\"; // 竖线是Shift+反斜杠
        }
        else if (c == ':') {
            needShift = true;
            keyStr = ";"; // 冒号是Shift+分号
        }
        else if (c == '"') {
            needShift = true;
            keyStr = "'"; // 双引号是Shift+单引号
        }
        else if (c == '<') {
            needShift = true;
            keyStr = ","; // 小于号是Shift+逗号
        }
        else if (c == '>') {
            needShift = true;
            keyStr = "."; // 大于号是Shift+句号
        }
        else if (c == '?') {
            needShift = true;
            keyStr = "/"; // 问号是Shift+斜杠
        }
        else if (c == '~') {
            needShift = true;
            keyStr = "`"; // 波浪号是Shift+反引号
        }
        // 普通字符直接映射
        else {
            keyStr = std::string(1, c);
        }

        // 检查键名是否存在
        if (KEYBOARD_MAPPING.find(keyStr) == KEYBOARD_MAPPING.end()) {
            return false; // 未找到键名，输入失败
        }

        // 若需要Shift，先按下Shift
        if (needShift) {
            KeyDown("shift", false);
        }

        // 按下并抬起对应键
        if (!PressKey(keyStr.c_str(), 1, 0, false)) {
            return false;
        }

        // 若需要Shift，抬起Shift
        if (needShift) {
            KeyUp("shift", false);
        }

        // 间隔时间
        std::this_thread::sleep_for(std::chrono::duration<double>(interval));
        if (!FailSafeCheck()) break;
    }

    HandlePause(pause);
    return true;
}

// DLL入口（初始化方向键扫描码）
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        InitDirectionKeyCodes();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}