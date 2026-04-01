# Data Flow

BSDF リファクタ後の主経路（入力 -> サンプリング -> 積分 -> 出力）を示します。

- 入力: SDL イベントでカメラ状態を更新
- レンダリング開始: `Renderer.render(scene, camera)`
- レイ追跡: `trace_ray` が `Scene.find_closest_hit` を呼ぶ
- BRDF/BSDF 評価:
    - 直達光: `bsdf.eval(wo, wi, hit, material)`
    - 影透過: 透明物の入出点厚みを計測し Beer-Lambert で減衰
    - 間接光: `bsdf.sample(wo, hit, material)` で次方向を生成
    - 密度: `bsdf.pdf(...)` は sample 内と将来 MIS 拡張で利用
- 出力: ガンマ補正後に SDL テクスチャへ転送

```mermaid
graph TD
        A[SDL Event Loop] --> B[camera_config update]
        B --> C[needs_render true]

        D[scene_config create_scene] --> E[Scene]
        C --> F[make_camera]
        F --> G[Renderer.render]

        G --> H[OpenMP pixel loop]
        H --> I[camera.get_ray]
        I --> J[trace_ray(ray, scene, depth)]

        J --> K[Scene.find_closest_hit]
        K -->|miss| L[Scene.background]
        K -->|hit| M[Fetch Material by id]

        M --> N[Direct sun via bsdf.eval]
        M --> V[Shadow transmittance via Beer-Lambert]
        M --> O[Indirect bounce via bsdf.sample]
        O --> P[Spawn next ray]
        P --> J

        N --> Q[Combine emission + direct + indirect]
        V --> Q
        L --> Q
        J --> Q

        Q --> R[Gamma correction]
        R --> S[pixels buffer]
        S --> T[SDL_UpdateTexture / Present]
```

旧設計と異なり、`Material.scatter` は存在せず、サンプリング責務は `IBSDF` に統一されています。