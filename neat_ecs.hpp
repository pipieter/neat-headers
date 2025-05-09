#ifndef NEAT_ECS_HPP_
#define NEAT_ECS_HPP_

#include <cstdint>
#include <cstdlib>
#include <queue>
#include <tuple>
#include <type_traits>
#include <vector>

namespace neat::ecs {

namespace typing {

template <typename...>
inline constexpr auto are_unique_types = std::true_type {};

template <typename Type, typename... Rest>
inline constexpr auto are_unique_types<Type, Rest...> = std::bool_constant<(!std::is_same_v<Type, Rest> && ...) && are_unique_types<Rest...>> {};

template <typename T, typename... Ts>
constexpr bool is_one_of = (std::is_same<T, Ts> {} || ...);

template <typename Type, typename Head, typename... Tail>
constexpr int get_index() {
    static_assert(is_one_of<Type, Head, Tail...>,
                  "Type not part of accepted types.");
    if constexpr (std::is_same<Type, Head>::value)
        return 0;
    else
        return 1 + get_index<Type, Tail...>();
}

template <typename...> inline constexpr auto are_all_classes = std::true_type {};

template <typename T, typename... Ts>
inline constexpr auto are_all_classes<T, Ts...> = std::bool_constant<(std::is_class_v<T>)&&are_all_classes<Ts...>> {};

template <typename Subset, typename Set> constexpr bool is_subset_of = false;
template <typename... Ts, typename... Us>
inline constexpr bool is_subset_of<std::tuple<Ts...>, std::tuple<Us...>> = (is_one_of<Ts, Us...> && ...);

}  // namespace typing

using entity_id                = std::size_t;
const entity_id invalid_entity = SIZE_MAX;

template <typename ComponentType>
class componentlist {
   private:
    std::vector<bool>          _tags;
    std::vector<ComponentType> _components;

   public:
    componentlist();
    ~componentlist();

    template <typename... Args>
    ComponentType* add(entity_id entity, Args... args);
    ComponentType* get(entity_id entity);

    bool has(entity_id entity) const;
    bool remove(entity_id entity);
    bool allocate(size_t new_count);

    std::tuple<entity_id, ComponentType*> first();
};

template <typename... RegisteredComponents>
class engine {
   private:
    class entities;
    class components;
    class systems;

   public:
    engine();
    ~engine();

    template <typename... RequestedComponents> std::vector<std::tuple<entity_id, RequestedComponents*...>> iterate();
    template <typename... RequestedComponents> std::vector<std::tuple<RequestedComponents*...>>            iterate_components();

    entities   entities;
    components components;
    systems    systems;

   private:
    template <typename RequestedComponent> componentlist<RequestedComponent>&                 _get_components_list();
    template <typename... RequestedComponents> std::vector<entity_id>                         _get_entities_who_have_components();
    template <typename... RequestedComponents> std::tuple<entity_id, RequestedComponents*...> _get_entity_components_with_entity(entity_id entity);
    template <typename... RequestedComponents> std::tuple<RequestedComponents*...>            _get_entity_components_without_entity(entity_id entity);

   private:
    class entities final {
       private:
        friend class engine;
        engine&               _ecs;
        std::vector<bool>     _entities;
        std::queue<entity_id> _free_entities;
        explicit entities(engine& e);

       public:
        entity_id              create();
        bool                   remove(entity_id entity);
        bool                   exists(entity_id entity) const;
        entity_id              last() const;
        std::vector<entity_id> all() const;
    };

    class components final {
       private:
        friend class engine;
        engine&                                            _ecs;
        std::tuple<componentlist<RegisteredComponents>...> _components;
        explicit components(engine& e);

       public:
        template <typename RequestedComponent> RequestedComponent*                   get(entity_id entity);
        template <typename RequestedComponent> std::vector<RequestedComponent*>      get(const std::vector<entity_id>& entity_list);
        template <typename RequestedComponent, typename... Args> RequestedComponent* add(entity_id entity, Args... args);

        template <typename RequestedComponent> bool has(entity_id entity) const;
        template <typename RequestedComponent> bool remove(entity_id entity);
        template <typename RequestedComponent> bool allocate(size_t new_size);
        bool                                        allocate_all(size_t new_size);

        template <typename RequestedComponent> std::tuple<entity_id, RequestedComponent*> first();
    };

