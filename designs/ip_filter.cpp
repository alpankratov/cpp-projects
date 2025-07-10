#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// IPV4 addresses can have 4 3-digit decimals only with each decimal max to be 255 (4 byte)
// https://en.wikipedia.org/wiki/Dot-decimal_notation
static int IPV4_DOT_DECIMAL_DIGIT_SIZE_LIMIT = 3;
static int IPV4_DOT_DECIMAL_NUMBER_CONSTANT = 4;
static int IPV4_DOT_DECIMAL_MAX = 255;

// ("",  '.') -> [""]
// ("11", '.') -> ["11"]
// ("..", '.') -> ["", "", ""]
// ("11.", '.') -> ["11", ""]
// (".11", '.') -> ["", "11"]
// ("11.22", '.') -> ["11", "22"]
std::vector<std::string> split(const std::string &str, const char d) {
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while (stop != std::string::npos) {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

void print_ipv4_vectors(const std::vector<std::vector<std::string> > &ip_pool) {
    for (const auto &ip: ip_pool) {
        for (const auto &ip_part: ip) {
            if (ip_part != ip[0]) {
                std::cout << ".";
            }
            std::cout << ip_part;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char const *argv[]) {
    try {
        std::vector<std::vector<std::string> > ip_pool;
        std::ifstream all_ip_addresses("../designs/ip_filter.tsv");
        if (!all_ip_addresses.is_open()) {
            std::cerr << "Could not open ip_filter file" << std::endl;
        }

        for (std::string line; std::getline(all_ip_addresses, line);) {
            auto v = split(line, '\t');
            ip_pool.push_back(split(v.at(0), '.'));
        }

        // to ensure that all ip addresses are IPV4 and their dot decimal notation is valid
        std::ranges::for_each(ip_pool, [](auto &ip) {
            assert(ip.size() == IPV4_DOT_DECIMAL_NUMBER_CONSTANT);
            std::ranges::for_each(ip, [](auto &el) {
                assert(el.size() <= IPV4_DOT_DECIMAL_DIGIT_SIZE_LIMIT);
                assert(std::stoi(el) <= IPV4_DOT_DECIMAL_MAX);
            });
        });

        std::ranges::sort(ip_pool, [](const auto &a, const auto &b) {
            std::string result_a;
            std::string result_b;
            for (const auto &a_el: a) {
                result_a += std::string(IPV4_DOT_DECIMAL_DIGIT_SIZE_LIMIT - std::size(a_el), '0') + a_el + ".";
            }
            for (const auto &b_el: b) {
                result_b += std::string(IPV4_DOT_DECIMAL_DIGIT_SIZE_LIMIT - std::size(b_el), '0') + b_el + ".";
            }
            return (result_a > result_b);
        });

        print_ipv4_vectors(ip_pool);

        // 222.173.235.246
        // 222.130.177.64
        // 222.82.198.61
        // ...
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first byte and output
        // ip = filter(1)

        // 1.231.69.33
        // 1.87.203.225
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first and second bytes and output
        // ip = filter(46, 70)

        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76

        // TODO filter by any byte and output
        // ip = filter_any(46)

        // 186.204.34.46
        // 186.46.222.194
        // 185.46.87.231
        // 185.46.86.132
        // 185.46.86.131
        // 185.46.86.131
        // 185.46.86.22
        // 185.46.85.204
        // 185.46.85.78
        // 68.46.218.208
        // 46.251.197.23
        // 46.223.254.56
        // 46.223.254.56
        // 46.182.19.219
        // 46.161.63.66
        // 46.161.61.51
        // 46.161.60.92
        // 46.161.60.35
        // 46.161.58.202
        // 46.161.56.241
        // 46.161.56.203
        // 46.161.56.174
        // 46.161.56.106
        // 46.161.56.106
        // 46.101.163.119
        // 46.101.127.145
        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76
        // 46.55.46.98
        // 46.49.43.85
        // 39.46.86.85
        // 5.189.203.46
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
