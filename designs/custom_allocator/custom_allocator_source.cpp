#include <iostream>
#include <ostream>
#include <map>
#include "custom_allocators.h"

int get_factorial_of_n(int n) {
    int result = 1;
    while (n > 1) {
        result *= n;
        --n;
    }
    return result;
}

int main() {
    using Alloc = custom_allocators::CustomAllocator<std::pair<const int,int>>;
    const auto alloc = Alloc{10};
    std::map<int,int,std::less<>,Alloc> factorial_dict{alloc};

    for (int i = 0; i < 10; ++i) {
        factorial_dict[i] = get_factorial_of_n(i);
    };
    for (auto &el: factorial_dict) {
        std::cout << el.first << ": " << el.second << '\n';
    }
    return 0;
}
