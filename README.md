# RayTracingEngine

> C++でレイトレーシングを本質から学びたい開発者向けに、OpenMP並列化で実用速度まで引き上げた学習特化レンダリングエンジン。

---

## Overview

3Dレンダリング技術に興味を持ったことをきっかけに、レイトレーシングの仕組みを理解するための実装を行いました。既存の学習コードは単発で画像を出すものが多く、全体の流れがつかみにくいと感じたため、数学的な基礎からレンダラーまでを段階的に整理しながら C++20 で実装しています。

競合的なサンプル実装との差別化ポイントは、OpenMPによる並列化やSIMD最適化を前提とした設計にあります。特に、データ構造を `Array of Structures of Arrays` とすることで、ベクトル化と並列化の両方を効率よく適用できるようにしています。これにより、単なる学習用途にとどまらず、実際に動作する最適化手法を同一コードベースで検証できる構成としています。

### Features

* OpenMP `schedule(dynamic)` による並列レンダリングで、測定環境にて約5.1倍の高速化を確認
* SDL3ウィンドウ上で即時プレビューしながら、WASD + U/Yのカメラ移動と再レンダリングを実行
* `sample / eval / pdf` を分離した PBR ベース BSDF（Lambert + GGX + transmission/ior）
* ガラス影に Beer-Lambert（厚み依存吸収）を導入し、浮いて見える透過物の不自然さを低減
* `math` / `scene` / `object` / `material` / `bsdf` / `renderer` の分離により、機能追加や差し替えがしやすい構成

---

## Demo

![Demo](img/test_output1.png)



---

## Quick Start

最短で動作確認するため、単一コマンドでビルドして実行できます。

### Requirements

* 言語 / ランタイム: C++20
* 必要ツール: GCC (MinGW) 15.x 以上、SDL3開発ライブラリ、OpenMP対応コンパイラ
* 推奨環境: Windows 11 (x64)

### Installation

```bash
git clone https://github.com/koretty/raytracing-engine
cd raytracing-engine

```

### Run

```bash
g++ -std=c++20 -O2 -fopenmp \
  src/main/main.cpp \
  src/renderer/renderer.cpp \
  src/scene/scene.cpp \
  src/object/sphere.cpp \
  src/bsdf/pbr_bsdf.cpp \
  -o raytracer.exe -lSDL3

./raytracer.exe
```

---

## Usage

起動後はキーボード入力でカメラを移動し、視点変更のたびに再レンダリングが走ります。

### Example

```bash
# 操作キー
# W/S/A/D : 前後左右移動
# U/Y     : 上下移動
# P       : 現在のフレームを render_output.ppm として保存
# ESC     : 終了
```

### Configuration

```json
{
  "camera": {
    "fov": 35.0,
    "aperture": 0.1,
    "move_speed": 0.5
  },
  "glass_material": {
    "transmission": 1.0,
    "ior": 1.5,
    "absorption_coefficient": [0.35, 0.10, 0.06]
  },
  "render": {
    "width": 800,
    "height": 600,
    "samples_per_pixel": 100,
    "max_depth": 10
  }
}
```

---

## Tech Stack

主要技術は「低レイヤの制御性」と「学習コストの低さ」を重視して選定しています。

| Category       | Technology              | Reason |
| :------------- | :---------------------- | :----- |
| Frontend       | SDL3 Window             | 軽量な描画ウィンドウを構築し、レンダリング結果を即時表示できる |
| Backend        | C++20                   | 高速な数値計算と明確な型設計でレイトレーシング実装に適している |
| Database       | -                       | オフライン描画ツールのため永続DBは不要 |
| Infrastructure | OpenMP (CPU並列処理)    | ピクセル計算を並列化してレンダリング時間を短縮できる |

---

## Project Structure

主要ディレクトリは、責務ごとに分離した構成です。

```text
.
├── img/                     # 出力例画像
├── src/
│   ├── main/                
│   │   ├── main.cpp
│   │   └── config/
│   │       ├── camera_config.hpp
│   │       └── scene_config.hpp
│   ├── math/                # ベクトル・Ray・乱数などの数学基盤
│   ├── material/            # マテリアルのパラメータと光学補助（Beer-Lambert）
│   ├── bsdf/                # BSDF抽象とPBR実装（Lambert + GGX + transmission）
│   ├── object/              # オブジェクト抽象と球体実装
│   ├── scene/               # シーンとカメラ
│   └── renderer/            # レンダリング本体
└── README.md
```

---

## Roadmap

* [x] 球体・マテリアル・背景光を含む基本レイトレーサー
* [x] OpenMPによるCPU並列化
* [ ] BVHなど空間分割による交差判定の高速化
* [ ] 外部設定ファイル（JSON/TOML）対応
* [ ] 自動テスト（数値検証・回帰テスト）の追加

---

## License

MIT License