    class systems final {
       private:
        friend class engine;
        engine& _ecs;
        explicit systems(engine& e);

       public:
        template <typename... FuncComponents> void execute(void (&system)(FuncComponents*...));
        template <typename... FuncComponents> void execute(void (&system)(entity_id, FuncComponents*...));
        template <typename... FuncComponents> void execute(void (&system)(engine<RegisteredComponents...>&, FuncComponents*...));
        template <typename... FuncComponents> void execute(void (&system)(engine<RegisteredComponents...>&, entity_id, FuncComponents*...));
    };
};
};  // namespace neat::ecs

#pragma region componentlist implementations

template <typename ComponentType>
neat::ecs::componentlist<ComponentType>::componentlist() {
    static_assert(std::is_class_v<ComponentType>, "Component type is not a struct or class.");
    static_assert(std::is_default_constructible_v<ComponentType>, "Component type does not have a default constructor.");
}

template <typename ComponentType>
neat::ecs::componentlist<ComponentType>::componentlist::~componentlist() {}

template <typename ComponentType>
bool neat::ecs::componentlist<ComponentType>::componentlist::has(
    entity_id entity) const {
    if (entity >= _tags.size())
        return false;
    return _tags[entity];
}

template <typename ComponentType>
ComponentType* neat::ecs::componentlist<ComponentType>::componentlist::get(entity_id entity) {
    if (!has(entity))
        return nullptr;
    return &_components[entity];
}

template <typename ComponentType>
template <typename... Args>
ComponentType* neat::ecs::componentlist<ComponentType>::componentlist::add(neat::ecs::entity_id entity, Args... args) {
    static_assert(std::is_constructible_v<ComponentType, Args...>, "Component type can't be built from given arguments.");
    if (entity >= _tags.size()) {
        _tags.resize(entity + 1, false);
        _components.resize(entity + 1);
    }

    _tags[entity]       = true;
    _components[entity] = ComponentType(args...);
    return &_components[entity];
}

template <typename ComponentType>
bool neat::ecs::componentlist<ComponentType>::componentlist::remove(entity_id entity) {
    if (!has(entity))
        return false;
    _tags[entity]       = false;
    _components[entity] = ComponentType();  // Replace with a default component
    return true;
}

template <typename ComponentType>
std::tuple<neat::ecs::entity_id, ComponentType*> neat::ecs::componentlist<ComponentType>::componentlist::first() {
    for (entity_id entity = 0; entity < _tags.size(); entity++) {
        if (_tags[entity]) {
            return {entity, &_components[entity]};
        }
    }
    return {invalid_entity, nullptr};
}

template <typename ComponentType>
bool neat::ecs::componentlist<ComponentType>::componentlist::allocate(
    size_t new_count) {
    if (new_count < _tags.size()) {
        return false;
    }
    _tags.resize(new_count, false);
    _components.resize(new_count);
    return true;
}

#pragma endregion componentlist implementations

#pragma region ecs implementations

template <typename... RegisteredComponents>
neat::ecs::engine<RegisteredComponents...>::engine()
    : entities(*this), components(*this), systems(*this) {
    static_assert(typing::are_unique_types<RegisteredComponents...>, "Not all registered component types are unique.");
    static_assert(typing::are_all_classes<RegisteredComponents...>, "All registered component types must be a struct or a class.");
}

template <typename... RegisteredComponents>
neat::ecs::engine<RegisteredComponents...>::engine::~engine() {}

template <typename... RegisteredComponents>
template <typename... RequestedComponents>
std::vector<std::tuple<neat::ecs::entity_id, RequestedComponents*...>> neat::ecs::engine<RegisteredComponents...>::iterate() {
    static_assert(typing::is_subset_of<std::tuple<RequestedComponents...>, std::tuple<RegisteredComponents...>>, "At least one of the requested components is not registered.");

    std::vector<entity_id> valid_entities = _get_entities_who_have_components<RequestedComponents...>();

    std::vector<std::tuple<entity_id, RequestedComponents*...>> results;
    results.reserve(valid_entities.size());

    for (entity_id entity : valid_entities) {
        results.push_back(_get_entity_components_with_entity<RequestedComponents...>(entity));
    }
    return results;
}

