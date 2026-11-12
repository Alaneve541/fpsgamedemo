import unreal
import math

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

path = "/Game/Levels/Materials/M_SimpleTex"
if EAL.does_asset_exist(path):
    EAL.delete_asset(path)
mat = AT.create_asset("M_SimpleTex", "/Game/Levels/Materials", unreal.Material, unreal.MaterialFactoryNew())
tex = unreal.load_asset("/Game/Levels/Textures/T_asphalt_02_D")
s = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
s.set_editor_property("texture", tex)
uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -700, 120)
uv.set_editor_property("u_tiling", 8.0)
uv.set_editor_property("v_tiling", 8.0)
MEL.connect_material_expressions(uv, "", s, "UVs")
ok = MEL.connect_material_property(s, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
print("[Simple] base connected", ok)
MEL.recompile_material(mat)
EAL.save_asset(path)

CUBE = "/Engine/BasicShapes/Cube.Cube"
def box(label, loc, size, mat_path):
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*loc), unreal.Rotator(0, 0, 0))
    a.set_actor_label(label)
    c = a.static_mesh_component
    c.set_static_mesh(unreal.load_asset(CUBE))
    a.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    c.set_material(0, unreal.load_asset(mat_path))
    return a

def light(cls, loc, rot, intensity, extra=None, comp_cls=unreal.LightComponent):
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(*rot))
    comp = a.get_component_by_class(comp_cls)
    comp.set_intensity(intensity)
    if extra:
        extra(comp)
    return a

box("Floor", (0, 0, 0), (8000, 8000, 20), path)
light(unreal.DirectionalLight, (0, 0, 900), (-50, -30, 0), 5.0)
light(unreal.SkyLight, (0, 0, 600), (0, 0, 0), 1.0,
      lambda c: c.set_editor_property("lower_hemisphere_is_black", False),
      unreal.SkyLightComponent)

world = unreal.EditorLevelLibrary.get_editor_world()
rt = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8)
eye, target = (0, -600, 500), (0, 200, 0)
ca = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(*eye), unreal.Rotator(0, 0, 0))
cap = ca.capture_component2d
cap.set_editor_property("texture_target", rt)
cap.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
cap.set_editor_property("capture_every_frame", False)
cap.set_editor_property("fov_angle", 90.0)
dx, dy, dz = target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]
yaw = math.degrees(math.atan2(dy, dx))
pitch = math.degrees(math.atan2(dz, math.hypot(dx, dy)))
ca.set_actor_rotation(unreal.Rotator(pitch, yaw, 0.0), False)
cap.capture_scene()
unreal.RenderingLibrary.export_render_target(world, rt, "C:/Users/alane/Desktop/111/fps_game 5.7/Saved", "PreviewSimple.png")
print("[Render] done")
