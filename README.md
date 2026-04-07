# RayTracingEngine

CPU ベースの学習向けパストレーサーです。C++20 で、PBR BSDF・透過影の Beer-Lambert・任意 HDRI 環境光を段階的に検証できる構成になっています。

## Overview

このプロジェクトは、数学基盤からレンダラーまでを小さなモジュールに分けて実装しています。`Renderer` は `IBSDF` 抽象に依存し、`Material` はパラメータ保持に限定されるため、BSDF 実装の差し替えと拡張がしやすい設計です。

## Implemented Features

- OpenMP によるピクセル並列レンダリング（`#pragma omp parallel for schedule(dynamic)`）
- SDL3 ウィンドウへの即時表示と、カメラ移動時の再レンダリング
- `sample / eval / pdf` 契約を持つ PBR BSDF（Lambert + GGX + transmission + ior）
- Beer-Lambert による透過影の厚み依存吸収
- 環境マップによる HDRI サンプリング（Radiance RGBE RLE 形式）
- レイ miss 時に環境マップまたは背景色を返す環境光経路

## Demo

![Demo](img/test_output1.png)

## Build And Run

### Requirements

- C++20 対応コンパイラ
- g++（MinGW/GCC）
- SDL3 開発ライブラリ
- OpenMP（任意。未検出時はシングルスレッド）

### Build

```bash
git clone https://github.com/koretty/raytracing-engine
cd raytracing-engine

g++ -std=c++20 -O2 -fopenmp \
	src/main/main.cpp \
	src/renderer/renderer.cpp \
	src/scene/scene.cpp \
	src/object/sphere.cpp \
	src/bsdf/pbr_bsdf.cpp \
	src/environment/environment_map.cpp \
	-o raytracer -lSDL3
```

### Run

Windows:

```powershell
.\raytracer.exe
```

Linux/macOS:

```bash
./raytracer
```


## Controls

- W/S/A/D: 前後左右移動
- U/Y: 上下移動
- P: 現在フレームを `render_output.ppm` として保存
- ESC: 終了

## Configuration

設定はヘッダ内のインライン変数で管理しています。

- カメラ: `src/main/config/camera_config.hpp`
- シーン・マテリアル・太陽光・HDRI: `src/main/config/scene_config.hpp`

HDRI を有効化する場合は `scene_config.hpp` の `config::environment` を変更します。

```cpp
namespace config {
namespace environment {

inline bool enabled = true;
inline const char* hdr_path = "img/environment.hdr";
inline float intensity = 1.0f;

} // namespace environment
} // namespace config
```

補足:

- 読み込み失敗時はログに `Failed to load HDRI` を出力し、背景色フォールバックになります。
- Beer-Lambert を無効化する場合は、ビルド時に `-DRAYTRACER_ENABLE_BEER_LAMBERT=0` を追加してください。

## Project Structure

```text
.
├── docs/
├── img/
├── src/
│   ├── bsdf/                # IBSDF 抽象と PbrBsdf 実装
│   ├── environment/         # HDRI ローダと方向サンプリング
│   ├── main/
│   │   ├── config/          # camera / scene 設定
│   │   └── main.cpp
│   ├── material/            # Material データ + 光学補助
│   ├── math/                # Vec3 / Ray / random
│   ├── object/              # Object 抽象 + Sphere
│   ├── renderer/            # パストレース本体
│   └── scene/               # Scene / Camera
├── CMakeLists.txt
└── README.md
```

## Roadmap

- [x] PBR BSDF の `sample/eval/pdf` 分離
- [x] Beer-Lambert 透過影
- [x] HDRI 環境マップ対応
- [ ] BVH 導入による交差判定高速化
- [ ] 環境光の importance sampling と MIS
- [ ] 自動テスト整備（BSDF/交差/HDR ローダ）

## License

MIT License
