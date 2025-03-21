# Single-header ECS

Single-header Entity-Component-System framework written for C++20. This framework makes heavy usage of templating to allow for quick access of entities and components with easy-to-read and intuitive syntax.



# Example usage

```C++
#include "ecs.hpp"

struct Transform { float x, y, rotation; };
struct Velocity  { float x, y; };
struct Rotation  { float speed; };

using ECS = ecs::ecs<Position, Velocity, Rotation>;

void movement_system(Transform* transform, Velocity* velocity) {
    transform.x += velocity.x * GetDeltaTime();
    transform.y += velocity.y * GetDeltaTime();
}

void rotation_system(ecs::entity_id entity, Transform* transform, Rotation* rotation) {
    if (entity % 2 == 0)
        transform.rotation += rotation.speed * GetDeltaTime();
    else
        transform.rotation -= rotation.speed * GetDeltaTime();
}

// ...

int main() {
    ECS ecs;

    // Create entities with components
    ecs::entity_id player = ecs.entities.create();
    ecs.components.add<Transform>(player, 0.0, 0.0, 0.0);
    ecs.components.add<Velocity>(player, 0.0, 0.0);
    ecs.components.add<Rotation>(player, 0.0);
    
    // ...

    while (1) {
        // Call systems
        ecs.systems.execute(movement_system);
        ecs.systems.execute(rotation_system);
       
        // ...
    }

    // Remove entities, this also removes their components
    ecs.entities.remove(player);

    return 0;
}
```

# Framework

The ECS-framework consists of three parts:
- The entities, which represent an object.
- The components, which contain the data an entity holds.
- The systems, which run on all entities and manipulate their state.

This framework exposes a new class to easily manage entities and components, and flexibly execute systems.

# API

The framework exposes two new types: `ecs::entity_id` and `ecs::ecs`:
- `ecs::entity_id` is an integer type that represents a unique id of an entity within an ECS system.
- `ecs::ecs` is the entire ECS system. It is composed of three parts: the `entities` part, the `components` part, and the `systems` part. `ecs::ecs` also allows iteration over all entities with certain components.

## Creation

An ECS object is created by specifying which components it will hold. For example, consider an ECS with a Position component, a Velocity component and a Render component. The environment is then created as:

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<Position, Velocity, Render> my_ecs;
    return 0;
}
```

It is possible to create an alias to increase readability, if desired.

```C++
#include "ecs.hpp"

using ECS = ecs::ecs<Position, Velocity, Render>;

int main() {
    ECS ecs;
    return 0;
}
```

The components allowed in the ECS have the following restrictions:
- All components must be unique, i.e. each component can only be listed once.
- All components must be structs or classes.
- All components must have a default constructor.

```C++
#include "ecs.hpp"

struct NonDefaultConstructor {
    NonDefaultConstructor(int x) {}
};

int main() {
    ecs::ecs<Position, Velocity> ecs1; // allowed, all components are unique structs/classes.
    ecs::ecs<EmptyStruct>        ecs2; // allowed, all components are unique structs/classes.
    ecs::ecs<>                   ecs3; // allowed, all components are unique structs/classes.

    ecs::ecs<Position, int>         ecs4; // not allowed, int is not a struct.
    ecs::ecs<Position, Position>    ecs5; // not allowed, Position appears multiple times.
    ecs::ecs<NonDefaultConstructor> ecs6; // not allowed, component does not have a default constructor.

    return 0;
}
```

Static asserts have been added to help navigate compilation errors in case any of the conditions is not met.

## Entities

Entities are represented by numerical ids. Entities are handled in the `ecs.entities` field. Entities can be created, removed and queried to see if they exist:

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<...> ecs;

    ecs::entity_id entity = ecs.entities.create(); // create a new entity
    bool exists  = ecs.entities.exists(entity);    // check if entity exists
    bool removed = ecs.entities.remove(entity);    // delete entity

    return 0;
}
```

The `ecs.entities.remove` method returns true if the entity existed and is now removed, otherwise it returns false.

New entities are guaranteed to have a unique id compared to all other existing entities. The ids of removed entities can (and will) be re-used later for new entities. Entities can have a maximum value of 2^64, which should be large enough for all practical purposes.

## Components

Components are data objects which represent the state of an entity. An entity can have one or more different components. Components can be added, removed and queried for their existence. Additionally, the first component of a type can be queried, based on the numerical value of the entities.

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<Position, ...> ecs;

    // Create entity
    ecs::entity_id entity = ecs.entities.create(); 
    
    // Add component to the entity
    Position* pos1 = ecs.components.add<Position>(entity, ...);
    
    // Check if entity has component
    bool exists  = ecs.components.has<Position>(entity); 
    // Remove component from entity
    bool removed = ecs.components.remove<Position>(entity); 
    
    // Get first entity and component of type
    auto [first_entity, first_position] = ecs.components.first<Position>();

    return 0;
}
```

The component type is given as a template in each method. This type must be registered in the ECS constructor, otherwise an error will be thrown.

`ecs.components.add` returns a pointer to the created component. If the component could not be created. Additionally, the arguments for the constructor of the component can be given after the entity id. If the entity does not exist, the component is not created and a null pointer is returned instead.

`ecs.components.has` checks whether an entity has a component of the given type. If the entity does not exist or the entity does exist but does not have the component, false is returned. Otherwise it returns true.

`ecs.components.remove` removes the component from an entity. If the entity does not exist or the entity does exist but does not have the component, this method does nothing and returns false. Otherwise it deletes the component and return true;

`ecs.components.first` returns the first entity and component pointer in the ECS for the component type. First is here defined as the first entity numerically, and is not guaranteed to be the earliest created component or entity that has the component type. The method returns a std::tuple of the entity id and pointer to the component. If not a single component of the type exists, it returns a tuple of the invalid entity id (defined as SIZE_MAX by the compiler) and a null pointer. This method should be particularly useful for components where it's expected only one of them will exist, such as a World component.

## Systems

Systems are functions which act on the components entities have. Systems are defined as functions with an (optional) entity id argument and the requested components. A valid system can be executed by the ECS.

```C++
#include <iostream>

