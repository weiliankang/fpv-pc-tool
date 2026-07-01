from PIL import Image
import os

# 读取 font_bf_24.png
font_path = r'E:\000_openclaw\file\debug_tool_qt\fonts\font_bf_24.png'
img = Image.open(font_path)
print(f"Font image: {img.size[0]}x{img.size[1]}")

# osdChars1080/1.png (笑脸)
osd1 = Image.open(r'E:\000_openclaw\file\debug_tool_qt\osdChars\1.png')
print(f"osdChars/1.png: {osd1.size[0]}x{osd1.size[1]}")

# 检查 font_bf_24.png 前4个字符 (x * yNum + y 算法)
# yNum = 9216/24 = 384
# idx 0: x=0,y=0  -> QR(0,0,24,24)
# idx 1: x=0,y=1  -> QR(0,24,24,24)
# idx 2: x=0,y=2  -> QR(0,48,24,24)
# idx 3: x=0,y=3  -> QR(0,72,24,24)

# 检查 font_bf_24.png 前4个字符 (y * xNum + x 算法)
# xNum = 96/24 = 4
# idx 0: y=0,x=0  -> QR(0,0,24,24)
# idx 1: y=0,x=1  -> QR(24,0,24,24)
# idx 2: y=0,x=2  -> QR(48,0,24,24)
# idx 3: y=0,x=3  -> QR(72,0,24,24)

print("\n=== 检查 x*yNum+y 算法 (SDK formfcosd) ===")
outdir = r'E:\000_openclaw\file\debug_tool_qt\temp_check'
os.makedirs(outdir, exist_ok=True)

yNum = img.size[1] // 24  # 384
xNum = img.size[0] // 24  # 4

for idx in [0, 1, 65, 133]:
    x = idx // yNum
    y = idx % yNum
    crop = img.crop((x*24, y*24, x*24+24, y*24+24))
    crop.save(os.path.join(outdir, f'sdk_{idx}.png'))
    # 统计非全透明像素
    data = crop.getdata()
    opaque = sum(1 for p in data if p[3] > 0)
    print(f"  idx={idx}: x={x}, y={y}, QR({x*24},{y*24},{24},{24}), 非透像素={opaque}")

print("\n=== 检查 y*xNum+x 算法 (列优先) ===")
for idx in [0, 1, 65, 133]:
    y = idx // xNum
    x = idx % xNum
    crop = img.crop((x*24, y*24, x*24+24, y*24+24))
    crop.save(os.path.join(outdir, f'row_{idx}.png'))
    data = crop.getdata()
    opaque = sum(1 for p in data if p[3] > 0)
    print(f"  idx={idx}: y={y}, x={x}, QR({x*24},{y*24},{24},{24}), 非透像素={opaque}")

print(f"\nDone! Check {outdir}")
