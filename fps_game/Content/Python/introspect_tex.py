import unreal
f = unreal.TextureFactory()
print("[TF]", [p for p in dir(f) if 'srgb' in p.lower() or 'compression' in p.lower() or 'lod' in p.lower() or 'group' in p.lower()])
tex = unreal.load_asset("/Game/Levels/Textures/T_asphalt_02_N")
print("[TEX]", [m for m in dir(tex) if any(k in m.lower() for k in ('update','resource','change','build','compress','save','filter','lod'))])
