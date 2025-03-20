#ifndef ECS_HPP_
#define ECS_HPP_

#include <cstdint>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ecs {

template <typename...>
inline constexpr auto __are_unique_types = std::true_type{};
template <typename Type, typename... Rest>
inline constexpr auto __are_unique_types<Type, Rest...> =
    std::bool_constant<(!std::is_same_v<Type, Rest> && ...) &&
                       __are_unique_types<Rest...>>{};

template <typename T, typename... Ts>
constexpr bool __is_any_of = (std::is_same<T, Ts>{} || ...);

template <typename Type, typename Head, typename... Tail>
constexpr int __get_index() {
  static_assert(__is_any_of<Type, Head, Tail...>,
                "Type not part of accepted types.");
  if constexpr (std::is_same<Type, Head>::value)
    return 0;
  else
    return 1 + __get_index<Type, Tail...>();
}

template <typename...>
inline constexpr auto __are_all_classes = std::true_type{};
template <typename T, typename... Ts>
inline constexpr auto __are_all_classes<T, Ts...> =
    std::bool_constant<(std::is_class_v<T>)&&__are_all_classes<Ts...>>{};

template <typename Subset, typename Set> constexpr bool __is_subset_of = false;
template <typename... Ts, typename... Us>
constexpr bool __is_subset_of<std::tuple<Ts...>, std::tuple<Us...>> =
    (__is_any_of<Ts, Us...> && ...);

using entity_id = std::size_t;
const entity_id invalid_entity = SIZE_MAX;

template <typename ComponentType> class __componentlist {
private:
  std::vector<bool> _tags;
  std::vector<ComponentType> _components;

public:
  __componentlist() {
    static_assert(std::is_class_v<ComponentType>,
                  "Component type is not a struct or class.");
    static_assert(std::is_default_constructible_v<ComponentType>,
                  "Component type does not have a default constructor.");
  }

  bool has(entity_id entity) const {
    if (entity >= _tags.size())
      return false;
    return _tags[entity];
  }

  ComponentType *get(entity_id entity) {
    if (!has(entity))
      return nullptr;
    return &_components[entity];
  }

  template <typename... Args>
  ComponentType *add(entity_id entity, Args... args) {
    static_assert(std::is_constructible_v<ComponentType, Args...>,
                  "Component type can't be built from given arguments.");
    if (entity >= _tags.size()) {
      _tags.resize(entity + 1, false);
      _components.resize(entity + 1);
    }

    _tags[entity] = true;
    _components[entity] = ComponentType(args...);
    return &_components[entity];
  }

  bool remove(entity_id entity) {
    if (!has(entity))
      return false;
    _tags[entity] = false;
    return true;
  }

  std::tuple<entity_id, ComponentType *> first() {
    for (entity_id entity = 0; entity < _tags.size(); entity++) {
      if (_tags[entity]) {
        return {entity, &_components[entity]};
      }
    }
    return {invalid_entity, nullptr};
  }
};

