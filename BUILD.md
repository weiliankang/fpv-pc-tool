# FPV PC Tool 编译指南

## 环境
- Qt 5.15.2 (MinGW 8.1.0 32-bit)
- 工具链路径：
  - `E:\Qt\5.15.2\mingw81_32\bin`
  - `E:\Qt\Tools\mingw810_32\bin`

## 正确编译方法（重要！）

Shadow build 目录为 `build_test\`，**一切 qmake / make 都在 `build_test\` 根目录执行**。

```bat
cd /d X:\open_project\lkwei_project\fpv-pc-tool\build_test
set PATH=E:\Qt\5.15.2\mingw81_32\bin;E:\Qt\Tools\mingw810_32\bin;%PATH%

:: （可选）首次或需要重新生成 Makefile 时：
qmake ..\debug_tool_qt.pro -spec win32-g++ "CONFIG+=debug"

:: 编译 Debug：
mingw32-make -f Makefile.Debug

:: 编译 Release：
mingw32-make -f Makefile.Release
```

产物：
- Debug：  `build_test\debug\debug_tool_qt.exe`
- Release：`build_test\release\debug_tool_qt.exe`

## ⚠️ 绝对不要这样（会导致路径错乱）

**不要** `cd` 进入 `build_test\debug\` 或 `build_test\release\` 里再跑 qmake / make！

那会在子目录里再生成一套错误的 Makefile，导致：
- moc 输出目录变成 `debug\debug\...`（两级 debug）
- 相对 include `../../../xxx.h` 从错误基准解析，找不到头文件
- 报错形如：`build_test\debug\debug\moc_xxx.cpp: fatal error: 找不到头文件`

如果出现这种报错，修复方法：
```bat
cd /d X:\open_project\lkwei_project\fpv-pc-tool\build_test
:: 删掉 debug / release 子目录里误生成的 Makefile 和 object_script
del debug\Makefile* debug\object_script* 2>nul
del release\Makefile* release\object_script* 2>nul
:: 然后重新从 build_test 根目录编译
mingw32-make -f Makefile.Debug
```

## 完整重建（清理所有中间产物）

```bat
cd /d X:\open_project\lkwei_project\fpv-pc-tool\build_test
mingw32-make -f Makefile.Debug clean
mingw32-make -f Makefile.Debug
```
