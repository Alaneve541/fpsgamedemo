import sys

sys.path.insert(0, r'C:\Users\alane\Desktop\111\fps_game 5.7\Content\Python')
import build_levels as bl  # noqa: E402

unreal = bl.unreal
unreal.log('[Campaign] rebuild start')
try:
    ok = bl.build_campaign()
    unreal.log('[Campaign] ok=%s' % ok)
except Exception as exc:
    unreal.log_error('[Campaign] failed: %s' % exc)
unreal.log('[Campaign] done')
