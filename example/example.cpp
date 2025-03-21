#include <raylib.h>
#include <rlgl.h>

#include <cmath>

#include "../ecs.hpp"

struct World;
struct Cube;
struct Rotation;
struct Translation;

using ECS = ecs::engine<World, Cube, Rotation, Translation>;

struct World {
    ECS*  ecs;
    float dt;
    float time;
};

struct Cube {
    Color   color;
    Vector3 position;
    Vector3 size;
    float   rotation;
};

struct Rotation {
    float speed;
};

struct Translation {
    float height;
};

void rotation_system(World* world) {
    for (auto [_, cube, rotation] : world->ecs->iterate<Cube, Rotation>()) {
        cube->rotation += rotation->speed * world->dt;
    }
}

void translation_system(World* world) {
    for (auto [_, cube, translation] : world->ecs->iterate<Cube, Translation>()) {
        cube->position.y = translation->height * std::sin(world->time);
    }
}

void draw_cube_system(Cube* cube) {
    rlPushMatrix();
    // Rotate in-place
    rlTranslatef(cube->position.x, cube->position.y, cube->position.z);
    rlRotatef(cube->rotation, 0, 1, 0);
    rlTranslatef(-cube->position.x, -cube->position.y, -cube->position.z);

    DrawCubeV(cube->position, cube->size, cube->color);
    rlPopMatrix();
}

int main() {
    ECS ecs;

    ecs::entity_id entityw = ecs.entities.create();
    ecs.components.add<World>(entityw);

    ecs::entity_id entity1 = ecs.entities.create();
    ecs::entity_id entity2 = ecs.entities.create();
    ecs::entity_id entity3 = ecs.entities.create();
    ecs::entity_id entity4 = ecs.entities.create();

    ecs.components.add<Cube>(entity1);
    ecs.components.add<Cube>(entity2);
    ecs.components.add<Cube>(entity3);
    ecs.components.add<Cube>(entity4);

    auto cube1 = ecs.components.get<Cube>(entity1);
    auto cube2 = ecs.components.get<Cube>(entity2);
    auto cube3 = ecs.components.get<Cube>(entity3);
    auto cube4 = ecs.components.get<Cube>(entity4);

    cube1->color = BLUE;
    cube2->color = GREEN;
    cube3->color = RED;
    cube4->color = YELLOW;

    cube1->position = Vector3 {-2.0, 0.0, -2.0};
    cube2->position = Vector3 {-2.0, 0.0, +2.0};
    cube3->position = Vector3 {+2.0, 0.0, -2.0};
    cube4->position = Vector3 {+2.0, 0.0, +2.0};

    cube1->size = {1.00, 1.00, 1.00};
    cube2->size = {1.50, 1.50, 1.50};
    cube3->size = {0.50, 0.50, 0.50};
    cube4->size = {0.75, 0.75, 0.75};

    cube1->rotation = 0.0;
    cube2->rotation = 0.0;
    cube3->rotation = 45.0;
    cube4->rotation = 45.0;

    ecs.components.add<Rotation>(entity3, 90.0f);
    ecs.components.add<Rotation>(entity4, 45.0f);
    ecs.components.add<Translation>(entity2, 1.0f);
    ecs.components.add<Translation>(entity4, 2.0f);

    InitWindow(1280, 720, "example");
    SetTargetFPS(60);

    Camera3D camera;
    camera.position   = Vector3 {8.0f, 8.0f, 8.0f};
    camera.target     = Vector3 {0.0f, 0.0f, 0.0f};
    camera.up         = Vector3 {0.0f, 1.0f, 0.0f};
    camera.fovy       = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        auto [_, world] = ecs.components.first<World>();
        world->ecs      = &ecs;
        world->dt       = GetFrameTime();
        world->time     = GetTime();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        DrawGrid(10, 1.0f);

        ecs.systems.execute(rotation_system);
        ecs.systems.execute(translation_system);
        ecs.systems.execute(draw_cube_system);

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}