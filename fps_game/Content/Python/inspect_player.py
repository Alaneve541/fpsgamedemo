import unreal

bp = unreal.load_class(None, "/Game/Code/Player/BP_FPSPlayer.BP_FPSPlayer_C")
cdo = unreal.get_default_object(bp)
print("[BP] class", bp.get_name())
for comp in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
    print("[Skel] name=", comp.get_name())
    try:
        mesh = comp.get_editor_property("skeletal_mesh_asset")
        print("   mesh=", mesh.get_path_name() if mesh else None)
        if mesh:
            sk = mesh.get_editor_property("skeleton")
            if sk:
                print("   skeleton=", sk.get_path_name())
                for sock in sk.get_editor_property("sockets"):
                    print("   socket:", sock.get_editor_property("socket_name"),
                          sock.get_editor_property("bone_name"),
                          sock.get_editor_property("relative_location"))
    except Exception as e:
        print("   ERR", e)
for comp in cdo.get_components_by_class(unreal.StaticMeshComponent):
    print("[Static] name=", comp.get_name(), comp.get_editor_property("static_mesh_asset"))
for comp in cdo.get_components_by_class(unreal.CameraComponent):
    print("[Cam] name=", comp.get_name(), comp.get_relative_location())
