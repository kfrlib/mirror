/*
 Mirror - (micro) reflection library for C++20
 Copyright 2022 Dan L Cazarín, Fractalium Ltd
 MIT license
 */
#pragma once

#include <tuple>
#include <string_view>
#include <type_traits>

namespace mirror {

// Structure for field details
template <typename Class, typename FieldType, typename... Attributes>
struct field : public Attributes... {
    std::string_view name;
    FieldType Class::*pointerToField;

    constexpr field(std::string_view name, FieldType Class::*pointerToField,
                    Attributes... attr) noexcept((std::is_nothrow_move_constructible_v<Attributes> && ...))
        : Attributes(std::move(attr))..., name(name), pointerToField(pointerToField) {}
};

// Type deduction rules. This is required because
// compiler cannot deduce string_view from string literal
template <std::size_t N, typename Class, typename FieldType, typename... Attributes>
field(const char (&)[N], FieldType Class::*, Attributes...) -> field<Class, FieldType, Attributes...>;

// Template variable that points to a tuple of field that
// describe the fields of type T
// All these specializations are needed to be able to omit decay_t or remove_cvref_t
// from calls to reflection_of<decltype(val)>
template <typename T>
inline constexpr const auto& reflection_of = T::reflection;
template <typename T>
inline constexpr const auto& reflection_of<const T> = reflection_of<T>;
template <typename T>
inline constexpr const auto& reflection_of<volatile T> = reflection_of<T>;
template <typename T>
inline constexpr const auto& reflection_of<T&> = reflection_of<T>;
template <typename T>
inline constexpr const auto& reflection_of<T&&> = reflection_of<T>;

// Type of tuple. Calling tuple_size_v on this will return the number of fields
template <typename T>
using reflection_type = std::remove_cvref_t<decltype(reflection_of<T>)>;

template <typename T>
constexpr inline std::size_t reflection_num_fields =
    std::tuple_size_v<std::remove_cvref_t<decltype(reflection_of<T>)>>;

namespace internal {
template <std::size_t Index, typename T, typename Class, typename FieldType, typename... Attributes,
          typename Fn>
inline constexpr void call_fn(T&& val, const field<Class, FieldType, Attributes...>& fld, Fn&& fn) {
    if constexpr (std::is_invocable_v<Fn, std::string_view, FieldType&,
                                      field<Class, FieldType, Attributes...>>)
        fn(fld.name, val.*(fld.pointerToField), fld);
    else
        fn(fld.name, val.*(fld.pointerToField));
}
template <std::size_t Index, typename Class, typename FieldType, typename... Attributes, typename Fn>
inline constexpr void call_fn(const field<Class, FieldType, Attributes...>& fld, Fn&& fn) {
    if constexpr (std::is_invocable_v<Fn, std::string_view, field<Class, FieldType, Attributes...>>)
        fn(fld.name, fld);
    else
        fn(fld.name);
}
template <typename T, typename Class, typename... FieldType, typename... Attributes, std::size_t... Indices,
          typename Fn>
inline constexpr void for_each_field(T&& val,
                                     const std::tuple<field<Class, FieldType, Attributes...>...>& fields,
                                     std::integer_sequence<std::size_t, Indices...>, Fn&& fn) {
    (call_fn<Indices>(val, std::get<Indices>(fields), fn), ...);
}
template <typename T, typename Class, typename... FieldType, typename... Attributes, std::size_t... Indices,
          typename Fn>
inline constexpr void for_each_field(const std::tuple<field<Class, FieldType, Attributes...>...>& fields,
                                     std::integer_sequence<std::size_t, Indices...>, Fn&& fn) {
    (call_fn<Indices>(std::get<Indices>(fields), fn), ...);
}
} // namespace internal

// Enumerate all fields in T and call fn for each field.
// fn must be lambda or a functor with template operator()
template <typename T, typename Fn>
inline constexpr void for_each_field(T&& val, Fn&& fn) {
    internal::for_each_field(std::forward<T>(val), reflection_of<T>,
                             std::make_index_sequence<reflection_num_fields<T>>{}, std::forward<Fn>(fn));
}
template <typename T, typename Fn>
inline constexpr void for_each_field(Fn&& fn) {
    internal::for_each_field<T>(reflection_of<T>, std::make_index_sequence<reflection_num_fields<T>>{},
                                std::forward<Fn>(fn));
}

} // namespace mirror
