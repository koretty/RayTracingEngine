# 改善提案（現状コード基準）

## 実装済み

- `IBSDF` 導入による散乱責務分離（`sample/eval/pdf`）
- `PbrBsdf` による Lambert + GGX + transmission/ior の統合
- 透明影の Beer-Lambert 減衰
- BVH（SAH 近似ビニング）による交差探索加速
- HDRI 環境マップ（Radiance RGBE RLE）ロードと方向サンプリング
- テクスチャ入力マテリアル（`SolidColor` / `CheckerTexture` / `ImageTexture`）
- CMake で `raytracer_core` と `raytracer` の分離

## 優先度 High

1. 環境光の importance sampling + MIS
- 現状は miss 時サンプリングのみで、高輝度 HDRI の収束が遅くなる場面があります。

2. Russian roulette 終了
- 深い再帰での不要計算を抑え、バイアスを避けつつ速度を改善できます。

3. 自動テスト整備
- 交差判定、BVH hit 一貫性、BSDF の pdf 整合、HDR ローダ（正常系/異常系）を最小セットで追加すると安全です。

4. 直接光サンプリング拡張
- 現状は太陽光を固定方向で評価しているため、将来的な面光源・発光体サンプリングへ拡張余地があります。

## 優先度 Medium

1. トーンマッピング
- 現状はガンマ補正中心のため、高ダイナミックレンジ表現を安定化できます。

2. テクスチャ品質改善
- 現状の `ImageTexture` は単純サンプリング中心のため、将来的にフィルタリングや mipmap 導入で遠景品質を改善できます。

3. 設定外部化
- ヘッダ内インライン設定を JSON/TOML に移すと、再ビルドなしでシーン調整が可能です。

## 設計上の維持ポイント

- `Renderer` の散乱評価は `IBSDF` 抽象依存を維持し、具象 BSDF の選択は注入可能な形を保つ。
- `Scene::sample_environment` を環境光の唯一窓口として維持し、呼び出し側を単純化する。
- `Material` はパラメータとテクスチャサンプリングに限定し、評価ロジックは `bsdf` / `material::optics` に閉じ込める。