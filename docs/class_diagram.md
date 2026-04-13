# クラス図

```mermaid
classDiagram
    class Texture {
        <<interface>>
        +value(u, v, p) Color
    }

    class SolidColor {
        +value(u, v, p) Color
    }

    class CheckerTexture {
        +value(u, v, p) Color
    }

    class ImageTexture {
        +load(path) bool
        +value(u, v, p) Color
        +is_valid() bool
    }

    class Material {
        +shared_ptr~Texture~ albedo_texture
        +shared_ptr~Texture~ emission_texture
        +float metallic
        +float roughness
        +float transmission
        +float ior
        +Color absorption_coefficient
        +sample_albedo(u, v, p) Color
        +sample_emission(u, v, p) Color
    }

    class EnvironmentMap {
        +load_hdr(path) bool
        +sample(direction) Color
        +set_intensity(value)
        +is_valid() bool
    }

    class BsdfSample {
        +Vec3 wi
        +Color weight
        +float pdf
        +bool valid
        +bool is_delta
    }

    class IBSDF {
        <<interface>>
        +sample(wo, hit, material) BsdfSample
        +eval(wo, wi, hit, material) Color
        +pdf(wo, wi, hit, material) float
    }

    class PbrBsdf {
        +sample(...)
        +eval(...)
        +pdf(...)
    }

    class Scene {
        -vector~shared_ptr~Object~~ objects
        -vector~Material~ materials
        -shared_ptr~Object~ bvh_root
        -bool bvh_dirty
        -EnvironmentMap environment_map
        -bool has_environment
        +find_closest_hit(...)
        +add_object(...)
        +set_environment_map(...)
        +sample_environment(direction) Color
    }

    class Renderer {
        -unique_ptr~IBSDF~ bsdf
        +render(scene, camera)
        -trace_ray(ray, scene, depth)
        -evaluate_shadow_transmittance(scene, shadow_ray)
    }

    class Object {
        <<abstract>>
        +hit(ray, t_min, t_max, rec)*
    }

    class Sphere

    class BVHNode

    class MaterialOptics {
        +beer_lambert_transmittance(material, distance) Color
    }

    SolidColor ..|> Texture
    CheckerTexture ..|> Texture
    ImageTexture ..|> Texture
    PbrBsdf ..|> IBSDF
    Renderer --> IBSDF : evaluates via abstraction
    Renderer ..> PbrBsdf : default implementation
    Renderer --> Scene
    Material --> Texture : samples
    Scene --> Material : owns
    Scene --> Object : owns
    Scene --> BVHNode : builds/uses
    Scene --> EnvironmentMap : optional
    Sphere --|> Object
    BVHNode --|> Object
    IBSDF --> Material : evaluates
    IBSDF --> BsdfSample : returns
    Renderer --> MaterialOptics : shadow attenuation
```

ポイント:

- `Renderer` の散乱評価は `IBSDF` 経由で実行され、既定実装として `PbrBsdf` を使用できます。
- `Scene` は `Object` を `shared_ptr` で保持し、BVH ノードから同じプリミティブを参照します。
- `Material` はテクスチャをサンプリングし、散乱評価は `IBSDF` 実装側に委譲します。
- 環境光は `Scene::sample_environment` に隠蔽され、背景色と HDRI の切り替えが同一 API で扱えます。