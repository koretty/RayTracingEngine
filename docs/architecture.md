# Architecture Overview

このリポジトリは、SDL3 でのリアルタイム表示ループを入口に、CPU パストレーシングを実行する学習向けレイトレーシングエンジンです。

## モジュール一覧と役割

- src/main/main.cpp: SDL 初期化、イベントループ、カメラ移動入力、レンダリング再実行トリガ、PPM保存を管理。
- src/main/config/camera_config.hpp: カメラ位置・視点・FOV・絞り・移動速度と Camera 生成ロジック。
- src/main/config/scene_config.hpp: シーン構築（背景、太陽光、マテリアル、球体オブジェクト配置）。
- src/renderer/renderer.{hpp,cpp}: ピクセル走査、サンプリング、OpenMP 並列化、再帰的な trace_ray による色計算。
- src/scene/scene.{hpp,cpp}: Object 群と Material 群の保持、最短交差判定、環境光/太陽光パラメータ提供。
- src/scene/camera.hpp: レンズ半径と焦点距離を使ったレイ生成（被写界深度対応）。
- src/object/object.hpp: 交差判定インターフェースと HitRecord の定義。
- src/object/sphere.{hpp,cpp}: 球体のレイ交差判定を実装。
- src/material/material.hpp: 拡散/金属/透過（屈折）散乱と発光、簡易 Fresnel 反射率。
- src/math/{vec3,ray,math_utils}.hpp: ベクトル演算、Ray、乱数とサンプリング補助。

## 設計原則

- 責務分離: math / object / material / scene / renderer / main-config を分離し、学習しやすい最小単位で拡張可能にする。
- 描画モデル: 1ピクセルあたり複数サンプルを取り、trace_ray の再帰で間接光を近似する CPU パストレーシング。
- 対話性: 入力があった時のみ再レンダリングするフラグ駆動ループで、操作感と処理負荷のバランスを取る。
- 実用速度: OpenMP schedule(dynamic) による scanline 並列化を採用。

## クラス図（概要）

```mermaid
classDiagram
    class Vec3 {
        +float x
        +float y
        +float z
        +length()
        +length_squared()
        +near_zero()
    }

    class Ray {
        +getOrigin()
        +getDirection()
        +at(t)
    }

    class Camera {
        +get_ray(u, v)
    }

    class HitRecord {
        +Point3 point
        +Vec3 normal
        +float t
        +int material_id
    }

    class Object {
        <<abstract>>
        +hit(ray, t_min, t_max, rec)
    }

    class Sphere {
        +hit(ray, t_min, t_max, rec)
    }

    class Material {
        +Color base_color
        +float metallic
        +float roughness
        +float transmission
        +float ior
        +Color emission
        +scatter(r_in, rec, attenuation, scattered)
    }

    class Scene {
        +add_object(object)
        +add_material(mat)
        +find_closest_hit(ray, t_min, t_max, rec)
        +get_background()
        +get_sun_direction()
    }

    class Renderer {
        +render(scene, camera)
        +get_pixels()
        -trace_ray(ray, scene, depth)
        -to_color32(color)
    }

    Sphere --|> Object
    Scene --> Object : owns
    Scene --> Material : owns
    Renderer --> Scene : samples
    Renderer --> Camera : casts rays
    Renderer --> Ray : traces
    Material --> Ray : scatters
    Material --> HitRecord : reads hit info
    Object --> HitRecord : writes
    Camera --> Vec3 : uses basis vectors
    Ray --> Vec3
```
