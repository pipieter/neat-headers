#ifndef ECS_HPP_
#define ECS_HPP_

#include <cstddef>
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

template <typename...> inline constexpr auto __is_any_of = std::true_type{};
template <typename Type, typename Head, typename... Tail>
inline constexpr auto __is_any_of<Type, Head, Tail...> =
    std::bool_constant<(std::is_same_v<Type, Head>) ||
                       __is_any_of<Type, Tail...>>{};

template <typename Type, typename Head, typename... Tail>
constexpr int __get_index() {
  static_assert(__is_any_of<Type, Head, Tail...>,
                "Type not part of possible type.");
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

using entity_id = std::size_t;

template <typename T> class __componentlist {
private:
  std::vector<bool> _tags;
  std::vector<T> _components;

public:
  __componentlist() {
    static_assert(std::is_default_constructible_v<T>,
                  "Component type does not have a default constructor.");
  }

  bool has(entity_id entity) const {
    if (entity >= _tags.size())
      return false;
    return _tags[entity];
  }

  T *get(entity_id entity) {
    if (!has(entity))
      return nullptr;
    return &_components[entity];
  }

  template <typename... Args> T *add(entity_id entity, Args... args) {
    static_assert(std::is_constructible_v<T, Args...>,
                  "Component can't be built from given arguments.");
    if (entity >= _tags.size()) {
      _tags.resize(entity + 1, false);
      _components.resize(entity + 1);
    }

    _tags[entity] = true;
    _components[entity] = T(args...);
    return &_components[entity];
  }

  bool remove(entity_id entity) {
    if (!has(entity))
      return false;
    _tags[entity] = false;
    return true;
  }

  void clear() {
    _tags.clear();
    _components.clear();
  }
};

template <typename... Cs> class ecs {
private:
  std::tuple<__componentlist<Cs>...> _components;
  std::vector<bool> _entities;

  template <typename T> __componentlist<T> &get_components_list() {
    static_assert(__is_any_of<T, Cs...>,
                  "Component type is not a registered type.");
    return std::get<__get_index<T, Cs...>()>(_components);
  }

  template <typename... Ts>
  std::vector<entity_id> get_entities_with_components() {
    std::vector<entity_id> entities;
    for (entity_id entity = 0; entity < _entities.size(); entity++) {
      if (!_entities[entity])
        continue;

      bool any_not = false;
      (
          [this, entity, &any_not]<Ts> {
            if (!this->get_components_list<Ts>().has(entity)) {
              any_not = true;
            }
          }(),
          ...);
      if (!any_not) {
        entities.push_back(entity);
      }
    }
    return entities;
  }

  template <typename T> T *get_unchecked_component(entity_id entity) {
    return get_components_list<T>().get(entity);
  }

  template <typename... Ts>
  std::tuple<entity_id, Ts *...>
  get_entity_unchecked_components(entity_id entity) {
    std::tuple<entity_id, Ts *...> result{
        entity, get_unchecked_component<Ts>(entity)...};
    return result;
  }

public:
  ecs() : entities(*this), components(*this) {
    static_assert(__are_unique_types<Cs...>,
                  "ECS expects all types to be unique.");
    static_assert(__are_all_classes<Cs...>,
                  "ECS only allows structs/classes for components.");
  }

  class __entities final {
  private:
    friend class ecs;
    ecs &_ecs;
    explicit __entities(ecs &e) : _ecs(e){};

  public:
    entity_id create() {
      for (entity_id entity = 0; entity < _ecs._entities.size(); entity++) {
        if (!_ecs._entities[entity]) {
          _ecs._entities[entity] = true;
          return entity;
        }
      }
      entity_id entity = _ecs._entities.size();
      _ecs._entities.push_back(true);
      return entity;
    }

    bool remove(entity_id entity) {
      if (!exists(entity))
        return false;
      std::apply([entity](auto &&...comp) { ((comp.remove(entity)), ...); },
                 _ecs._components);
    }

    bool exists(entity_id entity) const {
      if (entity >= _ecs._entities.size()) {
        return false;
      }
      return _ecs._entities[entity];
    }
  } entities;

  class __components final {
  private:
    friend class ecs;
    ecs &_ecs;
    explicit __components(ecs &e) : _ecs(e){};

  public:
    template <typename T> T *get(entity_id entity) {
      static_assert(__is_any_of<T, Cs...>,
                    "Component type is not a registered type.");
      if (!_ecs.entities.exists(entity))
        return nullptr;
      return _ecs.get_components_list<T>().get(entity);
    }

    template <typename T, typename... Args>
    T *add(entity_id entity, Args... args) {
      static_assert(__is_any_of<T, Cs...>,
                    "Component type is not a registered type.");
      if (!_ecs.entities.exists(entity))
        return nullptr;
      return _ecs.get_components_list<T>().add(entity, args...);
    }

    template <typename T> bool has(entity_id entity) const {
      return _ecs.get_components_list<T>().has(entity);
    }

    template <typename T> bool remove(entity_id entity) {
      static_assert(__is_any_of<T, Cs...>,
                    "Component type is not a registered type.");
      if (!_ecs.entities.exists(entity))
        return false;
      return _ecs.get_components_list<T>().remove(entity);
    }

  } components;

  template <typename... Ts>
  std::vector<std::tuple<entity_id, Ts *...>> iterate() {
    std::vector<entity_id> entities = get_entities_with_components<Ts...>();
    std::vector<std::tuple<entity_id, Ts *...>> results;

    for (entity_id entity : entities) {
      results.push_back(get_entity_unchecked_components<Ts...>(entity));
    }
    return results;
  }
};
}; // namespace ecs

#endif // ECS_HPP_