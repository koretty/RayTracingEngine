#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>
#include <chrono>
#include <memory>
#include <fstream>

#include "config/camera_config.hpp"
#include "config/scene_config.hpp"
#include "../renderer/renderer.hpp"

// 画像をPPM形式で保存する関数
void save_image(const char* filename, const uint32_t* pixels, int width, int height) {
    std::ofstream out(filename);
    out << "P3\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < width * height; i++) {
        uint32_t pixel = pixels[i];
        int r = (pixel >> 16) & 0xFF;
        int g = (pixel >> 8) & 0xFF;
        int b = pixel & 0xFF;
        out << r << " " << g << " " << b << "\n";
    }
    std::cout << "Image saved to " << filename << "\n";
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)){
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return -1;
    }

    int width = 800;
    int height = 600;

    SDL_Window* window = SDL_CreateWindow("RayTracing - Configurable", width, height, 0);
    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture* texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!window || !sdl_renderer || !texture) {
        std::cerr << "Create Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    Scene scene = config::scene::create_scene();

    Renderer rt_renderer(width, height, 100, 10);
    bool isRunning = true;
    SDL_Event event;

    bool needs_render = true;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false; 
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                Vec3 front = unit_vector(config::camera::lookat - config::camera::origin);
                Vec3 right = unit_vector(cross(front, config::camera::vup));

                bool moved = false;
                switch (event.key.key) {
                    case SDLK_ESCAPE: isRunning = false; break;
                    case SDLK_W: config::camera::origin += front * config::camera::move_speed; config::camera::lookat += front * config::camera::move_speed; moved = true; break;
                    case SDLK_S: config::camera::origin -= front * config::camera::move_speed; config::camera::lookat -= front * config::camera::move_speed; moved = true; break;
                    case SDLK_A: config::camera::origin -= right * config::camera::move_speed; config::camera::lookat -= right * config::camera::move_speed; moved = true; break;
                    case SDLK_D: config::camera::origin += right * config::camera::move_speed; config::camera::lookat += right * config::camera::move_speed; moved = true; break;
                    case SDLK_U: config::camera::origin.y += config::camera::move_speed; config::camera::lookat.y += config::camera::move_speed; moved = true; break;
                    case SDLK_Y: config::camera::origin.y -= config::camera::move_speed; config::camera::lookat.y -= config::camera::move_speed; moved = true; break;
                    case SDLK_P:
                        save_image("render_output.ppm", (const uint32_t*)rt_renderer.get_pixels(), width, height);
                        break;
                }
                if (moved) needs_render = true;
            }
        }

        if (needs_render) {
            Camera camera = config::camera::make_camera(width, height);

            auto start_time = std::chrono::high_resolution_clock::now();
            rt_renderer.render(scene, camera);
            auto end_time = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> elapsed = end_time - start_time;
            std::cout << "Render time: " << elapsed.count() << " s\n";

            SDL_UpdateTexture(texture, NULL, rt_renderer.get_pixels(), width * sizeof(uint32_t));
            needs_render = false;
        }

        SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl_renderer);
        SDL_FRect rect = { 0.0f, 0.0f, (float)width, (float)height };
        SDL_RenderTexture(sdl_renderer, texture, NULL, &rect);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
