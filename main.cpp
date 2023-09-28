#include <cassert>
#include <iostream>

#include "caches.hpp"

// slow get page imitation
int slow_get_page_int(int key) { return key; }

void update_progress_bar(int i, int n)
{
    int period = n / 100;
    if (i % period == 0)
    {
        std::cout << "\rProgress: " << i * 100/ n << "%";
    }
    return;
}

std::vector<int> read_data()
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

    std::vector<int> temp_line;

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

int perfect_cache_test(std::vector<int> temp_line, bool verbal, bool interactive)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::perfect_cache_t<int> my_cache {cache_size};

    my_cache.set_requests(temp_line);
    my_cache.analyze_request_line();

    std::cout << "\n";

    if (interactive)
    {
        std::cout << "\e[?25l"; // hide cursor
    }
    for (int i = 0; i < n; ++i)
    {
        if (verbal)
        {
            std::cout << "\nAsking for key " << temp_line[i] << "\n";
        }

        if (my_cache.lookup_update(slow_get_page_int, i))
        {
            hits += 1;
            if (verbal)
            {
                std::cout << "\x1B[92m";
                std::cout << "HIT\n";
            }
        }
        else
        {
            if (verbal)
            {
                std::cout << "\x1B[91m";
                std::cout << "MISS\n";
            }
        }

        if (verbal)
        {
            my_cache.print_cache();
            std::cout << "\x1B[0m";
            my_cache.print_hash();
            // my_cache.print_call_table();
        }
        if (interactive)
        {
            update_progress_bar(i, n);
        }
    }
    if (interactive)
    {
        std::cout << "\e[2K\r"; // clear line
        std::cout << "\e[?25h"; // reveal cursor
    }

    std::cout << "Perfect cache hits: " << hits << "/" << n << " (" << hits*100/n << "%)" << "\n";
    return hits;
}

int LFU_cache_test(std::vector<int> temp_line, bool verbal, bool interactive)
{
    int hits = 0;

    size_t n = temp_line.back();
    temp_line.pop_back();
    size_t cache_size = temp_line.back();
    temp_line.pop_back();

    caches::LFU_cache_t<int> my_cache {cache_size};

    for (int i = 0; i < n; ++i)
    {
        if (verbal)
        {
            std::cout << "\nAsking for key " << temp_line[i] << "\n";
        }

        if (my_cache.lookup_update(temp_line[i], slow_get_page_int))
        {
            hits += 1;
            if (verbal)
            {
                std::cout << "\e[92m";
                std::cout << "HIT\n";
            }
        }
        else
        {
            if (verbal)
            {
                std::cout << "\e[91m";
                std::cout << "MISS\n";
            }
        }
        if (verbal)
        {
            my_cache.print_cache();
            std::cout << "\e[0m";
            my_cache.print_hist();
        }
    }
    std::cout << "\nLFU cache hits: " << hits << "/" << n << " (" << hits*100/n << "%)" << "\n";
    return hits;
}

int main(int argc, char* argv[])
{
    bool verbal = false;
    bool interactive = false;

    for (int i = 1; i < argc; ++i)
    {
        if ((argv[i][0] == '-') && (argv[i][1] == 'V') && (argv[i][2] == '\0'))
        // "-V" flag (verbal, with dump of structures)
        {
            verbal = true;
        }
        else if ((argv[i][0] == '-') && (argv[i][1] == 'I') && (argv[i][2] == '\0'))
        // "-I" flag (interactive, with progress bar, seems to work only in windows cmd)
        {
            interactive = true;
        }
        else
        {
            std::cout << "Unknown flag \"" << argv[i] << "\"\n(use -V or -I)\n";
            return 0;
        }
    }

    std::vector<int> temp_line = read_data();



    LFU_cache_test(temp_line, verbal, interactive);

    perfect_cache_test(temp_line, verbal, interactive);

    return 0;
}
