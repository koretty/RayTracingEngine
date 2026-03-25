# モジュール依存関係

プロジェクト内部のモジュール依存を示します。

```mermaid
graph TD
    main_cpp["src/main/main.cpp"] --> renderer["src/renderer/renderer"]
    main_cpp --> camera_cfg["src/main/config/camera_config.hpp"]
    main_cpp --> scene_cfg["src/main/config/scene_config.hpp"]

    renderer --> scene_mod["src/scene/scene"]
    renderer --> camera_mod["src/scene/camera.hpp"]
    renderer --> math_utils["src/math/math_utils.hpp"]

    scene_mod --> object_abs["src/object/object.hpp"]
    scene_mod --> material_mod["src/material/material.hpp"]
    scene_mod --> ray_mod["src/math/ray.hpp"]

    object_sphere["src/object/sphere"] --> object_abs
    object_sphere --> ray_mod
    object_sphere --> vec3_mod["src/math/vec3.hpp"]

    material_mod --> object_abs
    material_mod --> ray_mod
    material_mod --> math_utils

    camera_mod --> ray_mod
    camera_mod --> math_utils
    ray_mod --> vec3_mod

    camera_cfg --> camera_mod
    scene_cfg --> scene_mod
    scene_cfg --> material_mod
    scene_cfg --> object_sphere

    %% 補足: 外部ライブラリ
    sdl3["SDL3"]
    openmp["OpenMP"]
    cpp_std["C++23 STL"]

    main_cpp --> sdl3
    renderer --> openmp
    main_cpp --> cpp_std
    renderer --> cpp_std
    scene_mod --> cpp_std
```

- 循環依存はない。主経路は main.cpp → renderer → scene/object/material/math。
- 設定ヘッダは main から参照される境界層で、レンダラー本体への依存は持たない。
- OpenMP 依存は renderer.cpp に閉じ込められており、他モジュールは並列化手法から独立している。
