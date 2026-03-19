# RayTracingEngine

> C++でレイトレーシングを本質から学びたい開発者向けに、OpenMP並列化で実用速度まで引き上げた学習特化レンダリングエンジン。

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)]()

---

## Overview

既存のレイトレーシング学習用コードは「単発で画像を出すだけ」の実装が多く、カメラ制御・マテリアル表現・シーン管理まで含めて段階的に理解しづらいという課題がありました。そこで本プロジェクトでは、数学基盤からレンダラーまでをC++23で分離実装し、学習と拡張の両立を目指しています。

このプロジェクトは、**「レイトレーシング実装の複雑化という課題」を「責務分離したモジュール設計と実行可能なサンプルシーン」で解決する**ことを目的としています。

競合的なサンプル実装との差別化ポイントは、OpenMPによる並列化を含む「動く最適化例」まで同一コードベースで確認できる点です。学習用途で終わらず、性能検証の入口としても使えます。

### Features

* OpenMP `schedule(dynamic)` による並列レンダリングで、測定環境にて約5.1倍の高速化を確認
* SDL3ウィンドウ上で即時プレビューしながら、WASD + U/Yのカメラ移動と再レンダリングを実行
* `math` / `scene` / `object` / `material` / `renderer` の分離により、機能追加や差し替えがしやすい構成

---

## Demo

![Demo](img/test_output1.png)

アプリ起動後にシーンを描画し、視点を移動して、必要なタイミングで `P` キーでPPM画像を書き出します。

```text
[Insert Demo Image or GIF]
```

---

## Quick Start

最短で動作確認するため、単一コマンドでビルドして実行できます。

### Requirements

* 言語 / ランタイム: C++23
* 必要ツール: GCC (MinGW) 15.x 以上、SDL3開発ライブラリ、OpenMP対応コンパイラ
* 推奨環境: Windows 11 (x64)

### Installation

```bash
git clone https://github.com/yourname/raytracing-engine.git
cd raytracing-engine

# 依存関係
# SDL3 を事前にインストールして、コンパイラ/リンカから参照可能な状態にしてください
```

### Run

```bash
g++ -std=c++23 -O2 -fopenmp \
  src/main/main.cpp \
  src/renderer/renderer.cpp \
  src/scene/scene.cpp \
  src/object/sphere.cpp \
  -o raytracing-engine -lSDL3

./raytracing-engine
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
| Backend        | C++23                   | 高速な数値計算と明確な型設計でレイトレーシング実装に適している |
| Database       | -                       | オフライン描画ツールのため永続DBは不要 |
| Infrastructure | OpenMP (CPU並列処理)    | ピクセル計算を並列化してレンダリング時間を短縮できる |

---

## Project Structure

主要ディレクトリは、責務ごとに分離した構成です。

```text
.
├── img/                     # 出力例画像
├── src/
│   ├── main/                # エントリーポイントと設定
│   │   ├── main.cpp
│   │   └── config/
│   │       ├── camera_config.hpp
│   │       └── scene_config.hpp
│   ├── math/                # ベクトル・Ray・乱数などの数学基盤
│   ├── material/            # マテリアル定義と散乱計算
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
