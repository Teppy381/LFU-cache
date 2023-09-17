#include <cassert>
#include <iostream>

#include "caches.hpp"

// slow get page imitation
int slow_get_page_int(int key) { return key; }

void update_progress_bar(int i, int n)
{
    float percentage = i * 100/ n;
    std::cout << "\rProgress: " << i << "/" << n << " (" << percentage << "%)";
}

std::vector<int> read_data()
{
    // std::cout << "Enter the data:\n";
    int n;
    size_t cache_size;

    std::cin >> cache_size >> n;
    assert(std::cin.good());

    std::vector<int> temp_line;

    for (int i = 0; i < n; ++i)
    {
        int page_id;
        std::cin >> page_id;
        assert(std::cin.good());

        temp_line.emplace_back(page_id);
    }

    temp_line.emplace_back(cache_size);
    temp_line.emplace_back(n);

    return temp_line;
}

int perfect_cache_test(std::vector<int> temp_line, bool human_mode)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    // std::cout << "n = " << n << ", cache_size = " << cache_size << "\n";
    // caches::print_container(temp_line);

    caches::perfect_cache_t<int> my_cache {cache_size};

    my_cache.set_requests(temp_line);
    my_cache.analyze_request_line();

    if (human_mode)
    {
        std::cout << "\e[?25l"; // hide cursor
        std::cout << "\nPerfect cache:\n";
    }
    for (int i = 0; i < n; ++i)
    {
        if (my_cache.lookup_update(slow_get_page_int, i))
        {
            hits += 1;
            if (human_mode)
            {
                // std::cout << "\x1B[92m";
            }
        }
        else
        {
            if (human_mode)
            {
                // std::cout << "\x1B[91m";
            }
        }

        if (human_mode)
        {
            // my_cache.print_cache();
            // std::cout << "\x1B[0m";
            update_progress_bar(i, n);
        }
    }
    if (human_mode)
    {
        std::cout << "\e[2K\r"; // clear line
        std::cout << "hits: " << hits << "/" << n << "\n";
        std::cout << "\e[?25h"; // reveal cursor
    }
    else
    {
        std::cout << "\nPerfect cache hits: " << hits << "/" << n << "\n";
    }
    return hits;
}

int LFU_cache_test(std::vector<int> temp_line, bool human_mode)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::LFU_cache_t<int> my_cache {cache_size};

    if (human_mode)
    {
        std::cout << "\e[?25l"; // hide cursor
        std::cout << "\nLFU cache:\n";
    }
    for (int i = 0; i < n; ++i)
    {
        if (my_cache.lookup_update(temp_line[i], slow_get_page_int))
        {
            hits += 1;
            if (human_mode)
            {
                // std::cout << "\x1B[92m";
            }
        }
        else
        {
            if (human_mode)
            {
                // std::cout << "\x1B[91m";
            }
        }
        my_cache.frequency_increase(temp_line[i]);

        if (human_mode)
        {
            // my_cache.print_cache();
            // std::cout << "\x1B[0m";
            update_progress_bar(i, n);
        }
    }
    if (human_mode)
    {
        std::cout << "\e[2K\r"; // clear line
        std::cout << "hits: " << hits << "/" << n << "\n";
        std::cout << "\e[?25h"; // reveal cursor
        // my_cache.print_hist();
    }
    else
    {
        std::cout << "\nLFU cache hits: " << hits << "/" << n << "\n";
    }
    return hits;
}

int main(int argc, char* argv[])
{
    bool human_mode = false;
    if (argc > 1)
    {
        if ((argv[1][0] == '-') && (argv[1][1] == 'H')) // "-H" flag
        {
            human_mode = true;
        }
        else
        {
            std::cout << "Unknown flag (use -H for \"human mode\")\n";
            return 0;
        }
    }

    std::vector<int> temp_line = read_data();

    LFU_cache_test(temp_line, human_mode);

    perfect_cache_test(temp_line, human_mode);

    return 0;
}
