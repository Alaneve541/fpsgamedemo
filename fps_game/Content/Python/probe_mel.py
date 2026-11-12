import unreal
names = [n for n in dir(unreal.MaterialEditingLibrary) if 'ompil' in n.lower() or 'rror' in n.lower()]
unreal.log("[MEL] " + ",".join(names))
names2 = [n for n in dir(unreal.Material) if 'ompil' in n.lower() or 'rror' in n.lower()]
unreal.log("[MAT] " + ",".join(names2))
