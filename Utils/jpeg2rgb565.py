from PIL import Image
import numpy as np

def jpeg_to_c_array(input_image_path, output_file_path, width=200, height=120):
    """
    将JPEG图片压缩并转换为C风格的unsigned short二维数组
    
    参数:
        input_image_path: 输入的JPEG图片路径
        output_file_path: 输出的C代码文件路径
        width: 目标宽度 (默认100)
        height: 目标高度 (默认100)
    """
    
    try:
        # 打开并转换图片
        with Image.open(input_image_path) as img:
            # 转换为RGB模式（确保处理彩色图片）
            img = img.convert('RGB')
            
            # 调整图片大小
            resized_img = img.resize((width, height), Image.Resampling.LANCZOS)
            
            # 转换为numpy数组
            img_array = np.array(resized_img)
            
            # 将RGB888转换为RGB565 (unsigned short)
            # RGB565格式: R(5位) G(6位) B(5位)
            rgb565_array = convert_to_rgb565(img_array)
            
            # 生成C代码
            generate_c_code(rgb565_array, output_file_path, width, height)
            
            print(f"转换成功！输出文件: {output_file_path}")
            print(f"图片尺寸: {width}x{height}")
            
    except Exception as e:
        print(f"错误: {e}")

def convert_to_rgb565(rgb_array):
    """
    将RGB888数组转换为RGB565格式的unsigned short数组
    """
    r = (rgb_array[:, :, 0] >> 3).astype(np.uint16)  # 5位红色
    g = (rgb_array[:, :, 1] >> 2).astype(np.uint16)  # 6位绿色
    b = (rgb_array[:, :, 2] >> 3).astype(np.uint16)  # 5位蓝色
    
    # 组合成RGB565格式
    rgb565 = (r << 11) | (g << 5) | b
    return rgb565

def generate_c_code(array, output_path, width, height):
    """
    生成C风格的二维数组代码
    """
    with open(output_path, 'w', encoding='utf-8') as f:
        # 写入文件头注释
        f.write("/*\n")
        f.write(" * 自动生成的图片数据\n")
        f.write(" * 格式: RGB565 (unsigned short)\n")
        f.write(f" * 尺寸: {width} x {height}\n")
        f.write(" */\n\n")
        
        # 写入数组定义
        f.write(f"const unsigned short image_data[{height} * {width}] = {{\n")
        
        # 写入每一行数据
        for i in range(height):
            f.write("    ")
            for j in range(width):
                # 写入像素值（十六进制格式）
                f.write(f"0x{array[i, j]:04X}")
                if j < width - 1:
                    f.write(", ")
            f.write("")
            if i < height - 1:
                f.write(",")
            f.write("\n")
        
        f.write("};\n")

def main():
    # 使用示例
    input_image = input("请输入JPEG图片路径: ").strip().strip('"')
    output_file = "code.txt"
    
    # 如果用户没有输入，使用默认值
    if not input_image:
        input_image = "input.jpg"  # 默认输入文件名
    
    # 执行转换
    jpeg_to_c_array(input_image, output_file)

if __name__ == "__main__":
    main()