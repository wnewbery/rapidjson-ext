#pragma once
#include <iterator>
#include <type_traits>
#include <string>

namespace rapidjson_ext_detail
{
    // Enable argument dependent lookup
    using std::begin;
    using std::end;

    /**Implementation for is_iterable*/
    template<class T>
    auto is_list_impl(int) -> decltype(
        std::declval<T>().push_back(std::declval<typename T::value_type>()),
        std::true_type{}
    );
    /**Not iterable. ... is less specific than (int)*/
    template<class T>
    std::false_type is_list_impl(...);

    /**Consider an object a list if it has a push_back(value_type)*/
    template <class T> struct is_list : public decltype(is_list_impl<T>(0)) {};
    // Enable argument dependent lookup
    using std::begin;
    using std::end;

    /**Implementation for is_iterable*/
    template<class T>
    auto is_iterable_impl(int) -> decltype(
        begin(std::declval<T&>()) == end(std::declval<T&>()),
        *begin(std::declval<T&>()),
        ++begin(std::declval<T&>()),
        std::true_type{}
        );
    /**Not iterable. ... is less specific than (int)*/
    template<class T>
    std::false_type is_iterable_impl(...);

    /**Consider an object iterable, if the following are valid:
     *
     *   - iterator = begin(T())
     *   - iterator = end(T())
     *   - iterator == iterator
     *   - *iterator
     *   - ++iterator
     *
     * The decltype will only be valid, resolving to std::true_type if the listed operations
     * are valid.
     */
    template <class T> struct is_iterable : public decltype(is_iterable_impl<T>(0)) {};

    using std::to_string;
    template<class T> auto has_to_string_impl(int) -> decltype(to_string(std::declval<T>()));
    template<class T> std::false_type has_to_string_impl(...);
    template<class T> struct has_to_string : public decltype(has_to_string_impl<T>(0)) {};
}
