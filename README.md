# 创建构建目录

mkdir build
cd build

# 配置项目 (指定使用 Visual Studio 编译器)

cmake .. -G "Visual Studio 17 2022" -A x64

# 编译

cmake --build . --config Debug
