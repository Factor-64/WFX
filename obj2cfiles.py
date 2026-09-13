import os
import math
import json
import numpy as np

FX_SHIFT = 8
FX_ONE = 1 << FX_SHIFT
SCALE = 0.3

model_ids = {}
current_id = 0

DEFAULT_JSON = {
    "health": "INT_MAX",
    "damage": 0,
    "color": 0,
    "dx": 0,
    "dy": 0,
    "dz": 0
}

def compute_aabb(vertices):
    xs = [v[0] for v in vertices]
    ys = [v[1] for v in vertices]
    zs = [v[2] for v in vertices]

    return (
        [min(xs), min(ys), min(zs)],
        [max(xs), max(ys), max(zs)]
    )

def load_or_create_json(json_path, raw_verts):
    aabb_min, aabb_max = compute_aabb(raw_verts)
    aabb_data = {
        "type": "aabb",
        "min": [aabb_min[0], aabb_min[1], aabb_min[2]],
        "max": [aabb_max[0], aabb_max[1], aabb_max[2]]
    }

    if not os.path.exists(json_path):
        data = DEFAULT_JSON.copy()
        data["aabb"] = aabb_data
        with open(json_path, "w") as jf:
            json.dump(data, jf, indent=4)
        return data

    with open(json_path) as jf:
        data = json.load(jf)

    merged = DEFAULT_JSON.copy()
    merged.update(data)

    merged.setdefault("aabb", aabb_data)
    merged["aabb"] = aabb_data
    return merged

def convert_obj(path, out_header, out_c, model_id):
    raw_verts = []
    edges = set()

    with open(path) as f:
        for line in f:
            if line.startswith("v "):
                _, x, y, z = line.split()
                raw_verts.append((float(x), float(y), float(z)))

    if not raw_verts:
        print("No vertices found in:", path)
        return

    cx = sum(v[0] for v in raw_verts) / len(raw_verts)
    cy = sum(v[1] for v in raw_verts) / len(raw_verts)
    cz = sum(v[2] for v in raw_verts) / len(raw_verts)

    raw_verts = [(x - cx, y - cy, z - cz) for (x, y, z) in raw_verts]

    final_scale = SCALE * FX_ONE
    verts = [
        (int(x * final_scale),
         int(y * final_scale),
         int(z * final_scale))
        for (x, y, z) in raw_verts
    ]
    '''
    bounding_radius = max(
        int(math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]))
        for v in verts
    )
    '''

    bounding_radius = max(
        int(math.sqrt(v[0]*v[0] + v[2]*v[2]))
        for v in verts
    )

    with open(path) as f:
        for line in f:
            if line.startswith("f "):
                parts = line.split()[1:]
                idx = [int(p.split("/")[0]) - 1 for p in parts]
                for i in range(len(idx)):
                    a = idx[i]
                    b = idx[(i + 1) % len(idx)]
                    edges.add(tuple(sorted((a, b))))

    name = os.path.basename(path).split('.')[0]
    caps = name.upper()

    json_path = path.replace(".obj", ".json")
    meta = load_or_create_json(json_path, verts)

    with open(out_header, "w") as out:
        out.write(f"#ifndef {caps}_H\n#define {caps}_H\n")
        out.write("#include \"global.h\"\n#include \"entity3d.h\"\n\n")
        out.write(f"extern const EntityTemplate {name}_template;\n\n")
        out.write("#endif\n")

    with open(out_c, "w") as out:
        out.write(f"#include \"{name}.h\"\n\n")

        out.write(f"const int NUM_VERTICES_{name} = {len(verts)};\n")
        out.write(f"const int NUM_EDGES_{name} = {len(edges)};\n\n")

        out.write(f"const s16 model_vertices_{name}[][3] = {{\n")
        for v in verts:
            out.write(f"    {{ {v[0]}, {v[1]}, {v[2]} }},\n")
        out.write("};\n\n")

        out.write(f"const u8 model_edges_{name}[][2] = {{\n")
        for a, b in edges:
            out.write(f"    {{ {a}, {b} }},\n")
        out.write("};\n\n")

        out.write(f"const EntityTemplate {name}_template = {{\n")
        out.write(f"    .id = {model_id},\n")
        out.write(f"    .num_vertices = NUM_VERTICES_{name},\n")
        out.write(f"    .num_edges = NUM_EDGES_{name},\n")
        out.write(f"    .model_vertices = model_vertices_{name},\n")
        out.write(f"    .model_edges = model_edges_{name},\n")
        out.write(f"    .bounding_radius = {bounding_radius},\n")
        out.write(f"    .color = {meta['color']},\n")
        out.write(f"    .dx = {meta['dx']},\n")
        out.write(f"    .dy = {meta['dy']},\n")
        out.write(f"    .dz = {meta['dz']},\n")
        out.write(f"    .hitbox_min_x = {meta['aabb']['min'][0]},\n")
        out.write(f"    .hitbox_min_y = {meta['aabb']['min'][1]},\n")
        out.write(f"    .hitbox_min_z = {meta['aabb']['min'][2]},\n")
        out.write(f"    .hitbox_max_x = {meta['aabb']['max'][0]},\n")
        out.write(f"    .hitbox_max_y = {meta['aabb']['max'][1]},\n")
        out.write(f"    .hitbox_max_z = {meta['aabb']['max'][2]},\n")
        out.write(f"    .health = {meta['health']},\n")
        out.write(f"    .damage = {meta['damage']},\n")
        out.write("};\n")

for filename in os.listdir("models"):
    if filename.lower().endswith(".obj"):
        inpath = os.path.join("models", filename)
        base = filename.replace(".obj", "")
        model_ids[base] = current_id
        current_id += 1
        out_header = os.path.join("models", base + ".h")
        out_c = os.path.join("models", base + ".c")

        print("Converting:", filename)
        convert_obj(inpath, out_header, out_c, model_ids[base])

models_header = "models/models.h"

with open(models_header, "w") as out:
    out.write("#ifndef MODELS_H\n#define MODELS_H\n\n")

    total = 0
    for name, mid in model_ids.items():
        out.write(f"#define MODEL_{name.upper()} {mid}\n")
        total += 1

    out.write(f"#define MODEL_MAX {total}\n")

    out.write("\n")

    for name in model_ids.keys():
        out.write(f"#include \"{name}.h\"\n")
    
    out.write("\n#endif\n")


print("Done.")
