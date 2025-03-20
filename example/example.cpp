#include <cmath>

#include "../ecs.hpp"

#include <raylib.h>
#include <rlgl.h>

struct Cube {
  Color color;
  Vector3 position;
  Vector3 size;
  float rotation;
};

struct Rotation {};

struct Translation {};

using ECS = ecs::ecs<Cube, Rotation, Translation>;

int main() {

  ECS ecs;
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
  std::vector<Cube *> cubes = {cube1, cube2, cube3, cube4};

  cube1->color = BLUE;
  cube2->color = GREEN;
  cube3->color = RED;
  cube4->color = YELLOW;

  cube1->position = (Vector3){-2.0, 0.0, -2.0};
  cube2->position = (Vector3){-2.0, 0.0, +2.0};
  cube3->position = (Vector3){+2.0, 0.0, -2.0};
  cube4->position = (Vector3){+2.0, 0.0, +2.0};

  cube1->size = {1.00, 1.00, 1.00};
  cube2->size = {1.50, 1.50, 1.50};
  cube3->size = {0.50, 0.50, 0.50};
  cube4->size = {0.75, 0.75, 0.75};

  cube1->rotation = 0.0;
  cube2->rotation = 0.0;
  cube3->rotation = 45.0;
  cube4->rotation = 45.0;

  ecs.components.add<Rotation>(entity3);
  ecs.components.add<Rotation>(entity4);
  ecs.components.add<Translation>(entity2);
  ecs.components.add<Translation>(entity4);

  InitWindow(1280, 720, "example");
  SetTargetFPS(60);

  Camera3D camera;
  camera.position = (Vector3){8.0f, 8.0f, 8.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    // Rotation system
    for (ecs::entity_id entity : {entity1, entity2, entity3, entity4}) {
      if (ecs.components.has<Rotation>(entity)) {
        Cube *cube = ecs.components.get<Cube>(entity);
        cube->rotation += 90.0 * GetFrameTime();
      }
    }

    // Translation system
    for (ecs::entity_id entity : {entity1, entity2, entity3, entity4}) {
      if (ecs.components.has<Translation>(entity)) {
        Cube *cube = ecs.components.get<Cube>(entity);
        cube->position.y = std::sin(GetTime());
      }
    }

    DrawGrid(10, 1.0f);
    for (Cube *cube : cubes) {
      rlPushMatrix();
      // Rotate in-place
      rlTranslatef(cube->position.x, cube->position.y, cube->position.z);
      rlRotatef(cube->rotation, 0, 1, 0);
      rlTranslatef(-cube->position.x, -cube->position.y, -cube->position.z);

      DrawCubeV(cube->position, cube->size, cube->color);
      rlPopMatrix();
    }

    EndMode3D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}