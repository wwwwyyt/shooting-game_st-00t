#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_bitmap_font.py
==================

从系统字体（TTF/OTF）或点阵字体（BDF）生成复古游戏风格的位图字库。

输出：
    <prefix>.bmp   单张位图字库，字符按等宽格子（默认 16x16）排布
    <prefix>.csv   字符 -> 图集坐标 对照表（可选 xlsx）

用法：
    # TTF / OTF
    python gen_bitmap_font.py chars.txt -o build/font --font "SourceHanSerifSC-Regular.otf"

    # BDF 点阵字体（例如文泉驿点阵宋体转换来的 wqy.bdf）
    python gen_bitmap_font.py chars.txt -o build/font --font wqy.bdf

常见字体名 / 路径：
    Windows : C:/Windows/Fonts/simsun.ttc        (宋体)
              C:/Windows/Fonts/msyh.ttc          (微软雅黑)
              C:/Windows/Fonts/simhei.ttf        (黑体)
    macOS   : /System/Library/Fonts/Songti.ttc
    Linux   : /usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc
              /usr/share/fonts/adobe-source-han-serif/SourceHanSerifSC-Regular.otf

依赖：
    pip install Pillow
    pip install openpyxl        # 仅当需要输出 xlsx 时
"""

import argparse
import csv
import math
import sys
import unicodedata
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("缺少 Pillow 库，请先运行： pip install Pillow")


# --------------------------------------------------------------------------- #
#  常量
# --------------------------------------------------------------------------- #

# ASCII 可打印字符：0x20(空格) ~ 0x7E(~)，共 95 个
ASCII_PRINTABLE = [chr(c) for c in range(0x20, 0x7F)]

SKIP_CHARS = {"\r", "\n", "\t", "\ufeff", "\u200b", "\u200c", "\u200d"}


# --------------------------------------------------------------------------- #
#  命令行参数
# --------------------------------------------------------------------------- #

def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="从系统字体生成复古游戏风格的位图字库（单 BMP + 对照表）",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("text_file",
                        help="包含所需字符的文本文件（UTF-8 编码）")
    parser.add_argument("-o", "--output", default="font",
                        help="输出文件前缀，默认 font（生成 font.bmp / font.csv）")
    parser.add_argument("--font", default="simsun.ttc",
                        help="字体文件完整路径、系统字体名，或 .bdf 点阵字体，默认 simsun.ttc")
    parser.add_argument("--font-index", type=int, default=0,
                        help="ttc / otc 字体集合中的索引，默认 0")
    parser.add_argument("--size", type=int, default=16,
                        help="每个全角字符格子的边长（像素），默认 16")
    parser.add_argument("--cols", type=int, default=16,
                        help="图集每行放多少个格子，默认 16")
    parser.add_argument("--threshold", type=int, default=128,
                        help="灰度二值化阈值 0-255，默认 128（仅对 TTF/OTF 有效）")
    parser.add_argument("--baseline", type=float, default=None,
                        help="手动指定基线 y 坐标（相对格子顶部），默认自动计算（仅对 TTF/OTF 有效）")
    parser.add_argument("--sort", choices=("codepoint", "frequency"),
                        default="codepoint",
                        help="字符排序方式：codepoint=按码位（默认），frequency=按首次出现顺序")
    parser.add_argument("--table", choices=("csv", "xlsx", "both", "none"),
                        default="csv",
                        help="对照表输出格式，默认 csv")
    parser.add_argument("--bit-depth", choices=("1", "8"), default="1",
                        help="BMP 位深：1=单色（默认，体积最小），8=灰度")
    parser.add_argument("--preview", action="store_true",
                        help="额外输出 4 倍放大的 PNG 预览图，便于肉眼检查")
    parser.add_argument("--no-ascii", action="store_true",
                        help="不强制包含 ASCII 可打印字符（默认会强制包含）")
    args = parser.parse_args(argv)

    if args.size < 1:
        parser.error("--size 必须 >= 1")
    if args.cols < 1:
        parser.error("--cols 必须 >= 1")
    if not 0 <= args.threshold <= 255:
        parser.error("--threshold 必须在 0~255 之间")

    return args


# --------------------------------------------------------------------------- #
#  字符收集
# --------------------------------------------------------------------------- #

def load_characters(path: str, sort_mode: str, ensure_ascii: bool = True):
    """
    读取文本，去重，过滤控制字符，返回排好序的字符列表。

    ensure_ascii=True 时，无论文本中是否出现，都会强制包含
    全部 ASCII 可打印字符（0x20~0x7E，共 95 个）。
    """
    try:
        text = Path(path).read_text(encoding="utf-8")
    except FileNotFoundError:
        sys.exit(f"找不到文本文件：{path}")
    except UnicodeDecodeError:
        sys.exit(f"文本文件 {path} 不是 UTF-8 编码，请先转换编码。")

    seen = set()
    first_seen = {}

    # --- 1) 先从文本里收集 ---
    for i, ch in enumerate(text):
        if ch in seen or ch in SKIP_CHARS:
            continue
        # 保留普通空格；其余不可打印字符（含各类控制符）丢弃
        if ch != " " and not ch.isprintable():
            continue
        seen.add(ch)
        first_seen[ch] = i

    # --- 2) 强制补齐 ASCII 可打印字符 ---
    if ensure_ascii:
        base = len(text) + 1
        for j, ch in enumerate(ASCII_PRINTABLE):
            if ch not in seen:
                seen.add(ch)
                first_seen[ch] = base + j

    chars = list(seen)
    if sort_mode == "frequency":
        chars.sort(key=lambda c: first_seen[c])
    else:
        chars.sort(key=ord)
    return chars


# --------------------------------------------------------------------------- #
#  TTF / OTF 渲染路径
# --------------------------------------------------------------------------- #

def measure_vertical_extent(chars, font):
    """
    遍历所有字符，测出相对基线的最大上伸 (ascent) 与最大下伸 (descent)。
    ascent  = 基线以上墨迹最高处的像素数（正数）
    descent = 基线以下墨迹最低处的像素数（正数）
    """
    max_ascent = 0.0
    max_descent = 0.0
    for ch in chars:
        try:
            bbox = font.getbbox(ch, anchor="ls")
        except Exception:
            continue
        if not bbox:
            continue
        max_ascent = max(max_ascent, -bbox[1])
        max_descent = max(max_descent, bbox[3])
    return max_ascent, max_descent


def fit_font_to_cell(chars, font_spec, index, cell, min_size=4):
    """
    加载字体；如果所有字形加起来的墨迹高度放不进 `cell`，
    就按比例缩小字号，直到放得下。

    返回 (font, actual_size, max_ascent, max_descent)。
    """
    size = cell
    while size >= min_size:
        font = ImageFont.truetype(font_spec, size, index=index)
        asc, desc = measure_vertical_extent(chars, font)
        if asc + desc <= cell:
            return font, size, asc, desc
        needed = int(size * cell / (asc + desc) * 0.98)
        size = needed if needed < size else size - 1

    # 到最小字号还放不下，只能接受
    font = ImageFont.truetype(font_spec, min_size, index=index)
    asc, desc = measure_vertical_extent(chars, font)
    return font, min_size, asc, desc


def render_glyph(ch: str, font, cell: int, baseline: float, threshold: int) -> Image.Image:
    """
    把单个字符渲染到 cell x cell 的灰度图上，并做二值化。
    窄字符（ASCII 等）按字宽水平居中；全角字符刚好铺满格子。
    """
    img = Image.new("L", (cell, cell), 0)
    draw = ImageDraw.Draw(img)

    try:
        advance = font.getlength(ch)
    except Exception:
        advance = float(cell)

    x = (cell - advance) / 2.0
    draw.text((x, baseline), ch, font=font, fill=255, anchor="ls")

    lut = [0] * threshold + [255] * (256 - threshold)
    return img.point(lut)


def build_atlas(chars, font, args):
    """TTF/OTF 分支：逐字渲染到图集。baseline 必须已在 args 中设置。"""
    cell = args.size
    cols = args.cols
    rows = math.ceil(len(chars) / cols)

    atlas = Image.new("L", (cols * cell, rows * cell), 0)
    baseline = args.baseline
    if baseline is None:
        # 理论上不会走到这里，main() 里已经算好；给个兜底值
        baseline = cell * 0.8

    records = []
    for i, ch in enumerate(chars):
        col = i % cols
        row = i // cols
        px, py = col * cell, row * cell

        glyph = render_glyph(ch, font, cell, baseline, args.threshold)
        atlas.paste(glyph, (px, py))

        records.append({
            "index": i,
            "char": ch,
            "unicode": "U+{:04X}".format(ord(ch)),
            "eaw": unicodedata.east_asian_width(ch),
            "pixel_x": px,
            "pixel_y": py,
            "col": col,
            "row": row,
            "width": cell,
            "height": cell,
        })

    return atlas, records, rows, baseline


# --------------------------------------------------------------------------- #
#  BDF 点阵字体路径
# --------------------------------------------------------------------------- #

def parse_bdf(path):
    """
    解析 BDF 文件，返回 {codepoint: (width, height, xoff, yoff, rows)}。
    rows 是每行像素的 0/1 列表，长度为 BBX 高度。
    """
    glyphs = {}
    with open(path, "r", encoding="latin-1") as f:
        lines = f.read().splitlines()

    i = 0
    while i < len(lines):
        if lines[i].startswith("STARTCHAR"):
            encoding = None
            bbx = None
            rows = []
            i += 1
            while i < len(lines) and not lines[i].startswith("ENDCHAR"):
                l = lines[i].strip()
                if l.startswith("ENCODING"):
                    encoding = int(l.split()[1])
                elif l.startswith("BBX"):
                    parts = l.split()
                    bbx = tuple(int(x) for x in parts[1:5])  # w h xoff yoff
                elif l.startswith("BITMAP"):
                    i += 1
                    while i < len(lines) and not lines[i].startswith("ENDCHAR"):
                        hexrow = lines[i].strip()
                        if hexrow and bbx is not None:
                            val = int(hexrow, 16)
                            nbits = len(hexrow) * 4
                            # BDF 每个 BITMAP 行按字节对齐，用 bbx 宽度截取
                            bits = [(val >> (nbits - 1 - b)) & 1
                                    for b in range(bbx[0])]
                            rows.append(bits)
                        i += 1
                    break
                i += 1
            if encoding is not None and encoding >= 0 and bbx is not None:
                glyphs[encoding] = (*bbx, rows)
        i += 1
    return glyphs


def build_atlas_from_bdf(chars, bdf_path, args):
    """BDF 分支：直接把位图字形拼进图集，跳过渲染与基线计算。"""
    glyphs = parse_bdf(bdf_path)
    if not glyphs:
        sys.exit(f"BDF 文件 {bdf_path} 中没有解析到任何字形。")

    cell = args.size
    cols = args.cols

    # 判断 BDF 的典型字形高度，用于居中放置
    heights = [g[1] for g in glyphs.values() if g[1] > 0]
    if not heights:
        sys.exit("BDF 中没有有效的字形高度信息。")
    ref_h = max(set(heights), key=heights.count)   # 出现次数最多的高度

    print(f"BDF 解析成功  : {len(glyphs)} 个字形，典型高度 {ref_h}px（格子 {cell}px）")
    if ref_h > cell:
        print(f"警告：BDF 字形高度 {ref_h} 超过格子 {cell}，底部可能被裁切。")

    rows_total = math.ceil(len(chars) / cols)
    atlas = Image.new("L", (cols * cell, rows_total * cell), 0)

    missing = []
    records = []
    for i, ch in enumerate(chars):
        col, row = i % cols, i // cols
        px, py = col * cell, row * cell

        cp = ord(ch)
        if cp in glyphs:
            gw, gh, gx, gy, bmp = glyphs[cp]
            # 字形在格子里水平居中、垂直居中
            ox = (cell - gw) // 2
            oy = (cell - gh) // 2
            for ry, bits in enumerate(bmp):
                for rx, v in enumerate(bits):
                    if v:
                        x = px + ox + rx
                        y = py + oy + ry
                        if 0 <= x < atlas.width and 0 <= y < atlas.height:
                            atlas.putpixel((x, y), 255)
        else:
            missing.append(ch)

        records.append({
            "index": i,
            "char": ch,
            "unicode": "U+{:04X}".format(ord(ch)),
            "eaw": unicodedata.east_asian_width(ch),
            "pixel_x": px,
            "pixel_y": py,
            "col": col,
            "row": row,
            "width": cell,
            "height": cell,
        })

    if missing:
        preview = "".join(missing[:30])
        more = "" if len(missing) <= 30 else f" ... 等 {len(missing)} 个"
        print(f"警告：BDF 中缺少 {len(missing)} 个字符，已留空：{preview}{more}")

    return atlas, records, rows_total


# --------------------------------------------------------------------------- #
#  对照表输出
# --------------------------------------------------------------------------- #

def write_csv(records, path: Path):
    with open(path, "w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=list(records[0].keys()))
        writer.writeheader()
        writer.writerows(records)


def write_xlsx(records, path: Path):
    try:
        from openpyxl import Workbook
    except ImportError:
        sys.exit("输出 xlsx 需要 openpyxl，请先运行： pip install openpyxl")

    wb = Workbook()
    ws = wb.active
    ws.title = "font"

    headers = list(records[0].keys())
    ws.append(headers)
    for rec in records:
        ws.append([rec[h] for h in headers])

    for col_idx, header in enumerate(headers, start=1):
        width = max(len(header) + 2, 10)
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = width

    wb.save(path)


# --------------------------------------------------------------------------- #
#  主流程
# --------------------------------------------------------------------------- #

def main(argv=None):
    args = parse_args(argv)

    chars = load_characters(args.text_file, args.sort,
                            ensure_ascii=not args.no_ascii)
    if not chars:
        sys.exit("文本文件中没有找到任何可用字符。")

    is_bdf = args.font.lower().endswith(".bdf")

    # --- 渲染 / 拼图集 ---
    if is_bdf:
        atlas, records, rows = build_atlas_from_bdf(chars, args.font, args)
    else:
        font, actual_size, max_ascent, max_descent = fit_font_to_cell(
            chars, args.font, args.font_index, args.size
        )
        if args.baseline is None:
            # 让整块墨迹在格子里垂直居中
            args.baseline = max_ascent + (args.size - max_ascent - max_descent) / 2.0
        atlas, records, rows, baseline = build_atlas(chars, font, args)

    # --- 输出路径 ---
    out_prefix = Path(args.output)
    if out_prefix.parent and str(out_prefix.parent) != ".":
        out_prefix.parent.mkdir(parents=True, exist_ok=True)
    base = str(out_prefix)

    # --- BMP ---
    bmp_path = Path(base + ".bmp")
    bmp_img = atlas.convert("1") if args.bit_depth == "1" else atlas
    bmp_img.save(bmp_path)

    # --- 对照表 ---
    table_paths = []
    if args.table in ("csv", "both"):
        p = Path(base + ".csv")
        write_csv(records, p)
        table_paths.append(p)
    if args.table in ("xlsx", "both"):
        p = Path(base + ".xlsx")
        write_xlsx(records, p)
        table_paths.append(p)

    # --- 预览图 ---
    preview_path = None
    if args.preview:
        scale = 4
        preview = atlas.resize(
            (atlas.width * scale, atlas.height * scale), Image.NEAREST
        )
        preview_path = Path(base + "_preview.png")
        preview.save(preview_path)

    # --- 汇总 ---
    print(f"字符数        : {len(chars)}")
    print(f"图集尺寸      : {atlas.width} x {atlas.height} px  ({args.cols} 列 x {rows} 行)")
    print(f"格子尺寸      : {args.size} x {args.size} px")
    print(f"字体来源      : {args.font}")
    if is_bdf:
        print(f"渲染方式      : BDF 直接位图（无抗锯齿、无基线计算）")
    else:
        print(f"渲染方式      : TTF/OTF 矢量栅格化")
        print(f"字体尺寸      : {actual_size}px（请求 {args.size}px）")
        print(f"上伸/下伸     : {max_ascent:.1f} / {max_descent:.1f} px")
        print(f"基线 y        : {baseline:.2f}")
    print(f"位深          : {args.bit_depth} bit")
    print(f"ASCII 强制包含: {'是' if not args.no_ascii else '否'}")
    print(f"字库 BMP      : {bmp_path}")
    for p in table_paths:
        print(f"对照表        : {p}")
    if preview_path:
        print(f"预览图        : {preview_path}")


if __name__ == "__main__":
    main()
    
