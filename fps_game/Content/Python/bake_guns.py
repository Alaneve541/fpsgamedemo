import os

SRC = r"C:/Users/alane/Desktop/111/fps_game 5.7/Content/Models/Guns"
OUT = os.path.join(SRC, "baked")

# source obj -> (target length in cm, output name)
JOBS = [
    ("m16.obj", "gun_virtus.obj", 95.0),
    ("stg44.obj", "gun_k9.obj", 82.0),
    ("12_gauge.obj", "gun_m90.obj", 104.0),
    ("sniper.obj", "gun_dmr.obj", 122.0),
]

os.makedirs(OUT, exist_ok=True)

for src, dst, target_len in JOBS:
    path = os.path.join(SRC, src)
    verts = []
    lines = []
    with open(path, "r", errors="ignore") as f:
        lines = f.readlines()
    for line in lines:
        if line.startswith("v "):
            p = line.split()
            verts.append((float(p[1]), float(p[2]), float(p[3])))

    xs = [v[0] for v in verts]
    ys = [v[1] for v in verts]
    zs = [v[2] for v in verts]
    lo = (min(xs), min(ys), min(zs))
    hi = (max(xs), max(ys), max(zs))
    # obj long axis is Z (all four guns). muzzle at min Z (small cross-section end).
    # target ue axes: +X = (maxZ - z) (muzzle forward), +Y = x (width), +Z = y (up)
    k = target_len / (hi[2] - lo[2])
    cy = (lo[1] + hi[1]) * 0.5
    cx = (lo[0] + hi[0]) * 0.5

    out = []
    for line in lines:
        if line.startswith("v "):
            p = line.split()
            x, y, z = float(p[1]), float(p[2]), float(p[3])
            nx = (hi[2] - z) * k
            ny = (x - cx) * k
            nz = (y - cy) * k
            out.append("v %.4f %.4f %.4f\n" % (nx, ny, nz))
        elif line.startswith("f "):
            # mirror flips winding; reverse face order to keep normals outward
            p = line.split()
            if len(p) >= 5:
                p = [p[0]] + [p[1]] + list(reversed(p[2:]))
            out.append(" ".join(p) + "\n")
        else:
            out.append(line)

    with open(os.path.join(OUT, dst), "w") as f:
        f.writelines(out)
    print("baked", dst, "len", round(target_len, 1), "verts", len(verts),
          "bbox_cm", (round((hi[0]-lo[0])*k, 1), round((hi[1]-lo[1])*k, 1), round(target_len, 1)))
