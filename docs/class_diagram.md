# クラス図（BSDF リファクタ後）

```mermaid
classDiagram
    class Material {
        +Color base_color
        +float metallic
        +float roughness
        +float transmission
        +float ior
        +Color absorption_coefficient
        +Color emission
    }

    class MaterialOptics {
        +beer_lambert_transmittance(material, distance) Color
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
        -GGX_D(...)
        -GGX_G(...)
        -Schlick_F(...)
    }

    class Renderer {
        -unique_ptr~IBSDF~ bsdf
        +render(scene, camera)
        -trace_ray(ray, scene, depth)
        -evaluate_shadow_transmittance(scene, shadow_ray)
    }

    class Scene {
        -vector~unique_ptr~Object~~ objects
        -vector~Material~ materials
        +find_closest_hit(...)
        +get_material(...)
    }

    class Object {
        <<abstract>>
        +hit(ray, t_min, t_max, rec)*
    }

    class Sphere

    PbrBsdf ..|> IBSDF
    Renderer --> IBSDF : uses abstraction only
    Renderer --> Scene
    Scene --> Material : owns
    Scene --> Object : owns
    Sphere --|> Object
    IBSDF --> Material : evaluates parameters
    IBSDF --> BsdfSample : returns
    Renderer --> MaterialOptics : uses for shadow attenuation
```

ポイント:

- `Renderer -> IBSDF` の依存逆転により、具体 BSDF 実装追加時もレンダラー変更は不要。
- `Material` はロジックを持たないため、責務は「パラメータ表現」に限定される。