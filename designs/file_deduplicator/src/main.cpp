#include "../include/config.h"
#include "../include/duplicate_finder.h"
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        // -----------------------------------------------------------------
        // STEP 1. Parse command‑line arguments
        // -----------------------------------------------------------------
        const bayan::Config cfg = bayan::parse_config(argc, argv);

        // -----------------------------------------------------------------
        // STEP 2. Run the duplicate‑finder engine
        // -----------------------------------------------------------------
        bayan::DuplicateFinder finder(cfg);
        const auto groups = finder.run();

        // -----------------------------------------------------------------
        // STEP 3. Emit results exactly as required:
        //     * one absolute path per line
        //     * blank line between groups
        // -----------------------------------------------------------------
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &g = groups[i];
            for (const auto &p: g)
                std::cout << p.string() << '\n';
            if (i + 1 < groups.size())
                std::cout << '\n'; // separator between groups
        }

        // If no duplicates were found we simply output nothing (as per spec).
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
