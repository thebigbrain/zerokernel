import os

# 约定布局表 (以字节为单位)
LAYOUT = {
    "kernel.exe": 0x0,
    "root_task.bin": 0x1000000,  # 16MB 处
    "kbd_driver.bin": 0x3000000,  # 48MB 处
    "sys_init.cfg": 0x2000000,  # 32MB 处 (配置文件)
}


def pack():
    full_image = bytearray(128 * 1024 * 1024)  # 预留 128MB 镜像空间

    for filename, offset in LAYOUT.items():
        if os.path.exists(filename):
            with open(filename, "rb") as f:
                data = f.read()
                full_image[offset : offset + len(data)] = data
                print(f"Packed {filename} at {hex(offset)}")

    with open("OS_FULL_PHYSICAL.img", "wb") as f:
        f.write(full_image)


if __name__ == "__main__":
    pack()
