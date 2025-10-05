#include "../include/duplicate_finder.h"
#include <boost/regex.hpp>
#include <iostream>
#include <queue>

namespace bfs = boost::filesystem;
using namespace bayan;

std::string escape_regex_chars(char c) {
    static const std::string special_chars = ".^$+{}[]()|\\ ";
    if (special_chars.find(c) != std::string::npos) {
        return std::string("\\") + c;
    }
    return std::string(1, c);
}

DuplicateFinder::DuplicateFinder(const Config& cfg) : cfg_(cfg) {}

std::vector<std::vector<bfs::path>>
DuplicateFinder::run()
{
    collect_candidates();

    std::vector<std::vector<bfs::path>> all_groups;

    for (auto& kv : files_by_size_)
    {
        const auto& candidates = kv.second;
        if (candidates.size() < 2) continue;   // nothing to compare
        process_size_group(candidates, all_groups);
    }

    return all_groups;
}

/* --------------------------------------------------------------------- */
/* 1️⃣  Gather candidate files                                            */
void DuplicateFinder::collect_candidates()
{
    // Build a fast lookup for excluded directories
    std::vector<bfs::path> excl_abs;
    for (auto& e : cfg_.exclude_dirs)
        excl_abs.emplace_back(bfs::canonical(e));

    // Helper to decide whether a path lies under any excluded dir
    auto is_excluded = [&](const bfs::path& p) -> bool
    {
        bfs::path cur = bfs::canonical(p);
        for (auto& ex : excl_abs)
            if (std::mismatch(ex.begin(), ex.end(), cur.begin()).first == ex.end())
                return true;
        return false;
    };

    // Convert glob masks to case‑insensitive regexes (if any)
    std::vector<boost::regex> mask_regexes;
    for (auto& m : cfg_.masks)
    {
        std::string r;
        for (char c : m)
        {
            if (c == '*') r += ".*";
            else if (c == '?') r += ".";
            else r += escape_regex_chars(c);
        }
        mask_regexes.emplace_back(r, boost::regex::icase);
    }

    // Recursive walk for each scan directory
    for (auto& root : cfg_.scan_dirs)
    {
        bfs::recursive_directory_iterator it(root,
            bfs::directory_options::none);
        bfs::recursive_directory_iterator end;

        while (it != end)
        {
            const bfs::directory_entry& ent = *it;
            const bfs::path& p = ent.path();

            // Respect depth limit
            if (cfg_.depth >= 0 &&
                std::distance(root.begin(), p.begin()) - std::distance(root.begin(), root.begin()) > cfg_.depth)
            {
                it.disable_recursion_pending();   // stop descending further
            }

            // Skip excluded directories completely
            if (ent.status().type() == bfs::directory_file && is_excluded(p))
            {
                it.disable_recursion_pending();
                ++it;
                continue;
            }

            // Process regular files only
            if (ent.status().type() == bfs::regular_file)
            {
                std::uintmax_t sz = bfs::file_size(p);
                if (sz < cfg_.min_size) { ++it; continue; }

                // Mask check (if masks were supplied)
                if (!mask_regexes.empty())
                {
                    bool ok = false;
                    const std::string fname = p.filename().string();
                    for (auto& rgx : mask_regexes)
                        if (boost::regex_match(fname, rgx)) { ok = true; break; }
                    if (!ok) { ++it; continue; }
                }

                // Store canonical absolute path
                bfs::path abs = bfs::canonical(p);
                files_by_size_[sz].push_back(abs);
            }

            ++it;
        }
    }
}

/* --------------------------------------------------------------------- */
/* 2️⃣  Quick mask helper                                                */
bool DuplicateFinder::matches_mask(const std::string& filename) const
{
    if (cfg_.masks.empty()) return true;
    for (auto& pat : cfg_.masks)
    {
        boost::regex rgx(pat, boost::regex::icase);
        if (boost::regex_match(filename, rgx))
            return true;
    }
    return false;
}

