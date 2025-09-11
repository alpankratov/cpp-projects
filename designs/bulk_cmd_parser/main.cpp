#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// printing a finished block
static void print_to_console(const vector<string> &cmds) {
    cout << "bulk: ";
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (i) cout << ", ";
        cout << cmds[i];
    }
    cout << '\n';
}

// writing a finished block to a log file
static void write_to_file(const vector<string> &cmds, time_t ts) {
    ostringstream fn;
    fn << "bulk" << ts << ".log";

    ofstream out(fn.str(), ios::out | ios::trunc);
    if (!out) {
        cerr << "Error: could not open file " << fn.str() << endl;
        return;
    }

    out << "bulk: ";
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (i) out << ", ";
        out << cmds[i];
    }
    out << '\n';
}

// emitting a finished block (print + file)
static void emit_block(const vector<string> &cmds, const time_t ts) {
    if (cmds.empty())
        return;

    print_to_console(cmds);
    write_to_file(cmds, ts);
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <block-size>\n";
        return 1;
    }

    long long n_ll = 0;
    try {
        n_ll = stoll(argv[1]);
    } catch (...) {
        cerr << "Invalid block size: " << argv[1] << endl;
        return 1;
    }
    if (n_ll <= 0) {
        cerr << "Block size must be a positive integer.\n";
        return 1;
    }
    const auto N = static_cast<size_t>(n_ll);

    // state variables initialization
    enum class Mode { STATIC, DYNAMIC };
    auto mode = Mode::STATIC;
    int brace_depth = 0; // nesting level of {}
    size_t static_cnt = 0; // commands collected in static mode
    vector<string> cur_cmds; // commands of the current block
    time_t cur_ts = 0; // timestamp of first command
    bool block_started = false; // true after first real command

    string line;
    while (getline(cin, line)) {
        // handling special brace lines
        if (line == "{") {
            if (brace_depth == 0) {
                // entering outermost dynamic block
                // flush any pending static block
                if (mode == Mode::STATIC && !cur_cmds.empty()) {
                    emit_block(cur_cmds, cur_ts);
                }
                // start a fresh dynamic block
                cur_cmds.clear();
                block_started = false;
                mode = Mode::DYNAMIC;
            }
            ++brace_depth;
            continue; // '{' itself is not a command
        }

        if (line == "}") {
            if (brace_depth > 0) {
                --brace_depth;
                if (brace_depth == 0) {
                    // closing outermost dynamic block
                    // emit only if we actually collected commands
                    if (!cur_cmds.empty())
                        emit_block(cur_cmds, cur_ts);
                    // return to static mode
                    cur_cmds.clear();
                    block_started = false;
                    mode = Mode::STATIC;
                }
            }
            // stray '}' (brace_depth==0) is ignored – treat as normal line
            continue; // '}' itself is not a command
        }

        /* ----- regular command line -------------------------------- */
        if (!block_started) {
            cur_ts = std::time(nullptr); // timestamp of first command
            block_started = true;
        }
        cur_cmds.push_back(line);

        if (mode == Mode::STATIC) {
            ++static_cnt;
            if (static_cnt == N) {
                // static block is full
                emit_block(cur_cmds, cur_ts);
                // reset for next static block
                cur_cmds.clear();
                block_started = false;
                static_cnt = 0;
            }
        }
        // in DYNAMIC mode we simply keep collecting until the matching '}'
    }

    // EOF handling
    if (mode == Mode::STATIC && !cur_cmds.empty()) {
        // leftover static block (size < N) – still must be emitted
        emit_block(cur_cmds, cur_ts);
    }

    return 0;
}
