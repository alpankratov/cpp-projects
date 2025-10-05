#pragma once
#include "../include/config.h"
#include "../include/hasher.h"
#include "../include/block_reader.h"
#include <boost/filesystem.hpp>
#include <functional>
#include <unordered_map>
#include <vector>

namespace bayan {

    /**
     * Core engine – discovers duplicate files according to the spec.
     *
     * Public API:
     *   • ctor takes a fully parsed Config.
     *   • run() performs the scan and returns groups of duplicate paths.
     */
    class DuplicateFinder
    {
    public:
        explicit DuplicateFinder(const Config& cfg);
        /** @return vector of groups; each group is a vector of absolute paths. */
        std::vector<std::vector<boost::filesystem::path>> run();

    private:
        /** Collects candidate files respecting depth, exclusions, masks, min‑size. */
        void collect_candidates();

        /** Helper that decides whether a filename matches any of the glob masks. */
        bool matches_mask(const std::string& filename) const;

        /** Splits a size‑group into duplicate groups using the lazy block‑wise algorithm. */
        void process_size_group(
            const std::vector<boost::filesystem::path>& files,
            std::vector<std::vector<boost::filesystem::path>>& out_groups);

        const Config cfg_;

        /** All files that survive the initial filtering, grouped by file size. */
        std::unordered_map<std::uintmax_t,
            std::vector<boost::filesystem::path>> files_by_size_;
    };

}