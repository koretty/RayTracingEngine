# Data Flow

データの流れ（入力 → レンダリングループ → 出力）を示します。

- 入力: SDL イベント（W/S/A/D/U/Y/ESC/P）
- 前処理: camera_config の状態更新と needs_render フラグ判定
- 実行主体: main ループが Renderer.render(scene, camera) を呼ぶ
- サンプリング: 1ピクセルごとにジッター付き複数レイを生成
- 交差判定: Scene.find_closest_hit() が最短ヒットを返す
- 散乱計算: Material.scatter() が次レイと減衰を決定
- 出力: SDL テクスチャ表示、任意で PPM 保存

```mermaid
graph TD
    A[SDL Event Loop in main.cpp] -->|key input| B[camera_config state update]
    B -->|moved?| C[needs_render flag]

    D[scene_config.create_scene] -->|build once| E[Scene]
    C -->|true| F[make_camera(width,height)]
    F --> G[Renderer.render(Scene, Camera)]

    G --> H[OpenMP parallel pixel loop]
    H --> I[Multi-sample camera.get_ray(u,v)]
    I --> J[trace_ray(ray, scene, depth)]
    J --> K[Scene.find_closest_hit]
    K -->|hit| L[Material.scatter + direct sun]
    K -->|miss| M[Scene.background]
    L --> J
    M --> N[accumulate and gamma-correct]
    J --> N

    N --> O[pixels buffer (ARGB8888)]
    O --> P[SDL_UpdateTexture]
    P --> Q[SDL_RenderPresent]

    A -->|P key| R[save_image -> render_output.ppm]
    O --> R
```

実運用の主経路は main ループ → Renderer → Scene/Material です。PPM保存は補助的な出力経路です。