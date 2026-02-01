import struct
import sys
import os
from enum import IntEnum

# --- 1. 与内核同步的格式定义 ---


class SectionType(IntEnum):
    ROOT_TASK = 1
    DRIVER = 2
    CONFIG = 3
    DATA = 4


ZIMG_MAGIC = 0xDEADBEEF

# 物理布局约定
# 注意：这些地址应与内核物理内存管理器的保留区域对齐
ROOT_PHYS_ADDR = 0x1000000  # 16MB
CONF_PHYS_ADDR = 0x2000000  # 32MB
DRV_BASE_ADDR = 0x3000000  # 48MB

# 结构体格式字符串 (小端序)
# ZImgHeader: magic(I), ver(I), head_size(I), sec_cnt(I), entry_off(Q), conf_phys(Q), mem_req(Q)
HEADER_FMT = "<IIIIQQQ"
# ZImgSection: name(8s), type(I), offset(I), dest(Q), size(I)
SECTION_FMT = "<8sIIQI"


# --- 2. 核心逻辑 ---
def get_pe_entry(filename):
    with open(filename, "rb") as f:
        data = f.read(1024)
        if data[:2] != b"MZ":
            return 0

        # 获取 PE Header 偏移
        pe_ptr = struct.unpack("<I", data[0x3C:0x40])[0]

        # 验证 PE 签名
        if data[pe_ptr : pe_ptr + 4] != b"PE\0\0":
            return 0

        # AddressOfEntryPoint 在可选头偏移 0x28 处
        # 对于 64 位 PE，可选头紧跟在 File Header (24 bytes) 之后
        entry_rva = struct.unpack("<I", data[pe_ptr + 0x28 : pe_ptr + 0x2C])[0]

        # --- 调试打印 ---
        print(f"[Build] Found PE Entry RVA: 0x{entry_rva:X}")

        return entry_rva


def build_zimg(output_path, root_bin, extras):
    """
    合成最终镜像
    """
    components = []

    # 1. 添加 RootTask
    if not os.path.exists(root_bin):
        print(f"[Error] RootTask not found: {root_bin}")
        return False
    components.append(
        {
            "name": "ROOT",
            "type": SectionType.ROOT_TASK,
            "path": root_bin,
            "phys": ROOT_PHYS_ADDR,
        }
    )

    # 2. 添加额外组件 (如驱动或配置)
    for i, path in enumerate(extras):
        if not os.path.exists(path):
            print(f"[Warning] Extra component not found: {path}, skipping.")
            continue

        ext = os.path.splitext(path)[1].lower()
        name = (
            os.path.basename(path).replace(".bin", "").replace(".conf", "").upper()[:8]
        )

        # 根据后缀或索引分配类型和地址
        if ext == ".conf":
            s_type = SectionType.CONFIG
            phys = CONF_PHYS_ADDR
        else:
            s_type = SectionType.DRIVER
            phys = DRV_BASE_ADDR + (i * 0x200000)  # 每个驱动间隔 2MB

        components.append({"name": name, "type": s_type, "path": path, "phys": phys})

    # 3. 计算头部大小
    header_size = struct.calcsize(HEADER_FMT)
    section_table_size = len(components) * struct.calcsize(SECTION_FMT)
    total_header_size = header_size + section_table_size

    # 4. 构建数据块和段表
    current_offset = total_header_size
    section_table_bin = b""
    payload_bin = b""

    for comp in components:
        with open(comp["path"], "rb") as f:
            data = f.read()
            size = len(data)

            # 构造段条目
            section_table_bin += struct.pack(
                SECTION_FMT,
                comp["name"].encode("ascii").ljust(8, b"\0"),
                int(comp["type"]),
                current_offset,
                comp["phys"],
                size,
            )

            payload_bin += data
            current_offset += size

    root_entry = get_pe_entry(root_bin)
    if root_entry % 2 != 0:
        print(f"[Warning] Misaligned Entry RVA 0x{root_entry:X} detected!")
        # root_entry = root_entry & ~0xF  # 暂时不要硬改，先看为什么不对

    # 5. 构造全局文件头
    header_bin = struct.pack(
        HEADER_FMT,
        ZIMG_MAGIC,
        1,  # version
        total_header_size,
        len(components),
        root_entry,  # root_entry_off (默认 0)
        CONF_PHYS_ADDR,  # config_phys (基准配置地址)
        128 * 1024 * 1024,  # memory_required (128MB)
    )

    # 6. 写入文件
    with open(output_path, "wb") as f:
        f.write(header_bin)
        f.write(section_table_bin)
        f.write(payload_bin)

    print(f"Successfully built: {output_path}")
    print(f"Total Sections: {len(components)}")
    for c in components:
        print(
            f"  - {c['name']} -> Phys: 0x{c['phys']:X}, Size: {os.path.getsize(c['path'])} bytes"
        )
    return True


# --- 3. 命令行接口 ---

if __name__ == "__main__":
    # 参数 1: 输出镜像路径
    # 参数 2: RootTask.bin 路径
    # 参数 3+: 其他 .bin 或 .conf 文件
    if len(sys.argv) < 3:
        print(
            "Usage: python build_zimg.py <out.img> <root.bin> [extra1 extra2...]",
            len(sys.argv),
            sys.argv,
        )
        sys.exit(1)

    out_img = sys.argv[1]
    root_bin = sys.argv[2]
    extra_files = sys.argv[3:]

    if build_zimg(out_img, root_bin, extra_files):
        sys.exit(0)
    else:
        sys.exit(1)
