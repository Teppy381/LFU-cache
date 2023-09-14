#include <cassert>
#include <iostream>

#include "caches.hpp"

// slow get page imitation
int slow_get_page_int(int key) { return key; }


std::vector<int> read_data()
{
    std::cout << "Enter the data:\n";
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

int perfect_cache_test(std::vector<int> temp_line)
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
    my_cache.analize_request_line();

    for (int i = 0; i < n; ++i)
    {
        if (my_cache.lookup_update(slow_get_page_int, i))
        {
            hits += 1;
            // std::cout << "\x1B[92m";
        }
        else
        {
            // std::cout << "\x1B[91m";
        }
        // my_cache.print_cache();
        // std::cout << "\x1B[0m";
    }
    return hits;
}

int LFU_cache_test(std::vector<int> temp_line)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::LFU_cache_t<int> my_cache {cache_size};

    for (int i = 0; i < n; ++i)
    {
        if (my_cache.lookup_update(temp_line[i], slow_get_page_int))
        {
            hits += 1;
            // std::cout << "\x1B[92m";
        }
        else
        {
            // std::cout << "\x1B[91m";
        }
        my_cache.frequency_increase(temp_line[i]);
        // my_cache.print_cache();
        // std::cout << "\x1B[0m";
    }

    // my_cache.print_hist();
    return hits;
}

int main()
{
    std::vector<int> temp_line = read_data();

    std::cout << "\nLFU cache hits: " << LFU_cache_test(temp_line) << "/" << temp_line.back() << "\n";
    std::cout << "\nPerfect cache hits: " << perfect_cache_test(temp_line) << "/" << temp_line.back() << "\n";

    return 0;
}
