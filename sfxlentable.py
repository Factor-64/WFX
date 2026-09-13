import re

LENGTHS = {
    "SFX_ENGINE": 2.068,
    "SFX_LASER1": 0.720,
    "SFX_BARRELROLL": 0.625,
    "SFX_START": 2.266,
    "SFX_EXPLOSION": 0.429,
    "SFX_EXPLOSION_BIG": 1.583,
    "SFX_HIT": 0.613,
    "SFX_HPUP": 1.014,
    "SFX_POWERUP": 1.283,
    "SFX_SELECT": 0.232
}

soundbank = "build/soundbank.h"
out_c = "source/sfx_lengths.c"
out_h = "include/sfx_lengths.h"

defines = {}
max_index = 0

with open(soundbank) as f:
    for line in f:
        m = re.match(r"#define\s+(SFX_[A-Za-z0-9_]+)\s+(\d+)", line)
        if m:
            name, idx = m.group(1), int(m.group(2))
            defines[name] = idx
            max_index = max(max_index, idx)

table = ["0"] * (max_index + 1)

for name, idx in defines.items():
    if name in LENGTHS:
        frames = LENGTHS[name]
        table[idx] = f"FRAME_COUNT({frames})"

with open(out_h, "w") as h:
    h.write("#ifndef SFX_LENGTHS\n")
    h.write("#define SFX_LENGTHS\n\n")
    h.write("#define FRAME_COUNT(x) ((int)((x) * 59.7275f))\n\n")
    h.write("extern const unsigned int sfx_lengths[];\n")
    h.write("extern const unsigned int sfx_lengths_count;\n\n")
    h.write("#endif")

with open(out_c, "w") as c:
    c.write('#include "sfx_lengths.h"\n\n')
    c.write("const unsigned int sfx_lengths[] = {\n")
    for i, v in enumerate(table):
        c.write(f"    /* {i} */ {v},\n")
    c.write("};\n\n")
    c.write("const unsigned int sfx_lengths_count = (sizeof(sfx_lengths) / sizeof(sfx_lengths[0]));\n")
