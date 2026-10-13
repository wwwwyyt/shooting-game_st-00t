#!/usr/bin/env python3
"""
将 C 枚举 SDL_Scancode 转换为 Lua 表。
用法: python convert_scancode.py <输入文件> > SDL_Scancode.lua
或直接通过标准输入传递内容。
"""

import re
import sys

def parse_enum(text):
    """从 C 枚举文本中提取 (名称, 数值) 列表。"""
    # 去除块注释 /* ... */ 和行注释 //
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    text = re.sub(r'//.*', '', text)
    # 匹配 SDL_SCANCODE_XXX = 数字
    pattern = r'\b(SDL_SCANCODE_[A-Z0-9_]+)\s*=\s*(\d+)'
    return re.findall(pattern, text)

def generate_lua(matches, output_file=None):
    """生成 Lua 表代码。"""
    lines = [
        "-- Auto generated file, do not modified manually.",
        "local SDL_Scancode = {"
    ]
    for name, value in matches:
        lines.append(f"    {name} = {value},")
    lines.append("}")
    lines.append("")
    lines.append("return SDL_Scancode")
    lua_code = "\n".join(lines)
    
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(lua_code)
    else:
        print(lua_code)

def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            content = f.read()
    else:
        content = sys.stdin.read()
    
    matches = parse_enum(content)
    generate_lua(matches)

if __name__ == "__main__":
    main()