template <typename... RegisteredComponents> class ecs {
private:
  template <typename RequestedComponent>
  __componentlist<RequestedComponent> &_get_components_list() {
    static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                  "Requested component type is not registered.");
    return std::get<__get_index<RequestedComponent, RegisteredComponents...>()>(
        components._components);
  }

  template <typename... RequestedComponents>
  std::vector<entity_id> _get_entities_who_have_components() {
    static_assert(
        __is_subset_of<std::tuple<RequestedComponents...>,
                       std::tuple<RegisteredComponents...>>,
        "At least one of the requested component types is not registered.");
    std::vector<entity_id> found_entities;
    for (entity_id entity = 0; entity < entities._entities.size(); entity++) {
      if (!entities._entities[entity])
        continue;

      bool any_not = false;
      (
          [this, entity, &any_not]<RequestedComponents> {
            if (!this->_get_components_list<RequestedComponents>().has(
                    entity)) {
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

  template <typename... Ts>
  std::tuple<entity_id, Ts *...>
  _get_entity_components_with_entity(entity_id entity) {
    std::tuple<entity_id, Ts *...> result{
        entity, _get_components_list<Ts>().get(entity)...};
    return result;
  }

  template <typename... Ts>
  std::tuple<Ts *...> _get_entity_components_without_entity(entity_id entity) {
    std::tuple<Ts *...> result{_get_components_list<Ts>().get(entity)...};
    return result;
  }

public:
  ecs() : entities(*this), components(*this), systems(*this) {
    static_assert(__are_unique_types<RegisteredComponents...>,
                  "Not all registered component types are unique.");
    static_assert(
        __are_all_classes<RegisteredComponents...>,
        "All registered component types must be a struct or a class.");
  }

  class __entities final {
  private:
    friend class ecs;
    ecs &_ecs;
    std::vector<bool> _entities;
    explicit __entities(ecs &e) : _ecs(e){};

  public:
    entity_id create() {
      for (entity_id entity = 0; entity < _entities.size(); entity++) {
        if (!_entities[entity]) {
          _entities[entity] = true;
          return entity;
        }
      }
      entity_id entity = _entities.size();
      _entities.push_back(true);
      return entity;
    }

    bool remove(entity_id entity) {
      if (!exists(entity))
        return false;
      std::apply([entity](auto &&...comp) { ((comp.remove(entity)), ...); },
                 _ecs.components._components);
    }

    bool exists(entity_id entity) const {
      if (entity >= _ecs.entities._entities.size()) {
        return false;
      }
      return _ecs.entities._entities[entity];
    }
  } entities;

  class __components final {
  private:
    friend class ecs;
    ecs &_ecs;
    std::tuple<__componentlist<RegisteredComponents>...> _components;

    explicit __components(ecs &e) : _ecs(e){};

  public:
    template <typename RequestedComponent>
    RequestedComponent *get(entity_id entity) {
      static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                    "Requested component type is not registered.");
      if (!_ecs.entities.exists(entity))
        return nullptr;
      return _ecs._get_components_list<RequestedComponent>().get(entity);
    }

    template <typename RequestedComponent, typename... Args>
    RequestedComponent *add(entity_id entity, Args... args) {
      static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                    "Requested component type is not registered.");
      if (!_ecs.entities.exists(entity))
        return nullptr;
      return _ecs._get_components_list<RequestedComponent>().add(entity,
                                                                 args...);
    }

    template <typename RequestedComponent> bool has(entity_id entity) const {
      static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                    "Requested component type is not registered.");
      return _ecs._get_components_list<RequestedComponent>().has(entity);
    }

    template <typename RequestedComponent> bool remove(entity_id entity) {
      static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                    "Requested component type is not registered.");
      if (!_ecs.entities.exists(entity))
        return false;
      return _ecs._get_components_list<RequestedComponent>().remove(entity);
    }

    template <typename RequestedComponent>
    std::tuple<entity_id, RequestedComponent *> first() {
      static_assert(__is_any_of<RequestedComponent, RegisteredComponents...>,
                    "Requested component type is not registered.");
      return _ecs._get_components_list<RequestedComponent>().first();
    }

  } components;

  struct __systems {
  private:
    friend class ecs;
    ecs &_ecs;
    explicit __systems(ecs &e) : _ecs(e){};

  public:
    template <typename... FuncComponents>
    void execute(void (&system)(entity_id, FuncComponents *...)) {
      for (auto data : _ecs.iterate<FuncComponents...>()) {
        std::apply(system, data);
      }
    }

    template <typename... FuncComponents>
    void execute(void (&system)(FuncComponents *...)) {
      for (auto data : _ecs.iterate_components<FuncComponents...>()) {
        std::apply(system, data);
      }
    }
  } systems;

  template <typename... RequestedComponents>
  std::vector<std::tuple<entity_id, RequestedComponents *...>> iterate() {
    static_assert(
        __is_subset_of<std::tuple<RequestedComponents...>,
                       std::tuple<RegisteredComponents...>>,
        "At least one of the requested components is not registered.");
    std::vector<entity_id> valid_entities =
        _get_entities_who_have_components<RequestedComponents...>();
    std::vector<std::tuple<entity_id, RequestedComponents *...>> results;
    results.reserve(valid_entities.size());

    for (entity_id entity : valid_entities) {
      results.push_back(
          _get_entity_components_with_entity<RequestedComponents...>(entity));
    }
    return results;
  }

  template <typename... RequestedComponents>
  std::vector<std::tuple<RequestedComponents *...>> iterate_components() {
    static_assert(
        __is_subset_of<std::tuple<RequestedComponents...>,
                       std::tuple<RegisteredComponents...>>,
        "At least one of the requested components is not registered.");
    std::vector<entity_id> valid_entities =
        _get_entities_who_have_components<RequestedComponents...>();
    std::vector<std::tuple<RequestedComponents *...>> results;
    results.reserve(valid_entities.size());

    for (entity_id entity : valid_entities) {
      results.push_back(
          _get_entity_components_without_entity<RequestedComponents...>(
              entity));
    }
    return results;
  }
};
}; // namespace ecs

#endif // ECS_HPP_