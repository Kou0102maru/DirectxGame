$files = @(
    'battle.h','billboard.h','bullet.h','bullet_hit_effect.h','camera.h','cirel_shadow.h',
    'collision.h','cube.h','debug_ostream.h','debug_text.h','direct3d.h','easing_cube.h',
    'effect.h','enemy.h','enemy_normal.h','fade.h','game.h','game_window.h','grid.h',
    'key_logger.h','keyboard.h','light.h','light_camera.h','main.h','map.h','mapcamera.h',
    'meshfield.h','model.h','monster.h','monster_dragon.h','monster_slime.h','monster_wolf.h',
    'mouse.h','pad_logger.h','particle.h','particle_test.h','player.h','player_camera.h',
    'sampler.h','scene.h','score.h','shader.h','shader3d.h','shader3d_unlit.h',
    'shader_billboard.h','shader_depth.h','shader_field.h','sky.h','sprite.h','sprite_anim.h',
    'system_timer.h','texture.h','title.h','trajectory.h','trajectory3d.h','Audio.h',
    'main.cpp','scene.cpp','game.cpp','battle.cpp','player.cpp','monster.cpp',
    'monster_slime.cpp','monster_wolf.cpp','monster_dragon.cpp','bullet.cpp',
    'bullet_hit_effect.cpp','camera.cpp','player_camera.cpp','mapcamera.cpp','map.cpp',
    'meshfield.cpp','model.cpp','direct3d.cpp','sprite.cpp','texture.cpp','cube.cpp',
    'billboard.cpp','particle.cpp','particle_test.cpp','effect.cpp','collision.cpp',
    'light.cpp','light_camera.cpp','shader.cpp','shader3d.cpp','shader3d_unlit.cpp',
    'shader_billboard.cpp','shader_depth.cpp','shader_field.cpp','sky.cpp','grid.cpp',
    'fade.cpp','title.cpp','enemy.cpp','enemy_normal.cpp','cirel_shadow.cpp',
    'debug_text.cpp','debug_ostream.cpp','easing_cube.cpp','game_window.cpp',
    'keyboard.cpp','key_logger.cpp','mouse.cpp','pad_logger.cpp','sampler.cpp',
    'score.cpp','sprite_anim.cpp','system_timer.cpp','trajectory.cpp','trajectory3d.cpp',
    'Audio.cpp'
)

foreach ($f in $files) {
    $p = "C:\DirectxGame\$f"
    if (Test-Path $p) {
        [System.IO.File]::WriteAllText($p, [System.IO.File]::ReadAllText($p, [System.Text.Encoding]::UTF8), [System.Text.Encoding]::GetEncoding('shift_jis'))
    }
}
Write-Host "All files converted back to Shift_JIS"
