import math
import os

FX_SHIFT = 8
FX_ONE = 1 << FX_SHIFT

N = 256
RECIP_N = 512

def fx(v):
    return int(v * FX_ONE)

os.makedirs("include", exist_ok=True)
os.makedirs("source", exist_ok=True)

with open("include/trig_lut.h", "w") as f:
    f.write("#ifndef TRIG_LUT_H\n#define TRIG_LUT_H\n")
    f.write("#include \"global.h\"\n\n")
    f.write("extern const int sin_tab[256];\n")
    f.write("extern const int cos_tab[256];\n\n")
    f.write("#endif\n")

with open("source/trig_lut.c", "w") as f:
    f.write('#include "trig_lut.h"\n\n')

    f.write("ALIGN4 const int sin_tab[256] = {\n")
    for i in range(N):
        angle = i * (2.0 * math.pi / N)
        s = fx(math.sin(angle))
        f.write(f"    {s},\n")
    f.write("};\n\n")

    f.write("ALIGN4 const int cos_tab[256] = {\n")
    for i in range(N):
        angle = i * (2.0 * math.pi / N)
        c = fx(math.cos(angle))
        f.write(f"    {c},\n")
    f.write("};\n")

with open("include/recip_lut.h", "w") as f:
    f.write("#ifndef RECIP_LUT_H\n#define RECIP_LUT_H\n")
    f.write("#include \"global.h\"\n\n")
    f.write(f"extern const fx recip_tab[{RECIP_N}];\n\n")
    f.write("#endif\n")

with open("source/recip_lut.c", "w") as f:
    f.write('#include "recip_lut.h"\n\n')

    f.write(f"ALIGN4 const fx recip_tab[{RECIP_N}] = {{\n")
    for i in range(RECIP_N):
        if i == 0:
            val = 0x7FFFFFFF
        else:
            val = (FX_ONE * FX_ONE) // i
        f.write(f"    {val},\n")
    f.write("};\n")

print("Generated trig_lut.h, trig_lut.c, recip_lut.h, recip_lut.c")
