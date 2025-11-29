#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
自动更新 resources.qrc 文件
扫描 images 文件夹中的所有文件，自动生成 Qt 资源文件
"""

import os
import xml.etree.ElementTree as ET
from pathlib import Path


def scan_images_folder(base_path="images"):
    """
    扫描images文件夹，返回所有图片文件的相对路径列表
    
    Args:
        base_path: images文件夹的路径
        
    Returns:
        list: 包含所有图片文件相对路径的列表
    """
    image_extensions = {'.png', '.jpg', '.jpeg', '.bmp', '.gif', '.svg', '.ico'}
    image_files = []
    
    if not os.path.exists(base_path):
        print(f"警告: {base_path} 文件夹不存在！")
        return image_files
    
    # 遍历images文件夹及其子文件夹
    for root, dirs, files in os.walk(base_path):
        for file in files:
            # 检查文件扩展名
            file_ext = os.path.splitext(file)[1].lower()
            if file_ext in image_extensions:
                # 获取相对于项目根目录的路径
                relative_path = os.path.join(root, file)
                # 转换为正斜杠（Qt资源文件格式）
                relative_path = relative_path.replace('\\', '/')
                image_files.append(relative_path)
    
    return sorted(image_files)


def generate_qrc(image_files, output_file="resources.qrc"):
    """
    生成resources.qrc文件
    
    Args:
        image_files: 图片文件路径列表
        output_file: 输出的qrc文件名
    """
    # 创建根元素
    root = ET.Element('RCC')
    
    # 创建qresource元素
    qresource = ET.SubElement(root, 'qresource')
    qresource.set('prefix', '/')
    
    # 添加所有文件
    for image_file in image_files:
        file_element = ET.SubElement(qresource, 'file')
        file_element.text = image_file
    
    # 创建ElementTree对象
    tree = ET.ElementTree(root)
    
    # 美化XML输出
    indent_xml(root)
    
    # 写入文件（使用UTF-8编码）
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        tree.write(f, encoding='unicode', xml_declaration=False)
        f.write('\n')
    
    print(f"✓ 成功生成 {output_file}")
    print(f"✓ 共包含 {len(image_files)} 个资源文件")


def indent_xml(elem, level=0):
    """
    美化XML输出，添加缩进
    
    Args:
        elem: XML元素
        level: 当前缩进级别
    """
    indent = "\n" + "    " * level
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = indent + "    "
        if not elem.tail or not elem.tail.strip():
            elem.tail = indent
        for child in elem:
            indent_xml(child, level + 1)
        if not child.tail or not child.tail.strip():
            child.tail = indent
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = indent


def print_file_tree(image_files):
    """
    打印文件树结构（用于调试）
    
    Args:
        image_files: 图片文件路径列表
    """
    print("\n扫描到的资源文件:")
    print("=" * 60)
    
    current_dir = ""
    for file_path in image_files:
        dir_name = os.path.dirname(file_path)
        file_name = os.path.basename(file_path)
        
        if dir_name != current_dir:
            current_dir = dir_name
            print(f"\n📁 {dir_name}/")
        
        print(f"  ├─ {file_name}")
    
    print("=" * 60)


def backup_existing_qrc(qrc_file="resources.qrc"):
    """
    备份现有的qrc文件
    
    Args:
        qrc_file: qrc文件路径
    """
    if os.path.exists(qrc_file):
        import shutil
        from datetime import datetime
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_file = f"{qrc_file}.backup_{timestamp}"
        shutil.copy2(qrc_file, backup_file)
        print(f"✓ 已备份原文件为: {backup_file}")


def main():
    """
    主函数
    """
    print("=" * 60)
    print("Qt Resources.qrc 自动生成工具")
    print("=" * 60)
    
    # 确保在项目根目录下运行
    if not os.path.exists("images"):
        print("错误: 找不到 images 文件夹！")
        print("请确保在项目根目录下运行此脚本。")
        return
    
    # 扫描images文件夹
    print("\n正在扫描 images 文件夹...")
    image_files = scan_images_folder("images")
    
    if not image_files:
        print("警告: 未找到任何图片文件！")
        return
    
    # 打印文件树
    print_file_tree(image_files)
    
    # 备份现有的qrc文件
    if os.path.exists("resources.qrc"):
        print("\n检测到已存在的 resources.qrc 文件")
        backup_existing_qrc("resources.qrc")
    
    # 生成新的qrc文件
    print("\n正在生成新的 resources.qrc 文件...")
    generate_qrc(image_files, "resources.qrc")
    
    print("\n" + "=" * 60)
    print("完成！")
    print("=" * 60)
    print("\n提示: 请在Qt项目中重新编译资源文件")
    print("命令: cmake --build . 或者重新编译整个项目")


if __name__ == "__main__":
    main()

