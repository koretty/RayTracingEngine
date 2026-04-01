# モジュール依存関係

BSDF 導入後の依存関係（依存逆転を反映）です。

```mermaid
graph TD
    main_cpp["src/main/main.cpp"] --> renderer_hpp["src/renderer/renderer.hpp"]
    main_cpp --> camera_cfg["src/main/config/camera_config.hpp"]
    main_cpp --> scene_cfg["src/main/config/scene_config.hpp"]

    renderer_cpp["src/renderer/renderer.cpp"] --> bsdf_if["src/bsdf/bsdf.hpp"]
    renderer_cpp --> bsdf_impl["src/bsdf/pbr_bsdf.hpp"]
    renderer_cpp --> scene_mod["src/scene/scene.hpp"]
    renderer_cpp --> optics_mod["src/material/optics.hpp"]

    bsdf_impl_cpp["src/bsdf/pbr_bsdf.cpp"] --> bsdf_if
    bsdf_impl_cpp --> material_mod["src/material/material.hpp"]
    bsdf_impl_cpp --> math_utils["src/math/math_utils.hpp"]
    bsdf_impl_cpp --> object_abs["src/object/object.hpp"]

    scene_mod --> material_mod
    scene_mod --> object_abs

    object_sphere["src/object/sphere.cpp"] --> object_abs
    object_sphere --> ray_mod["src/math/ray.hpp"]
    ray_mod --> vec3_mod["src/math/vec3.hpp"]

    scene_cfg --> scene_mod
    scene_cfg --> material_mod
    scene_cfg --> object_sphere

    %% External
    sdl3["SDL3"]
    openmp["OpenMP"]
    cmake_core["raytracer_core (static lib)"]

    main_cpp --> sdl3
    renderer_cpp --> openmp
    cmake_core --> renderer_cpp
    cmake_core --> bsdf_impl_cpp
```

- 重要: `Renderer` は抽象 `IBSDF` に依存し、`Material` 実装詳細や散乱分岐ロジックを持たない。
- `PbrBsdf` は `Material` パラメータを解釈するが、シーン管理や描画ループを知らない。
- Beer-Lambert は `material/optics.hpp` に分離し、`Renderer` は光学関数を呼ぶだけに限定している。
- `raytracer_core` ライブラリ化により、将来のテスト実行バイナリで同じコアを再利用できる。