#include "ecs.hpp"

void move_system(ecs::entity entity, Position* pos, Velocity* vel) {
    std::cout << "Updating movement for entity " << entity << std::endl;
    pos->x += vel->x;
    pos->y += vel->y;
}

int main() {
    ecs::ecs<Position, Velocity> ecs;
    // ...

    while (1) {
        ecs.systems.execute(move_system);
        // ...
    }

    return 0;
}

```

A system is a function that returns void and takes in an `ecs::entity_id` and zero or more registered component pointer types. The ECS will then iterate over all entities that have all listed components and execute the system for each found entity.

For example, in the code snippet above the ECS will iterate over all entities that have both a Position component and a Velocity component and apply the movement. It will not execute the system for entities that have a Position component but not a Velocity component, vice-versa, or neither of them.

A variation of the signature also allows the entity id to not be added. The signatures

```C++
void move_system(ecs::entity_id entity, Position* pos, Velocity* vel);
void move_system(Position* pos, Velocity* vel);
```

are both allowed. The only difference is that the second signature cannot use the entity id in its body. Additionally, systems must be function pointers, lambdas are not supported.

If no component types are listed in the signature, the ECS will iterate over all entities.

Creating and deleting components of a type within the system's signature is heavily discouraged, as this could result in the internal storage of components moving and thus invalidating the component pointers, potentially resulting in undefined behavior. For more information, see [Component location and lifetime](#component-location-and-lifetime).

## Iterating

It's also possible to iterate over the entities and components directly, without needing to use systems. This can be done using the `ecs.iterate` method.

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<Position, Velocity> ecs;

    for (auto [entity, position] : ecs.iterate<Position>()) {
        // ...
    }

    return 0;
}
```

The code snippet above will iterate over all entities that have a Position component. It is possible to have give multiple component types. In this case, it will iterate over all entities that have all those types. For example, in the code snippet below, the for loop will iterate over all entities that have a Position component, a Velocity component, and a Rotation component.

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<Position, Velocity, Rotation> ecs;

    for (
        auto [entity, position, velocity, rotation] 
        : ecs.iterate<Position, Velocity, Rotation>()) {
        // ...
    }

    return 0;
}
```

The item in each iteration will be a std::tuple containing the entity id and pointers to the requested components. A variation exists named `ecs.iterate_components` where the entity id is not included.

```C++
#include "ecs.hpp"

int main() {
    ecs::ecs<Position, Velocity> ecs;

    for (auto [position, velocity] : ecs.iterate_components<Position, Velocity>()) {
        // ...
    }

    return 0;
}
```

Similar to systems, it's heavily discouraged to create or delete components of a type while iterating over the same type, as this could cause the pointers to become invalidated, which could result in undefined behavior.

# Sharing global state 

The systems execute function does not allow additional parameters to be given, and will only (optionally) contain the entity id and the requested components. A way to handle this would be to create a single entity with a single component that represents shared data, and then calling a system using only that component.

```C++
#include "ecs.hpp"

struct World;
struct Transform;
struct Velocity;

using ECS = ecs::ecs<World, Transform, Velocity>;

struct World     { ECS* ecs; float dt; };
struct Transform { float x, y; };
struct Velocity  { float x, y; };

void movement_system(World* world) {
    for (auto [_, transform, velocity] : world->ecs.iterate<Transform, Velocity>()) {
        transform.x += velocity.x * world->dt;
        transform.y += velocity.y * world->dt;
    }
}

// ...

int main() {
    ECS ecs;

    // Create entities with components
    ecs::entity_id world = ecs.entities.create();
    ecs.components.add<World>(world, &ecs, 0.0);

    ecs::entity_id player = ecs.entities.create();
    ecs.components.add<Transform>(player, 0.0, 0.0, 0.0);
    ecs.components.add<Velocity>(player, 0.0, 0.0);

    while (1) {
        // Update delta time and world state
        float dt = 1.0 / 60.0;
        auto [_, world_component] = ecs.components.first<World>();
        world_component->dt = dt;

        // Call systems
        ecs.systems.execute(movement_system);
        
        // ...
    }

    // Remove entities
    ecs.entities.remove(player);
    ecs.entities.remove(world);

    return 0;
}
```

# Component location and lifetime

To maximize efficiency and cache-behavior, components are internally stored as a vector plain of structs and not as pointers to component structs. This has several consequences:

- Don't create new components within an iteration of the same component type. Creating new components might cause the vector to resize itself, potentially the existing moving in memory and invalidating the previous pointers. This also applies to creating components in a system where the component is used.
- Don't store pointers to components. As mentioned in the previous point, adding components might cause the existing components to be relocated to another place in memory, invalidating the previous pointers. Instead request the components again whenever you need them from the entity id.
- Unused components are internally stored as components created with the default constructor. It's thus adviced to keep the constructors of the components as simple as possible, to prevent overhead when creating and deleting components. It is recommended to use components as simple plain-old-data objects.
- When a component is deleted, it is internally replaced with a component created with the default constructor. This ensures that the destructor of the previous is called.