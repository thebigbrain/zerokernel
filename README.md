不同阶段对应不同的tag标签，如

- [从零实现OS内核，我与AI的对话（一）](documents/从零实现OS内核，我与AI的对话（一）.md) [tag-001](https://github.com/thebigbrain/zerokernel/releases/tag/tag-001)
- [从零实现OS内核，我与AI的对话（二）](documents/从零实现OS内核，我与AI的对话（二）.md) [tag-002](https://github.com/thebigbrain/zerokernel/releases/tag/tag-002)

- [从零实现OS内核，我与AI的对话（三）](documents/从零实现OS内核，我与AI的对话（三）.md) [tag-003](https://github.com/thebigbrain/zerokernel/releases/tag/tag-003)

- [从零实现OS内核，我与AI的对话（四）](documents/从零实现OS内核，我与AI的对话（四）.md) [tag-004](https://github.com/thebigbrain/zerokernel/releases/tag/tag-004)

**每一个tag都是确保可运行的。**

#### 创建构建目录

```bash
mkdir build
cd build
```

#### 配置项目 (指定使用 Visual Studio 编译器)

```bash
cmake .. -G "Visual Studio 17 2022" -A x64
```

#### 编译

```bash
cmake --build . --config Debug
```
