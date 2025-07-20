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
    std::cout << "\n\n\nCreating factorial map without allocator\n";
    std::map<int,int> factorial_dict{};
    for (int i = 0; i < 10; ++i) {
        factorial_dict[i] = get_factorial_of_n(i);
    };
    for (auto &el: factorial_dict) {
        std::cout << el.first << ": " << el.second << '\n';
    }
    std::cout << "Finish factorial map without allocator\n\n\n";

    std::cout << "\n\n\nCreating factorial map with Custom Allocator\n";
    using customAllocator = custom_allocators::CustomAllocator<std::pair<const int,int>>;
    const auto alloc_map = customAllocator{7};
    std::map<int,int,std::less<>,customAllocator> factorial_dict_allocator{alloc_map};
    for (int i = 0; i < 10; ++i) {
        factorial_dict_allocator[i] = get_factorial_of_n(i);
    };
    for (auto &el: factorial_dict_allocator) {
        std::cout << el.first << ": " << el.second << '\n';
    }
    std::cout << "By element deallocation check\n";
    factorial_dict_allocator.erase(2);  // should be deallocated from pool
    factorial_dict_allocator.erase(9); // should be deallocated from malloc
    std::cout << "By element deallocation check complete\n";
    std::cout << "Finish factorial map with Custom Allocator\n\n\n";


    return 0;
}