template <typename... RegisteredComponents>
template <typename... RequestedComponents>
std::vector<std::tuple<RequestedComponents*...>> neat::ecs::engine<RegisteredComponents...>::iterate_components() {
    static_assert(typing::is_subset_of<std::tuple<RequestedComponents...>, std::tuple<RegisteredComponents...>>, "At least one of the requested components is not registered.");

    std::vector<entity_id>                           valid_entities = _get_entities_who_have_components<RequestedComponents...>();
    std::vector<std::tuple<RequestedComponents*...>> results;

    results.reserve(valid_entities.size());

    for (entity_id entity : valid_entities) {
        results.push_back(_get_entity_components_without_entity<RequestedComponents...>(entity));
    }
    return results;
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
neat::ecs::componentlist<RequestedComponent>& neat::ecs::engine<RegisteredComponents...>::_get_components_list() {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    return std::get<typing::get_index<RequestedComponent, RegisteredComponents...>()>(components._components);
}

template <typename... RegisteredComponents>
template <typename... RequestedComponents>
std::vector<neat::ecs::entity_id> neat::ecs::engine<RegisteredComponents...>::_get_entities_who_have_components() {
    static_assert(typing::is_subset_of<std::tuple<RequestedComponents...>, std::tuple<RegisteredComponents...>>, "At least one of the requested component types is not registered.");
    std::vector<entity_id> found_entities;

    for (entity_id entity = 0; entity < entities._entities.size(); entity++) {
        if (!entities._entities[entity])
            continue;

        bool any_not = false;
        ([this, entity, &any_not] {
            if (!this->_get_components_list<RequestedComponents>().has(entity)) {
                any_not = true;
            }
        }(),
         ...);
        if (!any_not) {
            found_entities.push_back(entity);
        }
    }
    return found_entities;
}

template <typename... RegisteredComponents>
template <typename... RequestedComponents>
std::tuple<neat::ecs::entity_id, RequestedComponents*...> neat::ecs::engine<RegisteredComponents...>::_get_entity_components_with_entity(entity_id entity) {
    return {entity, _get_components_list<RequestedComponents>().get(entity)...};
}

template <typename... RegisteredComponents>
template <typename... RequestedComponents>
std::tuple<RequestedComponents*...> neat::ecs::engine<RegisteredComponents...>::_get_entity_components_without_entity(neat::ecs::entity_id entity) {
    return {_get_components_list<RequestedComponents>().get(entity)...};
}

#pragma endregion ecs implementations

#pragma region ecs entities implementations

template <typename... RegisteredComponents>
neat::ecs::engine<RegisteredComponents...>::entities::entities(engine& e)
    : _ecs(e) {};

template <typename... RegisteredComponents>
neat::ecs::entity_id neat::ecs::engine<RegisteredComponents...>::entities::create() {
    if (!_free_entities.empty()) {
        entity_id entity = _free_entities.front();
        _free_entities.pop();
        _entities[entity] = true;
        return entity;
    }

    entity_id entity = _entities.size();
    _entities.push_back(true);
    return entity;
}

template <typename... RegisteredComponents>
bool neat::ecs::engine<RegisteredComponents...>::entities::remove(entity_id entity) {
    if (!exists(entity))
        return false;
    std::apply([entity](auto&&... comp) { ((comp.remove(entity)), ...); },
               _ecs.components._components);
    _entities[entity] = false;
    _free_entities.push(entity);
    return true;
}

template <typename... RegisteredComponents>
bool neat::ecs::engine<RegisteredComponents...>::entities::exists(
    entity_id entity) const {
    if (entity >= _ecs.entities._entities.size()) {
        return false;
    }
    return _ecs.entities._entities[entity];
}

template <typename... RegisteredComponents>
neat::ecs::entity_id neat::ecs::engine<RegisteredComponents...>::entities::last() const {
    entity_id last = 0;
    for (entity_id entity = 0; entity < _entities.size(); entity++) {
        if (_entities[entity]) {
            last = entity;
        }
    }
    return last;
}

template <typename... RegisteredComponents>
std::vector<neat::ecs::entity_id> neat::ecs::engine<RegisteredComponents...>::entities::all() const {
    std::vector<entity_id> found_entities;
    for (entity_id entity = 0; entity < _entities.size(); entity++) {
        if (_entities[entity]) {
            found_entities.push_back(entity);
        }
    }
    return found_entities;
}

#pragma endregion ecs entities implementations

#pragma region ecs components implementations

template <typename... RegisteredComponents>
neat::ecs::engine<RegisteredComponents...>::components::components(engine& e)
    : _ecs(e) {};

template <typename... RegisteredComponents>
template <typename RequestedComponent>
RequestedComponent* neat::ecs::engine<RegisteredComponents...>::components::get(entity_id entity) {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    if (!_ecs.entities.exists(entity))
        return nullptr;
    return _ecs._get_components_list<RequestedComponent>().get(entity);
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
std::vector<RequestedComponent*> neat::ecs::engine<RegisteredComponents...>::components::get(const std::vector<entity_id>& entity_list) {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    std::vector<RequestedComponent*> result;
    result.reserve(entity_list.size());
    for (entity_id entity : entity_list) {
        result.push_back(get<RequestedComponent>(entity));
    }
    return result;
}

template <typename... RegisteredComponents>
template <typename RequestedComponent, typename... Args>
RequestedComponent* neat::ecs::engine<RegisteredComponents...>::components::add(entity_id entity, Args... args) {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    if (!_ecs.entities.exists(entity))
        return nullptr;
    return _ecs._get_components_list<RequestedComponent>().add(entity, args...);
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
bool neat::ecs::engine<RegisteredComponents...>::components::has(entity_id entity) const {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    if (!_ecs.entities.exists(entity))
        return false;
    return _ecs._get_components_list<RequestedComponent>().has(entity);
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
bool neat::ecs::engine<RegisteredComponents...>::components::remove(entity_id entity) {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    if (!_ecs.entities.exists(entity))
        return false;
    return _ecs._get_components_list<RequestedComponent>().remove(entity);
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
std::tuple<neat::ecs::entity_id, RequestedComponent*> neat::ecs::engine<RegisteredComponents...>::components::first() {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    return _ecs._get_components_list<RequestedComponent>().first();
}

template <typename... RegisteredComponents>
template <typename RequestedComponent>
bool neat::ecs::engine<RegisteredComponents...>::components::allocate(size_t new_size) {
    static_assert(typing::is_one_of<RequestedComponent, RegisteredComponents...>, "Requested component type is not registered.");
    return _ecs._get_components_list<RequestedComponent>().allocate(new_size);
}

template <typename... RegisteredComponents>
bool neat::ecs::engine<RegisteredComponents...>::components::allocate_all(size_t new_size) {
    bool allocated = false;
    std::apply([new_size, &allocated](auto&&... comp) { ((allocated = allocated || comp.allocate(new_size)), ...); }, _ecs.components._components);
    return allocated;
}

#pragma endregion ecs component implementations

#pragma region ecs system implementations

template <typename... RegisteredComponents>
neat::ecs::engine<RegisteredComponents...>::systems::systems(engine& e)
    : _ecs(e) {};

template <typename... RegisteredComponents>
template <typename... FuncComponents>
void neat::ecs::engine<RegisteredComponents...>::systems::execute(void (&system)(entity_id, FuncComponents*...)) {
    for (auto data : _ecs.iterate<FuncComponents...>()) {
        std::apply(system, data);
    }
}

template <typename... RegisteredComponents>
template <typename... FuncComponents>
void neat::ecs::engine<RegisteredComponents...>::systems::execute(void (&system)(FuncComponents*...)) {
    for (auto data : _ecs.iterate_components<FuncComponents...>()) {
        std::apply(system, data);
    }
}

template <typename... RegisteredComponents>
template <typename... FuncComponents>
void neat::ecs::engine<RegisteredComponents...>::systems::execute(void (&system)(engine<RegisteredComponents...>&, entity_id, FuncComponents*...)) {
    for (auto data : _ecs.iterate<FuncComponents...>()) {
        std::apply(system, std::tuple_cat(std::tie(this->_ecs), data));
    }
}

template <typename... RegisteredComponents>
template <typename... FuncComponents>
void neat::ecs::engine<RegisteredComponents...>::systems::execute(void (&system)(engine<RegisteredComponents...>&, FuncComponents*...)) {
    for (auto data : _ecs.iterate_components<FuncComponents...>()) {
        std::apply(system, std::tuple_cat(std::tie(this->_ecs), data));
    }
}

#pragma endregion ecs systems implementations

#endif  // NEAT_ECS_HPP_