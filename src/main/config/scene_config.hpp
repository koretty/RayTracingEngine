#pragma once

#include "../../scene/scene.hpp"
#include "../../material/material.hpp"
#include "../../object/sphere.hpp"

namespace config {
namespace scene {

inline Scene create_scene() {
    Scene sc;

    sc.set_background(Color(0.60f, 0.80f, 1.00f));

    Material ground;
    ground.base_color = Color(0.33f, 0.52f, 0.28f);
    ground.metallic = 0.0f;
    ground.roughness = 0.97f;

    Material brushed_metal;
    brushed_metal.base_color = Color(0.9f, 0.85f, 0.8f);
    brushed_metal.metallic = 1.0f;
    brushed_metal.roughness = 0.2f;

    Material glass;
    glass.base_color = Color(0.98f, 0.99f, 1.0f);
    glass.metallic = 0.0f;
    glass.roughness = 0.02f;
    glass.transmission = 1.0f;
    glass.ior = 1.5f;
    glass.absorption_coefficient = Color(5.0f, 5.0f, 10.0f);

    Material red_matte;
    red_matte.base_color = Color(0.9f, 0.2f, 0.2f);
    red_matte.metallic = 0.0f;
    red_matte.roughness = 0.8f;

    sc.add_material(ground);
    sc.add_material(brushed_metal);
    sc.add_material(glass);
    sc.add_material(red_matte);


    sc.add_object(std::make_unique<Sphere>(Point3(0.0f, -1000.0f, 0.0f), 1000.0f, 0));
    float spacing = 2.2f;
    sc.add_object(std::make_unique<Sphere>(Point3(-spacing, 0.5f,  1.0f), 0.5f, 1));
    sc.add_object(std::make_unique<Sphere>(Point3(0.0f,     0.5f,  0.0f), 0.5f, 2));
    sc.add_object(std::make_unique<Sphere>(Point3(spacing,  0.5f, -1.0f), 0.5f, 3));

    sc.set_sun_direction(Vec3(-0.6f, -1.0f, -0.4f));
    sc.set_sun_intensity(0.5f);
    sc.set_sun_color(Color(1.00f, 0.97f, 0.88f));

    return sc;
}

} 
} 
