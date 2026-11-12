import unreal

BPS = [
    '/Game/Code/Zombie/BP_Zombie',
    '/Game/Code/Weapon/Base/BP_WeaponBase',
    '/Game/Code/Weapon/BP_VIRTUS',
    '/Game/Code/Player/BP_FPSPlayer',
    '/Game/Code/Player/System/BP_FPSGameMode',
]


def prim(v):
    return isinstance(v, (int, float, bool, str))


for path in BPS:
    print('\n===== %s' % path)
    try:
        cls = unreal.load_class(None, path + '_C')
    except Exception as exc:
        cls = None
        print('load_class failed:', exc)
    if not cls:
        try:
            bp = unreal.load_asset(path)
            cls = bp.generated_class()
        except Exception as exc:
            print('load failed:', exc)
            continue
    print('class:', cls.get_name() if hasattr(cls, 'get_name') else cls)
    cdo = None
    try:
        cdo = unreal.get_default_object(cls)
    except Exception as exc:
        print('cdo failed:', exc)
    if cdo:
        names = [n for n in dir(cdo) if not n.startswith('_')]
        for name in names:
            if name.startswith(('get_', 'set_', 'k2_', 'call_method', 'cast', 'cast_to')):
                continue
            try:
                val = cdo.get_editor_property(name)
            except Exception:
                continue
            if prim(val):
                print('  %-32s = %r' % (name, val))
            elif isinstance(val, unreal.Object):
                print('  %-32s -> %s' % (name, val.get_name()))
            elif isinstance(val, (list, tuple)) and len(val) < 8:
                try:
                    print('  %-32s = %r' % (name, val))
                except Exception:
                    pass
        # component defaults
        for comp_name in ('CharacterMovement', 'CharacterMovementComponent'):
            try:
                comp = cdo.get_editor_property(comp_name)
            except Exception:
                continue
            if comp:
                print('  [%s]' % comp_name)
                for pname in ('max_walk_speed', 'jump_z_velocity', 'gravity_scale', 'braking_deceleration', 'ground_friction'):
                    try:
                        print('    %s = %r' % (pname, comp.get_editor_property(pname)))
                    except Exception:
                        pass
print('DONE')
