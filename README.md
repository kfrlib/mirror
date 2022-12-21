# (micro) reflection library for C++20

Usage example:

```c++
#include <mirror.hpp>
#include <iostream>

template <typename T>
struct color_rgb {
    T red{};
    T green{};
    T blue{};

    // Reflection information in the class itself
    static constexpr inline std::tuple reflection{
        field{ "red", &color_rgb::red },
        field{ "green", &color_rgb::green },
        field{ "blue", &color_rgb::blue },
    };
};

template <typename T>
color_rgb(T, T, T) -> color_rgb<T>;

// Reflection informatino for an external class
template <typename T1, typename T2>
inline constexpr const std::tuple mirror::reflection_of<std::pair<T1, T2>> = {
    field{ "first", &std::pair<T1, T2>::first },
    field{ "second", &std::pair<T1, T2>::second },
};

int main() {
    
    std::cout << std::endl << "reading struct fields:" << std::endl;
    for_each_field(std::make_pair("answer", 42), [](std::string_view name, auto value) {
        std::cout << name << "=" << value << std::endl;
    });
    for_each_field(color_rgb{ 1.0, 0.0, 0.5 }, [](std::string_view name, auto value) {
        std::cout << name << "=" << value << std::endl;
    });

}
```

Another example with field attributes:

```c++

#include <mirror.hpp>
#include <bit>
#include <iostream>
#include <iomanip>

struct attr_mask {
    uint32_t mask;
};

struct attr_json_name {
    std::string_view json_name;
};

struct color_rgba32 {
    uint8_t red{};
    uint8_t green{};
    uint8_t blue{};
    uint8_t alpha{};

    // Reflection information with extra fields
    static constexpr inline std::tuple reflection{
        field{ "red", &color_rgba32::red, attr_mask{ 0x000000FFu }, attr_json_name{ "r" } },
        field{ "green", &color_rgba32::green, attr_mask{ 0x0000FF00u }, attr_json_name{ "g" } },
        field{ "blue", &color_rgba32::blue, attr_mask{ 0x00FF0000u }, attr_json_name{ "b" } },
        field{ "alpha", &color_rgba32::alpha, attr_mask{ 0xFF000000u }, attr_json_name{ "a" } },
    };
};

constexpr auto print_field = [](std::string_view name, auto value) {
    std::cout << name << "=";
    if constexpr (std::is_integral_v<decltype(value)>)
        std::cout.operator<<(value).operator<<(std::endl); // avoid printing uint8_t as character
    else
        std::cout << value << std::endl;
};

int main() {

    color_rgba32 c;
    uint32_t input = 0xFFCC80EE;
    for_each_field(c, [input](std::string_view name, uint8_t& value, const attr_mask& mask) {
        value = (input & mask.mask) >> std::countr_zero(mask.mask);
    });
    for_each_field(c, print_field);

    std::cout << std::endl << "enumerating struct fields:" << std::endl;
    for_each_field<color_rgba32>(
        [](std::string_view name, const field<color_rgba32, uint8_t, attr_mask, attr_json_name>& field) {
            std::cout << name << " (mask: " << std::hex << std::setw(8) << std::setfill('0') << field.mask
                      << std::dec << ", json: " << field.json_name << ")" << std::endl;
        });

}
```

## License

MIT