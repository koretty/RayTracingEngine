#pragma once

#include "../../scene/scene.hpp"
#include "../../material/material.hpp"
#include "../../object/sphere.hpp"

namespace config {
namespace scene {

inline Scene create_scene() {
    Scene sc;

    sc.set_background(Color(0.60f, 0.80f, 1.00f));

    sc.add_material(Material::matte(Color(0.8f, 0.8f, 0.75f))); 
    sc.add_material(Material::metal(Color(0.9f, 0.85f, 0.8f)));    
    sc.add_material(Material::glass(1.5f));                       
    sc.add_material(Material::matte(Color(0.9f, 0.2f, 0.2f)));      


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
