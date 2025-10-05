#include "../include/config.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace bayan;

namespace {
    bool to_hash_algo(const std::string &s, HashAlgo &out) {
        std::string low = s;
        std::transform(low.begin(), low.end(), low.begin(),
                       [](const unsigned char c) { return std::tolower(c); });
        if (low == "crc32") {
            out = HashAlgo::CRC32;
            return true;
        }
        if (low == "md5") {
            out = HashAlgo::MD5;
            return true;
        }
        return false;
    }
} // anonymous

Config bayan::parse_config(const int argc, char *argv[]) {
    Config cfg;

    po::options_description desc("bayan – duplicate‑file finder");
    desc.add_options()
            ("help,h", "Show this help message")
            ("scan-dir", po::value<std::vector<std::string> >()->multitoken(), "Directory (or directories) to scan")
            ("exclude-dir", po::value<std::vector<std::string> >()->multitoken(),
             "Directory (or directories) to exclude")
            ("depth", po::value<int>(), "Recursion depth (0 = no sub‑dirs)")
            ("min-size", po::value<std::uintmax_t>(), "Minimal file size in bytes (default 2)")
            ("mask", po::value<std::vector<std::string> >()->multitoken(), "Case‑insensitive glob mask for filenames")
            ("block-size", po::value<std::size_t>(), "Size of a block (bytes) used for hashing")
            ("hash", po::value<std::string>(), "Hash algorithm: crc32 or md5");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        std::exit(0);
    }

    if (vm.count("scan-dir"))
        for (auto &s: vm["scan-dir"].as<std::vector<std::string> >()) cfg.scan_dirs.emplace_back(s);

    if (vm.count("exclude-dir"))
        for (auto &s: vm["exclude-dir"].as<std::vector<std::string> >()) cfg.exclude_dirs.emplace_back(s);

    if (vm.count("depth")) cfg.depth = vm["depth"].as<int>();

    if (vm.count("min-size")) cfg.min_size = vm["min-size"].as<std::uintmax_t>();

    if (vm.count("mask")) cfg.masks = vm["mask"].as<std::vector<std::string> >();

    if (vm.count("block-size")) cfg.block_size = vm["block-size"].as<std::size_t>();

    if (vm.count("hash")) {
        HashAlgo ha;
        if (!to_hash_algo(vm["hash"].as<std::string>(), ha)) {
            std::cerr << "Unsupported hash algorithm. Use crc32 or md5.\n";
            std::exit(1);
        }
        cfg.hash_algo = ha;
    }

    // Basic validation
    if (cfg.scan_dirs.empty()) {
        std::cerr << "At least one --scan-dir must be supplied.\n";
        std::exit(1);
    }
    if (cfg.block_size == 0) {
        std::cerr << "--block-size must be > 0.\n";
        std::exit(1);
    }

    return cfg;
}
