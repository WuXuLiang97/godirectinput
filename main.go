package main

import (
	"fmt"
	"syscall"
	"time"
	"unsafe"
)

// 全局变量：加载DLL并绑定导出函数
var (
	// 加载DLL（如果DLL不在同目录，写绝对路径，如："E:/project/directinput.dll"）
	dll, _ = syscall.LoadDLL("directinput.dll")

	// ========== 新增：绑定SetPause函数 ==========
	setPauseProc = dll.MustFindProc("SetPause")

	// ========== 鼠标操作函数 ==========
	// 获取鼠标位置
	getMousePosProc = dll.MustFindProc("GetMousePosition")
	// 鼠标点击
	mouseClickProc = dll.MustFindProc("MouseClick")
	// 移动鼠标
	moveMouseToProc = dll.MustFindProc("MoveMouseTo")
	// 鼠标按下/抬起（可选）
	mouseDownProc = dll.MustFindProc("MouseDown")
	mouseUpProc   = dll.MustFindProc("MouseUp")

	// ========== 键盘操作函数 ==========
	// 按键按下/抬起
	keyDownProc = dll.MustFindProc("KeyDown")
	keyUpProc   = dll.MustFindProc("KeyUp")
	// 点击按键（按下+抬起）
	pressKeyProc = dll.MustFindProc("PressKey")
	// 输入字符串
	typeWriteProc = dll.MustFindProc("TypeWrite")
)

// ========== 新增：封装SetPause函数 ==========
// SetPause 设置C++ DLL中的默认暂停时间（秒）
func SetPause(newPauseValue float64) {
	// Go的float64对应C的double，直接传指针即可
	setPauseProc.Call(
		uintptr(unsafe.Pointer(&newPauseValue)),
	)
}

func SetFailsafe(newFailsafeValue bool) {
	// Go的bool对应C的int，直接传指针即可
	setPauseProc.Call(
		boolToUint(newFailsafeValue),
	)
}

// ------------------------------ 原有封装函数不变 ------------------------------
// GetMousePosition 获取当前鼠标坐标
func GetMousePosition() (int, int) {
	var x, y int
	getMousePosProc.Call(
		uintptr(unsafe.Pointer(&x)),
		uintptr(unsafe.Pointer(&y)),
	)
	return x, y
}

// MoveMouseTo 移动鼠标到指定坐标
// x,y: 目标坐标；relative: true=相对当前位置移动，false=绝对坐标
func MoveMouseTo(x, y int, relative bool) bool {
	ret, _, _ := moveMouseToProc.Call(
		uintptr(x),                    // 目标X坐标
		uintptr(y),                    // 目标Y坐标
		uintptr(boolToUint(relative)), // bool转uintptr（C中bool是int）
		boolToUint(true),              // 修复：用工具函数转bool→uintptr
	)
	// 返回值非0表示成功
	return ret != 0
}

// MouseClick 鼠标点击
// x,y: 点击坐标（-1表示不移动，点击当前位置）；clicks: 点击次数；interval: 点击间隔（秒）；button: "left"/"middle"/"right"
func MouseClick(x, y int, clicks int, interval float64, button string) bool {
	// Go字符串转C风格字符串（char*）
	btnStr, _ := syscall.BytePtrFromString(button)
	ret, _, _ := mouseClickProc.Call(
		uintptr(x),                         // 点击X坐标
		uintptr(y),                         // 点击Y坐标
		uintptr(clicks),                    // 点击次数
		uintptr(unsafe.Pointer(&interval)), // 间隔秒数（float64指针）
		uintptr(unsafe.Pointer(btnStr)),    // 按键类型（char*）
		boolToUint(true),                   // 修复：用工具函数转bool→uintptr
	)
	return ret != 0
}

// PressKey 点击单个按键（按下+抬起）
// key: 按键名称（如"enter"/"space"/"a"/"f1"）；presses: 点击次数；interval: 间隔秒数
func PressKey(key string, presses int, interval float64) bool {
	keyStr, _ := syscall.BytePtrFromString(key)
	ret, _, _ := pressKeyProc.Call(
		uintptr(unsafe.Pointer(keyStr)),    // 按键名称（char*）
		uintptr(presses),                   // 点击次数
		uintptr(unsafe.Pointer(&interval)), // 间隔秒数
		boolToUint(true),                   // 修复：用工具函数转bool→uintptr
	)
	return ret != 0
}

// TypeWrite 输入字符串（支持字母、数字、符号）
// msg: 要输入的字符串；interval: 每个字符的输入间隔（秒）
func TypeWrite(msg string, interval float64) bool {
	msgStr, _ := syscall.BytePtrFromString(msg)
	ret, _, _ := typeWriteProc.Call(
		uintptr(unsafe.Pointer(msgStr)),    // 要输入的字符串（char*）
		uintptr(unsafe.Pointer(&interval)), // 字符间隔秒数
		boolToUint(true),                   // 修复：用工具函数转bool→uintptr
	)
	return ret != 0
}

// boolToUint Go的bool转C的bool（C中bool是int，true=1，false=0）
func boolToUint(b bool) uintptr {
	if b {
		return 1
	}
	return 0
}

// ------------------------------ 测试主函数 ------------------------------
func main() {

	// 确保程序结束时释放DLL
	defer dll.Release()

	// 示例1：设置为0秒（无暂停）
	SetPause(0.0)

	SetFailsafe(false)

	// ========== 测试1：获取鼠标位置 ==========
	x, y := GetMousePosition()
	fmt.Printf("当前鼠标位置：X=%d, Y=%d\n", x, y)
	// time.Sleep(1 * time.Second)

	// // ========== 测试2：移动鼠标到(800, 500) ==========
	// fmt.Println("移动鼠标到(800, 500)...")
	MoveMouseTo(100, 100, true)

	// ========== 测试3：左键单击 ==========
	fmt.Println("左键单击(800, 500)...")
	MouseClick(800, 500, 1, 0.1, "left")
	time.Sleep(1 * time.Second)

	// ========== 测试4：按回车键 ==========
	fmt.Println("按回车键...")
	PressKey("enter", 1, 0)
	time.Sleep(1 * time.Second)

	// ========== 测试5：输入字符串 ==========
	fmt.Println("输入字符串：Hello Golang + C++ DLL!")
	TypeWrite("Hello Golang + C++ DLL!", 0.1)
	time.Sleep(1 * time.Second)

	// // ========== 测试6：右键点击当前位置 ==========
	// fmt.Println("右键点击当前位置...")
	// MouseClick(-1, -1, 1, 0, "right")

	fmt.Println("所有操作执行完成！")
}
