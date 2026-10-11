#!/usr/bin/env python3
"""
obj2lua.py - 将 OBJ 3D 模型转换为 Lua 数据文件

用法:
    python obj2lua.py model.obj
    python obj2lua.py model.obj -o assets/model.lua
    python obj2lua.py model.obj --no-uv
    python obj2lua.py model.obj --compute-normals

生成的 Lua 文件结构:
    return {
        vertexCount = N,        -- 顶点数量
        stride      = K,        -- 每个顶点的浮点分量数 (3 / 5 / 6 / 8)
        layout      = {...},    -- 每个分量的语义名称
        vertices    = {         -- 扁平浮点数组，每顶点 K 个分量
            v0x, v0y, v0z, v0nx, v0ny, v0nz, v0u, v0v,
            ...
        }
    }
"""

import argparse
import sys
from pathlib import Path


# ---------------------------------------------------------------- 解析

def parse_obj(path):
    """解析 OBJ 文件，返回 (positions, texcoords, normals, triangles)。"""
    positions = []   # [(x, y, z), ...]
    texcoords = []   # [(u, v), ...]
    normals = []     # [(nx, ny, nz), ...]
    triangles = []   # [((vi, ti, ni), (vi, ti, ni), (vi, ti, ni)), ...]

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line_no, raw in enumerate(f, 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            tag = parts[0]

            try:
                if tag == "v":
                    positions.append((float(parts[1]),
                                      float(parts[2]),
                                      float(parts[3])))
                elif tag == "vt":
                    u = float(parts[1])
                    v = float(parts[2]) if len(parts) > 2 else 0.0
                    texcoords.append((u, v))
                elif tag == "vn":
                    normals.append((float(parts[1]),
                                    float(parts[2]),
                                    float(parts[3])))
                elif tag == "f":
                    verts = []
                    for token in parts[1:]:
                        fields = token.split("/")
                        vi = int(fields[0]) if len(fields) > 0 and fields[0] else 0
                        ti = int(fields[1]) if len(fields) > 1 and fields[1] else 0
                        ni = int(fields[2]) if len(fields) > 2 and fields[2] else 0
                        verts.append((vi, ti, ni))
                    if len(verts) < 3:
                        continue
                    # 三角扇形，支持 n-gon
                    for i in range(1, len(verts) - 1):
                        triangles.append((verts[0], verts[i], verts[i + 1]))
                # 其他标签（g, o, s, mtllib, usemtl）忽略
            except (IndexError, ValueError) as e:
                print(f"警告: 第 {line_no} 行解析失败: {line!r} ({e})",
                      file=sys.stderr)

    return positions, texcoords, normals, triangles


def resolve_index(idx, count):
    """OBJ 索引（1 基，可负）→ 0 基。无效返回 -1。"""
    if idx > 0:
        return idx - 1 if idx <= count else -1
    if idx < 0:
        return count + idx if -idx <= count else -1
    return -1


def compute_face_normal(p0, p1, p2):
    """三角面法线（叉积归一化）。"""
    ux, uy, uz = p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]
    vx, vy, vz = p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]
    nx = uy * vz - uz * vy
    ny = uz * vx - ux * vz
    nz = ux * vy - uy * vx
    length = (nx * nx + ny * ny + nz * nz) ** 0.5
    if length > 1e-9:
        return (nx / length, ny / length, nz / length)
    return (0.0, 0.0, 0.0)


# ---------------------------------------------------------------- 展开

def build_vertices(positions, texcoords, normals, triangles,
                   include_normal, include_uv, compute_missing_normals):
    """把三角形展开成扁平顶点列表。"""
    result = []

    for tri in triangles:
        # 收集三角形的三个位置，用于可能的面法线计算
        tri_pos = []
        for (vi, _, _) in tri:
            p_idx = resolve_index(vi, len(positions))
            if p_idx < 0:
                raise ValueError(f"顶点索引越界: {vi}")
            tri_pos.append(positions[p_idx])

        face_normal = None
        if compute_missing_normals and not normals:
            face_normal = compute_face_normal(*tri_pos)

        for (vi, ti, ni) in tri:
            p_idx = resolve_index(vi, len(positions))
            v = list(positions[p_idx])

            if include_normal:
                if ni != 0:
                    n_idx = resolve_index(ni, len(normals))
                    if n_idx < 0:
                        raise ValueError(f"法线索引越界: {ni}")
                    v.extend(normals[n_idx])
                elif face_normal is not None:
                    v.extend(face_normal)
                else:
                    v.extend((0.0, 0.0, 0.0))

            if include_uv:
                if ti != 0:
                    t_idx = resolve_index(ti, len(texcoords))
                    if t_idx < 0:
                        raise ValueError(f"纹理索引越界: {ti}")
                    v.extend(texcoords[t_idx])
                else:
                    v.extend((0.0, 0.0))

            result.append(v)

    return result