/* --------------------------------------------------------------------- */
/* 3️⃣  Process a single size‑group using the lazy block‑wise algorithm */
void DuplicateFinder::process_size_group(
    const std::vector<bfs::path>& files,
    std::vector<std::vector<bfs::path>>& out_groups)
{
    /* -------------------------------------------------------------
       Conceptual overview
       -------------------
       * All files in the group have the same size → they *might* be equal.
       * We keep a collection of “buckets”.  Each bucket contains files
         that are still indistinguishable after having compared the first N blocks.
       * For every bucket we read the next block (N‑th) **once per file**, hash it,
         and re‑bucket the files by that hash.
       * Buckets that shrink to a single element are discarded – they cannot form
         a duplicate set any longer.
       * When a bucket survives a round and the next block would be past EOF for
         all its members, the bucket represents a full duplicate group.
       ------------------------------------------------------------- */

    using Bucket = std::vector<bfs::path>;

    // Start with a single bucket that holds every candidate of this size.
    std::vector<Bucket> active_buckets{files};

    // Helper that creates a fresh hasher of the requested algorithm.
    auto make_hasher_instance = [&]() { return make_hasher(cfg_.hash_algo); };

    std::size_t block_index = 0;          // which block we are currently comparing
    bool any_bucket_active = true;        // loop guard

    while (any_bucket_active)
    {
        any_bucket_active = false;        // will become true again if we create new work
        std::vector<Bucket> next_round;   // buckets for the following iteration

        for (auto& bucket : active_buckets)
        {
            if (bucket.size() < 2) continue;   // nothing to compare here

            // -----------------------------------------------------------------
            // 1️⃣  Open a BlockReader for every file in the bucket.
            //    We advance each reader to the *current* block_index.
            // -----------------------------------------------------------------
            std::unordered_map<std::string, Bucket> hash_map; // hash → files

            for (const bfs::path& p : bucket)
            {
                BlockReader br(p, cfg_.block_size);

                // Fast‑forward to the block we need.
                // Because we know that we never read a block twice,
                // we can simply call `next()` block_index times.
                for (std::size_t i = 0; i < block_index; ++i)
                {
                    if (!br.has_next()) break;   // safety – should not happen for equal‑size files
                    br.next();                    // discard previous blocks
                }

                // If the file ended before we reach the desired block,
                // it means the file length is a multiple of block_size and we are
                // already at EOF. In that case the block is all zeros.
                std::vector<unsigned char> blk;
                if (br.has_next())
                    blk = br.next();          // real data (maybe padded)
                else
                    blk.assign(cfg_.block_size, 0); // zero‑filled block

                // -------------------------------------------------------------
                // 2️⃣  Compute the hash of the block.
                // -------------------------------------------------------------
                auto hasher = make_hasher_instance();
                hasher->update(blk.data(), blk.size());
                std::string dig = hasher->digest();

                // -------------------------------------------------------------
                // 3️⃣  Put the file into the bucket identified by that hash.
                // -------------------------------------------------------------
                hash_map[dig].push_back(p);
            }

            // -------------------------------------------------------------
            // 4️⃣  Re‑bucket: every hash that still has ≥2 files survives.
            // -------------------------------------------------------------
            for (auto& kv : hash_map)
            {
                if (kv.second.size() >= 2)
                {
                    next_round.push_back(std::move(kv.second));
                    any_bucket_active = true;   // we have work for the next block
                }
                // Singletons are dropped – they cannot be duplicates.
            }
        }

        // -------------------------------------------------------------
        // 5️⃣  Anything that survived *without* needing another block
        //     (i.e. we reached the end of the file for all members)
        //     forms a final duplicate group.
        // -------------------------------------------------------------
        for (auto& bucket : next_round)
        {
            // All files in a bucket have the same size.
            // If the next block would be beyond EOF for *any* member,
            // the whole bucket is a complete duplicate set.
            bool reached_eof = false;
            for (const bfs::path& p : bucket)
            {
                std::uintmax_t fsize = bfs::file_size(p);
                std::uintmax_t blocks_needed =
                    (fsize + cfg_.block_size - 1) / cfg_.block_size; // ceil
                if (block_index + 1 >= blocks_needed)   // next block would be past EOF
                {
                    reached_eof = true;
                    break;
                }
            }
            if (reached_eof)
            {
                out_groups.push_back(bucket);
                // Remove it from further processing – do NOT push back into next_round.
                bucket.clear();
            }
        }

        // Clean empty buckets that were turned into groups.
        std::vector<Bucket> cleaned;
        for (auto& b : next_round)
            if (!b.empty()) cleaned.push_back(std::move(b));
        active_buckets.swap(cleaned);

        ++block_index;   // advance to the next block for the following pass
    }
}
