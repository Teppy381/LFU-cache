#include <cassert>
#include <iostream>

#include "perfect_cache.hpp"
#include "LFU_cache.hpp"
#include "settings_parser.hpp"

// slow get page imitation
int slow_get_page_int(int key) { return key; }

void update_progress_bar(size_t i, size_t n)
{
    int period = n / 100;
    if (i % period == 0)
    {
        std::cout << "\rProgress: " << i * 100/ n << "%";
    }
    return;
}

std::vector<size_t> read_data()
{
    std::cout << "Reading the data\n";

    int n;
    size_t cache_size;

    std::cin >> cache_size >> n;
    if (!(std::cin.good()))
    {
        std::cout << "Unable to read cache size or sequence length\n";
        exit(1);
    }

    std::vector<size_t> temp_line;

    for (int i = 0; i < n; ++i)
    {
        int page_id;
        std::cin >> page_id;
        if (!(std::cin.good()))
        {
            std::cout << "Unable to read page id that goes " << i + 1 << "th in the sequence\n";
            if (temp_line.size() != 0)
            {
                std::cout << "(the id of the last page read is " << temp_line.back() << ")\n";
            }
            exit(1);
        }

        temp_line.emplace_back(page_id);
    }

    temp_line.emplace_back(cache_size);
    temp_line.emplace_back(n);

    return temp_line;
}

int perfect_cache_test(std::vector<size_t> temp_line, const settings_parser::my_settings_t& settings)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::perfect_cache_t<int, size_t> my_cache {cache_size};

    my_cache.set_requests(temp_line);
    my_cache.analyze_request_line();

    std::cout << "\n";

    if (settings.progress_bar)
    {
        std::cout << "\e[?25l"; // hide cursor
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (settings.verbose)
        {
            std::cout << "\nAsking for key " << temp_line[i] << "\n";
        }

        if (my_cache.lookup_update(slow_get_page_int, i))
        {
            hits += 1;
            if (settings.verbose)
            {
                std::cout << "\x1B[92m";
                std::cout << "HIT\n";
            }
        }
        else
        {
            if (settings.verbose)
            {
                std::cout << "\x1B[91m";
                std::cout << "MISS\n";
            }
        }

        if (settings.verbose)
        {
            my_cache.print_cache();
            std::cout << "\x1B[0m";
            my_cache.print_hash();
            // my_cache.print_call_table();
        }
        if (settings.progress_bar)
        {
            update_progress_bar(i, n);
        }
    }
    if (settings.progress_bar)
    {
        std::cout << "\e[2K\r"; // clear line
        std::cout << "\e[?25h"; // reveal cursor
    }

    std::cout << "Perfect cache hits: " << hits << "/" << n << " (" << hits*100/n << "%)" << "\n";
    return hits;
}

int LFU_cache_test(std::vector<size_t> temp_line, const settings_parser::my_settings_t& settings)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::LFU_cache_t<int, size_t> my_cache {cache_size};

    std::cout << "\n";

    if (settings.progress_bar)
    {
        std::cout << "\e[?25l"; // hide cursor
    }

    for (size_t i = 0; i < n; ++i)
    {
        if (settings.verbose)
        {
            std::cout << "\nAsking for key " << temp_line[i] << "\n";
        }

        if (my_cache.lookup_update(temp_line[i], slow_get_page_int))
        {
            hits += 1;
            if (settings.verbose)
            {
                std::cout << "\e[92m";
                std::cout << "HIT\n";
            }
        }
        else
        {
            if (settings.verbose)
            {
                std::cout << "\e[91m";
                std::cout << "MISS\n";
            }
        }
        if (settings.verbose)
        {
            my_cache.print_cache();
            std::cout << "\e[0m";
            my_cache.print_hist_size();
            // my_cache.print_hist();
            // my_cache.print_freq_list();
        }
        if (settings.progress_bar)
        {
            update_progress_bar(i, n);
        }
    }
    if (settings.progress_bar)
    {
        std::cout << "\e[2K\r"; // clear line
        std::cout << "\e[?25h"; // reveal cursor
    }

    std::cout << "\nLFU cache hits: " << hits << "/" << n << " (" << hits*100/n << "%)" << "\n";
    return hits;
}

int main(int argc, const char* argv[])
{
    settings_parser::my_settings_t settings;
    settings.parse_settings(argc, argv);

    std::vector<size_t> temp_line = read_data();

    if (settings.test_LFU)
    {
        LFU_cache_test(temp_line, settings);
    }

    if (settings.test_perfect)
    {
        perfect_cache_test(temp_line, settings);
    }

    return 0;
}
