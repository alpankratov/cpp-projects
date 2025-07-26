#include <algorithm>
#include <iostream>
#include <string>
#include <cstdint>
#include <list>
#include <vector>
#include <tuple>

template<typename T>
concept ip_integer =
        std::same_as<T, int8_t> || std::same_as<T, int16_t> || std::same_as<T, int32_t> || std::same_as<T, int64_t>;

template<typename T>
concept ip_container =
        std::same_as<T, std::vector<typename T::value_type> > || std::same_as<T, std::list<typename T::value_type> >;

template<typename T>
concept ip_string = std::same_as<T, std::string>;


// Perhaps a better alternative to consider - container is everything that has size(), begin() and end()
// however, string fall into this category - so above concept is preferred
// template<typename T>
// concept container = requires(T c) {
//     { c.size() } -> std::convertible_to<std::size_t>;
//     { c.begin() };
//     { c.end()   };
// };

template<ip_string T>
auto print_ip(const T &ip) {
    std::cout << ip << std::endl;
}

template<ip_container T>
void print_ip(T ip) {
    for (size_t i = 0; i < ip.size(); ++i) {
        if (i > 0) {
            std::cout << ".";
        }
        auto it = std::next(ip.begin(), i); // advance begin() by i steps
        std::cout << *it;
        // std::cout << ip.at(i);  // no [] and at() overloads for list so falling back to iterator pointer
    }
    std::cout << std::endl;
}

template<ip_integer T>
auto print_ip(T ip) {
    const auto n_bytes = sizeof(T);
    std::vector<int> out;
    out.reserve(n_bytes);
    for (std::size_t i = 0; i < n_bytes; ++i) {
        std::size_t shift = 8 * (n_bytes - i - 1);
        const int ip_byte = static_cast<int>(ip >> shift & 0xFF);
        out.push_back(ip_byte); // ip >> shift is equivalent to ip* + shift
    }
    print_ip(out); // use template for containers here
}

template<typename T, typename... Ts>
    requires (std::same_as<T, Ts> && ...)  // ensures every `Ts` is the same type as `T`
auto print_ip(const std::tuple<T, Ts...> &ip) {
    // collect all elements into a vector<T>
    std::vector<T> out;
    out.reserve(sizeof...(Ts) + 1);
    std::apply([&](auto &&... args) {
        (out.push_back(args), ...);
    }, ip);
    print_ip(out); // use template for containers here
}


int main() {
    print_ip(int8_t{-1});
    print_ip(int16_t{0}); // 0.0
    print_ip(int32_t{2130706433});
    print_ip(int64_t{8875824491850138409}); // 123.45.67.89.101.112.131.41
    print_ip(std::string{"Hello, World!"}); // Hello, World!
    // print_ip( "Hello, World!" ); // No implementation for const char* - so won't compile
    print_ip(std::vector<int>{100, 200, 300, 400}); // 100.200.300.400
    print_ip(std::list<int>{400, 300, 200, 100}); // 400.300.200.100
    print_ip(std::make_tuple(123, 456, 789, 0)); // 123.456.789.0
    // print_ip(std::make_tuple(123, 456, 789, "0")); // elements are not of the same type - won't compile
}
