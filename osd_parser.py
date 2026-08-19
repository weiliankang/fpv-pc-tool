# -*- coding: utf-8 -*-
"""
独立 OSD 解析器 —— 复刻 fpv-pc-tool F0ReadPage::parseOsdData(verbose=true)
读取 test.txt（每行: [时间戳] 57 <OSD数据hex，含3字节CRC>），
逐帧打印 visualizeRawOsdData 风格的详细调试信息。

与 debug_tool_qt.exe 完全解耦，纯 Python 实现。

用法:
    python osd_parser.py                 # 逐帧打印全部
    python osd_parser.py --frame 0       # 只打印指定帧(0-based)
    python osd_parser.py --range 0 10    # 打印第0~9帧
    python osd_parser.py --summary       # 只打印每帧一行摘要
"""
import sys, io, re, argparse

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

ROWS, COLS = 20, 53


def parse_osd(osd_payload, verbose=False):
    """
    解析一帧 OSD 数据 → 20x53 字符矩阵。
    osd_payload: bytes，MSPV2头 + payload + CRC（可含也可不含CRC，自动截断）。
    返回 (matrix, parsedCount, info) 并可选打印 verbose 日志。
    """
    osd = osd_payload
    osd_size = len(osd)
    mat = [[0] * COLS for _ in range(ROWS)]
    parsed_count = 0

    out = []
    if verbose:
        out.append(f"visualizeRawOsdData: 总数据 {osd_size} bytes")
        out.append(f"OSD包找到: offset=0, dataLen={osd_size}, osdSize={osd_size}")
        out.append("osd数据头: " + ' '.join(f"{b:02x}" for b in osd))

    # ---- 跳过 MSP V2 头 ----
    data_start = 0
    max_parse_end = osd_size
    msp_len = 0
    cmd = 0
    crc = False

    if osd_size >= 9 and osd[0] == 0x24 and osd[1] == 0x58 and osd[2] == 0x3E:
        data_start = 8
        msp_len = osd[6] | (osd[7] << 8)
        cmd = osd[4] | (osd[5] << 8)
        if msp_len > 0 and data_start + msp_len < max_parse_end:
            max_parse_end = data_start + msp_len  # 截掉末尾 CRC
        crc = (data_start + msp_len <= osd_size)
        if verbose:
            out.append(f"跳过 MSP V2 头: cmd=0x{cmd:04x} len={msp_len} crc={'yes' if crc else 'no'}")

    # ---- 解析 DISPLAYPORT 子命令 ----
    p = data_start
    while p < max_parse_end:
        subcmd = osd[p]

        if subcmd == 0x35:  # 整帧 0x35 [seq][rsv][entry...]
            if p + 3 > max_parse_end:
                break
            ep = p + 3
            while ep + 4 <= max_parse_end:
                elen = osd[ep]
                if elen < 4:
                    break
                if ep + elen > max_parse_end:
                    break
                row, col, raw_attr = osd[ep+1], osd[ep+2], osd[ep+3]
                chars = osd[ep+4:ep+elen]
                if 0 <= row < ROWS and 0 <= col < COLS:
                    for i, ch in enumerate(chars):
                        if col + i < COLS:
                            mat[row][col+i] = ch
                    parsed_count += 1
                ep += elen
            p = ep
        elif subcmd == 0x05:  # 文本 0x05 [row][col][attr][chars...]
            if p + 4 > max_parse_end:
                break
            row, col, raw_attr = osd[p+1], osd[p+2], osd[p+3]
            text_start = p + 4
            text_end = text_start
            while text_end < max_parse_end:
                b = osd[text_end]
                if b in (0x00, 0x05, 0x09, 0x0A, 0x35) or b >= 0x80:
                    break
                text_end += 1
            text_len = text_end - text_start
            if 0 <= row < ROWS:
                for i in range(text_len):
                    if col + i < COLS:
                        mat[row][col+i] = osd[text_start+i]
                parsed_count += 1
            p = max(text_end, p + 4)
        elif subcmd in (0x09, 0x0A, 0x00):
            p += 1
        elif subcmd >= 4:  # 容错: [len][row][col][attr][chars...]
            elen = subcmd
            if p + elen <= max_parse_end:
                text_len = elen - 4
                if text_len > 0 and text_len < 60:
                    row, col, raw_attr = osd[p+1], osd[p+2], osd[p+3]
                    if 0 <= row < ROWS and 0 <= col < COLS:
                        for i in range(text_len):
                            if col + i < COLS:
                                mat[row][col+i] = osd[p+4+i]
                        parsed_count += 1
                    p += elen
                    continue
            p += 1
        else:
            p += 1

    # ---- verbose 打印 OSD 内容 ----
    if verbose:
        out.append(f"解析了 {parsed_count} 个条目")
        out.append("--- OSD 内容 ---")
        for r in range(ROWS):
            line = ''
            has = False
            for c in range(COLS):
                ch = mat[r][c]
                if ch == 0:
                    line += ' '
                elif 32 <= ch <= 126:
                    line += chr(ch)
                    has = True
                else:
                    line += '?'
                    has = True
            if has:
                out.append(f"[{r}] {line}")
        out.append(f"✅ OSD: {parsed_count} 条目 ({osd_size} bytes)")

    return mat, parsed_count, out


def load_frames(path):
    """读 test.txt → [(timestamps, osd_bytes), ...]"""
    frames = []
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        for ln in f:
            ln = ln.strip()
            if not ln or not ln.startswith('['):
                continue
            rb = ln.index(']')
            ts = ln[1:rb].strip()
            body = ln[rb+1:]
            # 提取所有 hex（去时间戳）
            hexes = re.findall(r'\b[0-9A-Fa-f]{2}\b', body)
            if not hexes:
                continue
            data = [int(h, 16) for h in hexes]
            if data and data[0] == 0x57:
                frames.append((ts, bytes(data[1:])))  # 去掉 57 命令号
    return frames


def main():
    ap = argparse.ArgumentParser(description='独立 OSD 解析器（复刻 F0 页 verbose 打印）')
    ap.add_argument('--file', default='test.txt', help='test.txt 路径')
    ap.add_argument('--frame', type=int, default=None, help='只打印指定帧(0-based)')
    ap.add_argument('--range', nargs=2, type=int, default=None, metavar=('START','END'),
                    help='打印 START~END-1 帧')
    ap.add_argument('--summary', action='store_true', help='每帧只打印一行摘要')
    args = ap.parse_args()

    frames = load_frames(args.file)
    print(f"共加载 {len(frames)} 帧 (from {args.file})")

    idxs = list(range(len(frames)))
    if args.frame is not None:
        idxs = [args.frame]
    if args.range is not None:
        s, e = args.range
        idxs = [i for i in range(s, e) if 0 <= i < len(frames)]

    for idx in idxs:
        ts, payload = frames[idx]
        if args.summary:
            mat, cnt, _ = parse_osd(payload, verbose=False)
            # 单行摘要
            r10 = ''.join(chr(c) if 32 <= c <= 126 else '?' for c in mat[10]).rstrip()
            r14 = ''.join(chr(c) if 32 <= c <= 126 else '?' for c in mat[14]).rstrip()
            print(f"帧#{idx:3d} [{ts}] {len(payload)}B 条目={cnt} R10='{r10}' R14='{r14}'")
        else:
            print(f"\n========== 帧#{idx} [{ts}] ==========")
            _, _, out = parse_osd(payload, verbose=True)
            for o in out:
                print(o)


if __name__ == '__main__':
    main()
