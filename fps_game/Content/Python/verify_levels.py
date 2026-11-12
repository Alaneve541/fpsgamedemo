import unreal

def log(m):
    unreal.log('[Verify] ' + str(m))

# Campaign
db = unreal.load_asset('/Game/Story/DA_Campaign')
log('db=%s' % db)
chapters = db.get_editor_property('chapters')
log('chapters=%d' % len(chapters))
for ch in chapters:
    beats = ch.get_editor_property('beats')
    voiced = [b.get_editor_property('beat_id') for b in beats
              if b.get_editor_property('voice') is not None]
    log('  %s | %s | beats=%d | obj=%s' % (ch.get_editor_property('chapter_id'),
                                           ch.get_editor_property('title'),
                                           len(beats), ch.get_editor_property('objective')))
    log('     voices=%s' % voiced)

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
for name in ['Ch01', 'Ch02', 'Ch03', 'Ch04', 'Ch05']:
    world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/' + name)
    if not world:
        log('%s LOAD FAILED' % name)
        continue
    actors = eas.get_all_level_actors()
    classes = {}
    for a in actors:
        cls = a.get_class().get_name()
        classes[cls] = classes.get(cls, 0) + 1
    log('%s actors=%d %s' % (name, len(actors), sorted(classes.items())))
    for a in actors:
        if a.get_class().get_name() == 'ZombieWaveSpawner':
            log('   spawner zombie=%s waves=%s base=%s' % (
                a.get_editor_property('zombie_class'),
                a.get_editor_property('total_waves'),
                a.get_editor_property('base_count')))
        if a.get_class().get_name() == 'StoryNote':
            log('   note title=%s mesh=%s' % (a.get_editor_property('note_title'),
                                              a.get_editor_property('visual_mesh')))
        if a.get_class().get_name() == 'LevelAmbience':
            log('   ambience sound=%s vol=%s pitch=%s' % (
                a.get_editor_property('ambient_sound'),
                a.get_editor_property('volume'),
                a.get_editor_property('pitch')))
log('done')
