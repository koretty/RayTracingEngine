# 改善提案（BSDF リファクタ後）

## 実施済み

- `Material` を純粋データ化し、散乱ロジックを削除。
- `IBSDF`（`sample/eval/pdf`）を導入して責務分離を明確化。
- `PbrBsdf` で Lambert + GGX + transmission/ior を単一モデルとして統合。
- `Renderer` を BSDF 抽象依存へ変更し、実装差し替え可能にした。
- CMake を `raytracer_core` ライブラリ構成へ変更し、テスト導入しやすくした。
- 透明影に Beer-Lambert（厚み依存吸収）を導入し、ガラスの接地感を改善。

## 次の拡張候補

1. 新しい BRDF/BSDF の追加
  - `IBSDF` 実装として `OrenNayarBsdf` や `DisneyBsdf` を追加。

2. 複数ローブ合成
  - `CompositeBsdf` を導入し、クリアコートやシーン別ローブ合成を実現。

3. テクスチャマッピング
  - `Material` の各パラメータを定数からテクスチャサンプルへ拡張。

4. テスト自動化
  - BSDF 単体テスト（エネルギー保存、半球対称性、pdf 正規性）を `tests/` 配下へ追加。

5. 光源サンプリング拡張
  - `pdf` を活用した MIS 対応でノイズ低減を図る。

6. 体積吸収の一般化
  - 目前は影透過での厚み吸収を実装済み。将来は経路全体へ同じ吸収モデルを拡張する。

## 設計指針（維持するべき点）

- `Renderer` は `Material` 内部や具体 BSDF 実装に直接依存しない。
- `Material` はデータのみ、BSDF は評価ロジックのみという境界を崩さない。
- 新機能は `bsdf` モジュールへ閉じ込め、`scene/object/math` への不要依存を避ける。