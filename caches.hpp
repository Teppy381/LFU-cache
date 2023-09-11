#pragma once

#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>

namespace caches
{

template <typename T, typename KeyT = int>
struct cache_t   // struct is same as class but all its fields are public
{
    size_t sz_;

    std::unordered_map<KeyT, size_t> HIST;

    std::list<std::pair<KeyT, T>> CACHE;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> HASH;




    cache_t(size_t sz) : //constructor
        sz_(sz)
    {}
    // same as
    // {
    //   sz_ = sz;
    // }

    size_t frequency_lookup(KeyT key)
    {
        auto got = HIST.find(key);
        if (got == HIST.end()) // not found
        {
            HIST.emplace(key, 0);
            return 0;
        }
        // found
        return got->second;
    }

    auto determine_victim(size_t given_frequency) const // if returns cache_.end() then there is no victim
    {
        auto victim = CACHE.end();
        size_t min_frequency = given_frequency;

        auto i = CACHE.begin();
        while (i != CACHE.end())
        {
            KeyT key = i->first;
            if (HIST.at(key) <= min_frequency)
            {
                victim = i;
                min_frequency = HIST.at(key);
            }
            ++i;
        }
        return victim;
    }



    void frequency_increase(KeyT key)
    {
        auto got = HIST.find(key);
        if (got == HIST.end()) // not found
        {
            HIST.emplace(key, 1);
            return;
        }
        got->second += 1;
        return;
    }



    bool is_full() const
    {
        return (CACHE.size() == sz_);
    }

    void print_cache() const
    {
        std::cout << "CACHE:\n";
        for (auto& i: CACHE)
        {
            std::cout << i.first << " ";
        }
        std::cout << "\n";
        return;
    }

    void print_hist() const
    {
        std::cout << "HIST:\n";
        for (auto& i: HIST)
        {
            std::cout << '{' << i.first << ", " << i.second << "}\n";
        }
        std::cout << "\n";
        return;
    }


    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page)
    {
        auto page = HASH.find(key);

        if (page != HASH.end()) // found
        {
            return true;
        }

        if (!(is_full())) // there is free space
        {
            CACHE.emplace_front(key, slow_get_page(key));
            HASH.emplace(key, CACHE.begin());
            return false;
        }

        // else (gotta find the victim)

        size_t page_frequency = frequency_lookup(key);

        auto victim = determine_victim(page_frequency);
        if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
        {
            return false;
        }

        CACHE.erase(victim);
        CACHE.emplace_front(key, slow_get_page(key));
        HASH.emplace(key, CACHE.begin());

        return false;
    }
};


template <typename T, typename KeyT = int>
struct perfect_cache_t   // struct is same as class but all its fields are public
{
    size_t sz_;

    std::vector<KeyT> REQUEST_LINE;

    std::list<std::pair<KeyT, T>> CACHE;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> HASH;




    perfect_cache_t(size_t sz) : //constructor
        sz_(sz)
    {}
    // same as
    // {
    //   sz_ = sz;
    // }

    size_t find_next_request(KeyT key, int i_0) const  // if returns REQUEST_LINE.size, then such request is not found
    {
        size_t size = REQUEST_LINE.size();

        for (size_t i = i_0; i < size; ++i)
        {
            if (REQUEST_LINE[i] == key)
            {
                return i;
            }
        }
        return size;
    }

    auto determine_victim(size_t new_page_next_request, int i_0) const // if returns cache_.end() then there is no victim
    {
        auto victim = CACHE.end();
        size_t farthest_request = new_page_next_request;

        auto i = CACHE.begin();
        while (i != CACHE.end())
        {
            KeyT key = i->first;

            size_t next_request = find_next_request(key, i_0);
            // std::cout << "key: " << key << " next_request: " << next_request << "\n";
            if (next_request >= farthest_request)
            {
                if (next_request == REQUEST_LINE.size()) // not found
                {
                    return i;
                }
                victim = i;
                farthest_request = next_request;
            }
            ++i;
        }
        return victim;
    }



    void add_request(KeyT key)
    {
        REQUEST_LINE.emplace_back(key);
    }


    bool is_full() const
    {
        return (CACHE.size() == sz_);
    }

    void print_cache() const
    {
        std::cout << "CACHE:\n";
        for (auto& i: CACHE)
        {
            std::cout << i.first << " ";
        }
        std::cout << "\n";
        return;
    }



    template <typename F>
    bool lookup_update(F slow_get_page, int i)
    {
        KeyT key = REQUEST_LINE[i];

        auto page = HASH.find(key);

        if (page != HASH.end()) // found
        {
            return true;
        }

        if (!(is_full())) // there is free space
        {
            CACHE.emplace_front(key, slow_get_page(key));
            HASH.emplace(key, CACHE.begin());
            return false;
        }

        // else (gotta find the victim)
        size_t new_page_next_request = find_next_request(key, i);
        if (new_page_next_request == REQUEST_LINE.size()) // this page won't be requested again
        {
            return false;
        }
        // std::cout << "key: " << key << " next_request: " << new_page_next_request << "\n";
        auto victim = determine_victim(new_page_next_request, i);
        if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
        {
            // printf("each page in the cache is more valuable than a new page\n");
            return false;
        }

        CACHE.erase(victim);
        CACHE.emplace_front(key, slow_get_page(key));
        HASH.emplace(key, CACHE.begin());

        return false;
    }
};
} // namespace caches
