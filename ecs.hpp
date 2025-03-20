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

  template <typename T> __componentlist<T> &get_components() {
    static_assert(__is_any_of<T, Cs...>,
                  "Component type is not a registered type.");
    return std::get<__get_index<T, Cs...>()>(_components);
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
      return _ecs.get_components<T>().get(entity);
    }

    template <typename T, typename... Args>
    T *add(entity_id entity, Args... args) {
      static_assert(__is_any_of<T, Cs...>,
                    "Component type is not a registered type.");
      if (!_ecs.entities.exists(entity))
        return nullptr;
      return _ecs.get_components<T>().add(entity, args...);
    }

    template <typename T>
    bool has(entity_id entity) const {
      return _ecs.get_components<T>().has(entity);
    }

    template <typename T> bool remove(entity_id entity) {
      static_assert(__is_any_of<T, Cs...>,
                    "Component type is not a registered type.");
      if (!_ecs.entities.exists(entity))
        return false;
      return _ecs.get_components<T>().remove(entity);
    }

  } components;
};

}; // namespace ecs

#endif // ECS_HPP_