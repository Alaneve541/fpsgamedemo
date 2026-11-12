import unreal
unreal.log("[Enum] " + ",".join([a for a in dir(unreal.MaterialSamplerType) if not a.startswith("_")]))
