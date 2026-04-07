# 改善提案（現状コード基準）

## 実装済み

- `IBSDF` 導入による散乱責務分離（`sample/eval/pdf`）
- `PbrBsdf` による Lambert + GGX + transmission/ior の統合
- 透明影の Beer-Lambert 減衰
- HDRI 環境マップ（Radiance RGBE RLE）ロードと方向サンプリング
- CMake で `raytracer_core` と `raytracer` の分離

## 優先度 High

1. BVH 導入
- 現状は全オブジェクト線形探索のため、オブジェクト数増加時に計算量が増大します。

2. 環境光の importance sampling
- 現状は miss 時サンプリングのみで、高輝度 HDRI の収束が遅くなる場面があります。

3. Russian roulette 終了
- 深い再帰での不要計算を抑え、バイアスを避けつつ速度を改善できます。

4. 自動テスト整備
- 交差判定、BSDF の pdf 整合、HDR ローダ（正常系/異常系）を最小セットで追加すると安全です。

## 優先度 Medium

1. トーンマッピング
- 現状はガンマ補正中心のため、高ダイナミックレンジ表現を安定化できます。

2. マテリアル入力拡張
- `Material` を定数値からテクスチャ入力へ拡張し、見た目の表現力を上げられます。

3. 設定外部化
- ヘッダ内インライン設定を JSON/TOML に移すと、再ビルドなしでシーン調整が可能です。

## 設計上の維持ポイント

- `Renderer` は `IBSDF` 抽象依存を維持し、具体 BSDF 実装へ直接結合しない。
- `Scene::sample_environment` を環境光の唯一窓口として維持し、呼び出し側を単純化する。
- `Material` はデータ保持に限定し、評価ロジックは `bsdf` / `material::optics` に閉じ込める。