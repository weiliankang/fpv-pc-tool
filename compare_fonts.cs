// Compare font_bf_24.png vs font_inav_24.png char blocks - compile & run with csc
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

class FontCompare
{
    static byte[,] LoadBlocks(string path, int fontW, int fontH, int maxIdx)
    {
        using (Bitmap bmp = new Bitmap(path))
        {
            int W = bmp.Width, H = bmp.Height;
            var rect = new Rectangle(0, 0, W, H);
            var data = bmp.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            int stride = data.Stride;
            byte[] bytes = new byte[stride * H];
            Marshal.Copy(data.Scan0, bytes, 0, bytes.Length);
            bmp.UnlockBits(data);

            int xNum = W / fontW;
            int yNum = H / fontH;
            byte[,] blocks = new byte[maxIdx, fontW * fontH];
            for (int x = 0; x < xNum; x++)
            {
                for (int y = 0; y < yNum; y++)
                {
                    int idx = x * yNum + y;
                    if (idx >= maxIdx) continue;
                    int bi = 0;
                    for (int cy = 0; cy < fontH; cy++)
                    {
                        int basep = (y * fontH + cy) * stride + (x * fontW) * 4;
                        for (int cx = 0; cx < fontW; cx++)
                        {
                            int p = basep + cx * 4;
                            int b = bytes[p], g = bytes[p + 1], r = bytes[p + 2], a = bytes[p + 3];
                            blocks[idx, bi++] = (a > 128 && (r + g + b) < 600) ? (byte)1 : (byte)0;
                        }
                    }
                }
            }
            return blocks;
        }
    }

    static void Main(string[] args)
    {
        int fontW = 24, fontH = 36, maxIdx = 1024;
        var bf = LoadBlocks(@"X:\open_project\lkwei_project\fpv-pc-tool\fonts\font_bf_24.png", fontW, fontH, maxIdx);
        var inav = LoadBlocks(@"X:\open_project\lkwei_project\fpv-pc-tool\fonts\font_inav_24.png", fontW, fontH, maxIdx);

        var diffs = new System.Collections.Generic.List<Tuple<int, int>>();
        int px = fontW * fontH;
        for (int i = 0; i < maxIdx; i++)
        {
            int d = 0;
            for (int k = 0; k < px; k++)
                if (bf[i, k] != inav[i, k]) d++;
            if (d > 0) diffs.Add(Tuple.Create(i, d));
        }

        Console.WriteLine("Chars with differences: " + diffs.Count);
        Console.WriteLine();
        Console.WriteLine("=== Top 50 by diff pixels (good visual check points) ===");
        diffs.Sort((a, b) => b.Item2.CompareTo(a.Item2));
        int shown = 0;
        foreach (var t in diffs)
        {
            if (shown++ >= 50) break;
            char c = (t.Item1 >= 0x20 && t.Item1 <= 0x7E) ? (char)t.Item1 : '.';
            Console.WriteLine("idx {0,4} (0x{1:X2}, '{2}')  diffpx: {3}", t.Item1, t.Item1, c, t.Item2);
        }
        Console.WriteLine();
        Console.WriteLine("=== Common ASCII visible range (0x20-0x7E) differences ===");
        var common = diffs.FindAll(t => t.Item1 >= 0x20 && t.Item1 <= 0x7E);
        Console.WriteLine("ASCII visible chars with diff: " + common.Count);
        common.Sort((a, b) => b.Item2.CompareTo(a.Item2));
        shown = 0;
        foreach (var t in common)
        {
            if (shown++ >= 25) break;
            Console.WriteLine("  '{0}' (0x{1:X2})  diff: {2} px", (char)t.Item1, t.Item1, t.Item2);
        }
    }
}
