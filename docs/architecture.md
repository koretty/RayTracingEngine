# Architecture Overview

このプロジェクトは、マテリアル散乱を `scatter` 関数から分離し、`BSDF` インターフェースを中心に統一 PBR モデルへ再構成した CPU レイトレーサです。

## 設計目標

- 疎結合: `Renderer` は `IBSDF` 抽象のみを知り、具体実装 (`PbrBsdf`) には依存しない。
- 高凝集: `Material` はパラメータ保持に限定し、散乱ロジックは `bsdf` モジュールに集約。
- 拡張性: `sample / eval / pdf` 契約を守る BSDF 実装を追加するだけで新 BRDF/BSDF を導入可能。
- テスト容易性: BSDF 計算を独立モジュール化し、レンダラー外での数値検証を可能にする。

## モジュール責務

- `src/material/material.hpp`
    - データクラス（`base_color`, `metallic`, `roughness`, `transmission`, `ior`, `absorption_coefficient`, `emission`）のみ。
- `src/material/optics.hpp`
    - Beer-Lambert の体積透過 $T(d)=\exp(-\sigma_a d)$ を提供する光学補助関数。
- `src/bsdf/bsdf.hpp`
    - 抽象インターフェース `IBSDF` とサンプリング結果 `BsdfSample` を定義。
- `src/bsdf/pbr_bsdf.{hpp,cpp}`
    - 単一 BSDF 内で Lambert 拡散 + GGX マイクロファセット鏡面 + transmission/ior を統合。
- `src/renderer/renderer.{hpp,cpp}`
    - `trace_ray` で `IBSDF::sample/eval/pdf` を利用。
    - 影評価では交差の入出点から厚みを求め、Beer-Lambert で透過光を減衰。
- `src/scene`, `src/object`, `src/math`
    - 幾何・交差判定・数学基盤を提供し、BSDF と独立に保守可能。

## BSDF 契約

- `sample(wo, hit, material)`
    - 次方向 `wi` をサンプルし、再帰寄与重み `weight` を返す。
- `eval(wo, wi, hit, material)`
    - 与えた方向対の BSDF 値を返す（主に direct lighting で使用）。
- `pdf(wo, wi, hit, material)`
    - サンプル分布の確率密度を返す（MIS や検証に利用可能）。

## PBR モデル

- Diffuse: Lambert
    - $f_d = \frac{(1-t)(1-m)\,\mathrm{base\_color}}{\pi}$
- Specular: GGX (Trowbridge-Reitz)
    - $f_s = (1-t)\frac{D\,G\,F}{4(n\cdot wo)(n\cdot wi)}$
    - $D$: GGX NDF, $G$: Smith (Schlick-GGX), $F$: Schlick Fresnel
- Transmission: dielectric refraction
    - `transmission` と `ior` を使って屈折方向をサンプリングし、全反射時は反射へフォールバック
- Volume attenuation: Beer-Lambert
    - 透明物の内部厚み $d$ に対して $T(d)=\exp(-\sigma_a d)$ を適用
    - $\sigma_a$ は `absorption_coefficient`（RGBの吸収係数）

ここで $m$ は metallic、$t$ は transmission。

## なぜ疎結合か

- `Renderer` は `IBSDF` だけに依存しているため、`PbrBsdf` を `DisneyBsdf` や `OrenNayarBsdf` に交換してもレンダラー改修が不要。
- `Material` は純粋データなので、将来テクスチャ入力を加える場合も `Material` の供給層と BSDF 評価層を独立に発展できる。
- CMake では `raytracer_core` ライブラリ化を行い、レンダリング本体とエントリーポイント (`main`) を分離してテスト導入を容易化。