# ---------------------------------------------------------------- 输出

LAYOUTS = {
    3: ["x", "y", "z"],
    5: ["x", "y", "z", "u", "v"],
    6: ["x", "y", "z", "nx", "ny", "nz"],
    8: ["x", "y", "z", "nx", "ny", "nz", "u", "v"],
}


def fmt(x, precision):
    """格式化浮点数，去掉多余尾随零，但保证有小数点。"""
    s = f"{x:.{precision}g}"
    if "." not in s and "e" not in s and "E" not in s:
        s += ".0"
    return s


def write_lua(vertices, path, precision, chunk_size):
    n = len(vertices)
    stride = len(vertices[0])
    layout = LAYOUTS.get(stride, [f"c{i}" for i in range(stride)])
    layout_str = ", ".join(f'"{name}"' for name in layout)

    with open(path, "w", encoding="utf-8") as f:
        f.write("-- 由 obj2lua.py 自动生成，请勿手动编辑\n")
        f.write("--\n")
        f.write("-- vertexCount : 顶点总数\n")
        f.write("-- stride      : 每个顶点的浮点分量数\n")
        f.write("-- layout      : 每个分量的语义\n")
        f.write("-- vertices    : 扁平浮点数组，每顶点 stride 个分量，按 layout 顺序\n\n")
        f.write("return {\n")
        f.write(f"    vertexCount = {n},\n")
        f.write(f"    stride      = {stride},\n")
        f.write(f"    layout      = {{ {layout_str} }},\n")
        f.write("    vertices    = {\n")

        for i in range(0, n, chunk_size):
            chunk = vertices[i:i + chunk_size]
            parts = []
            for v in chunk:
                parts.extend(fmt(c, precision) + "," for c in v)
            f.write("        " + " ".join(parts) + "\n")

        f.write("    },\n")
        f.write("}\n")


# ---------------------------------------------------------------- CLI

def main():
    parser = argparse.ArgumentParser(
        description="OBJ 3D 模型 → Lua 数据文件",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("input", help="输入 .obj 文件")
    parser.add_argument("-o", "--output",
                        help="输出 .lua 文件（默认与输入同名）")
    parser.add_argument("--no-normals", action="store_true",
                        help="不输出法线")
    parser.add_argument("--no-uv", action="store_true",
                        help="不输出纹理坐标")
    parser.add_argument("--compute-normals", action="store_true",
                        help="OBJ 缺法线时，用面法线补齐")
    parser.add_argument("--precision", type=int, default=6,
                        help="浮点有效数字（默认 6）")
    parser.add_argument("--chunk", type=int, default=4,
                        help="每行输出的顶点数（默认 4）")
    args = parser.parse_args()

    input_path = Path(args.input)
    if not input_path.is_file():
        print(f"错误: 文件不存在: {input_path}", file=sys.stderr)
        return 1

    output_path = (Path(args.output) if args.output
                   else input_path.with_suffix(".lua"))

    print(f"解析 {input_path}")
    positions, texcoords, normals, triangles = parse_obj(input_path)
    print(f"  v={len(positions)}  vt={len(texcoords)}  "
          f"vn={len(normals)}  tri={len(triangles)}")

    if not triangles:
        print("错误: OBJ 中没有三角形面", file=sys.stderr)
        return 1

    include_normal = not args.no_normals
    include_uv = (not args.no_uv) and len(texcoords) > 0

    if include_normal and not normals and not args.compute_normals:
        print("提示: OBJ 无法线，将输出 (0,0,0)。"
              "如需自动计算请加 --compute-normals")

    try:
        vertices = build_vertices(
            positions, texcoords, normals, triangles,
            include_normal=include_normal,
            include_uv=include_uv,
            compute_missing_normals=args.compute_normals,
        )
    except ValueError as e:
        print(f"错误: {e}", file=sys.stderr)
        return 1

    stride = len(vertices[0])
    print(f"  展开顶点 {len(vertices)} 个，每顶点 {stride} 个 float")

    write_lua(vertices, output_path, args.precision, args.chunk)

    size_kb = output_path.stat().st_size / 1024
    print(f"已写入 {output_path} （{size_kb:.1f} KB）")
    return 0


if __name__ == "__main__":
    sys.exit(main())
