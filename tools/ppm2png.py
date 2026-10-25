"""Joins PPM frames side by side into one PNG. No dependencies.
usage: ppm2png.py OUT.png IN1.ppm [IN2.ppm ...]"""
import struct
import sys
import zlib


def read_ppm(path):
    data = open(path, "rb").read()
    parts = data.split(b"\n", 3)
    w, h = map(int, parts[1].split())
    return w, h, parts[3]


def main():
    out, ins = sys.argv[1], sys.argv[2:]
    frames = [read_ppm(p) for p in ins]
    w, h = frames[0][0], frames[0][1]
    gap = 8
    total_w = w * len(frames) + gap * (len(frames) - 1)
    rows = []
    for y in range(h):
        row = bytearray(b"\x00")
        for i, (_, _, px) in enumerate(frames):
            if i:
                row += b"\xff\xff\xff" * gap
            row += px[y * w * 3:(y + 1) * w * 3]
        rows.append(bytes(row))

    def chunk(kind, body):
        c = struct.pack(">I", len(body)) + kind + body
        return c + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", total_w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(b"".join(rows), 9))
    png += chunk(b"IEND", b"")
    open(out, "wb").write(png)


if __name__ == "__main__":
    main()
