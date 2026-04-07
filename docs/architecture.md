# Architecture Overview

このプロジェクトは、CPU 上で動くパストレーサーを小さなモジュールへ分割した構成です。現在は BSDF 分離と HDRI 環境光を含むレンダリング経路まで実装されています。

## 設計原則

- 依存逆転: `Renderer` は `IBSDF` 抽象を通して散乱評価を呼び出す。
- 責務分離: `Material` はデータ保持のみ、散乱ロジックは `bsdf` 側に集約。
- 設定集中: カメラとシーン設定は `src/main/config` に集約し、`main` から生成。
- フォールバック設計: 環境マップ無効時は背景色を返して処理を継続。

## モジュール責務

- `src/main/main.cpp`
    - SDL イベントループ、再レンダリング制御、PPM 保存。
- `src/main/config/camera_config.hpp`
    - カメラの位置・FOV・移動速度設定。
- `src/main/config/scene_config.hpp`
    - マテリアル、オブジェクト、太陽光、HDRI 設定とシーン構築。
- `src/renderer/renderer.{hpp,cpp}`
    - パストレース本体、直達太陽光、透過影評価、再帰積分。
- `src/bsdf/bsdf.hpp`
    - `IBSDF` 契約（`sample/eval/pdf`）と `BsdfSample`。
- `src/bsdf/pbr_bsdf.{hpp,cpp}`
    - Lambert + GGX + transmission/ior の統合モデル。
- `src/environment/environment_map.{hpp,cpp}`
    - Radiance HDR（RGBE RLE）ロード、lat-long 方向サンプリング、双線形補間。
- `src/scene/scene.{hpp,cpp}`
    - オブジェクト・マテリアル所有、交差探索、環境サンプル窓口。
- `src/object/*`, `src/math/*`
    - 幾何交差と数学ユーティリティ。

## レンダリングパイプライン

1. `main` が設定から `Scene` と `Camera` を生成。
2. `Renderer::render` が OpenMP で画素ループを実行。
3. 各サンプルで `trace_ray` を呼び、hit/miss を判定。
4. hit 時は `bsdf.eval` で直達太陽光、`bsdf.sample` で間接光を評価。
5. 透明影は交差の入出点厚みを使って Beer-Lambert 減衰を適用。
6. miss 時は `Scene::sample_environment` で HDRI または背景色を返す。

## HDRI 実装範囲

- 入力形式: Radiance `.hdr`（`FORMAT=32-bit_rle_rgbe`）
- 読み込み: scanline RLE デコード + RGBE から浮動小数色へ復元
- サンプリング: 方向ベクトルを緯度経度 UV に変換し双線形補間
- 有効化: `config::environment::enabled` を `true` に設定

## ビルド構成

- `raytracer_core`: `src/main/main.cpp` 以外のコア実装をまとめた静的ライブラリ
- `raytracer`: SDL エントリーポイントを持つ実行バイナリ
- OpenMP は検出時にリンク、未検出時はシングルスレッドで動作
