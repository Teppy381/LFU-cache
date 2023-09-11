#include <cassert>
#include <iostream>

#include "caches.hpp"

// slow get page imitation
int slow_get_page_int(int key) { return key; }

int perfect_cache_test()
{
    int hits = 0;
    int n;
    size_t cache_size;

    std::cin >> cache_size >> n;
    assert(std::cin.good());

    caches::perfect_cache_t<int> my_cache {cache_size};

    for (int i = 0; i < n; ++i)
    {
        int page_id;
        std::cin >> page_id;
        assert(std::cin.good());

        my_cache.add_request(page_id);
    }

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

    std::cout << "Hits: " << hits << "/" << n << std::endl;

    return hits;
}

int LFU_cache_test()
{
    int hits = 0;
    int n;
    size_t cache_size;

    std::cin >> cache_size >> n;
    assert(std::cin.good());

    caches::cache_t<int> my_cache {cache_size};

    for (int i = 0; i < n; ++i)
    {
        int page_id;
        std::cin >> page_id;
        assert(std::cin.good());

        if (my_cache.lookup_update(page_id, slow_get_page_int))
        {
            hits += 1;
            // std::cout << "\x1B[92m";
        }
        else
        {
            // std::cout << "\x1B[91m";
        }
        my_cache.frequency_increase(page_id);
        // my_cache.print_cache();
        // std::cout << "\x1B[0m";

    }

    std::cout << "Hits: " << hits << "/" << n << std::endl;
    // my_cache.print_hist();

    return hits;
}

int main()
{
    std::cout << "\nLFU cache test:\n";
    LFU_cache_test();
    std::cout << "\nPerfect cache test\n";
    perfect_cache_test();
}
