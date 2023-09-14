#pragma once

#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>

namespace caches
{
template <typename Container>
inline void print_container(const Container &container)
{
    for (const auto &v : container)
    {
        std::cout << v << " ";
    }
    std::cout << "\n";
}
//
// template <typename Container>
// void print_map(const Container &container)
// {
//     for (const auto &v : container)
//     {
//         std::cout << v.first << ": ";
//         print_container(v.second);
//     }
// }


template <typename T, typename KeyT = int>
class LFU_cache_t   // struct is same as class but all its fields are public
{
private:
    size_t sz_;
    std::unordered_map<KeyT, size_t> HIST;
    std::list<std::pair<KeyT, T>> CACHE;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> HASH;

    // size_t frequency_lookup(KeyT key);
    // auto determine_victim(size_t given_frequency) const;
    // bool is_full() const;

public:
    LFU_cache_t(size_t sz) : //constructor
        sz_(sz) {}

    // void print_cache() const;
    // void print_hist()  const;
    // void frequency_increase(KeyT key);

    // template <typename F>
    // bool lookup_update(KeyT key, F slow_get_page);


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
}; // end of LFU_cache_t


template <typename T, typename KeyT = int>
class perfect_cache_t
{
private:
    size_t sz_;
    std::vector<KeyT> REQUEST_LINE;
    std::list<std::pair<KeyT, T>> CACHE;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> HASH;

    std::unordered_map<KeyT, std::list<size_t>> CALL_TABLE;

    // auto determine_victim(KeyT new_key, int i_0);
    // bool is_full() const;

public:
    perfect_cache_t(size_t sz) : //constructor
        sz_(sz) {}

    // void analize_request_line();
    // void set_requests(std::vector<KeyT> request_line);
    // void print_cache() const;

    // template <typename F>
    // bool lookup_update(F slow_get_page, int i);



    void analize_request_line()
    {
        size_t size = REQUEST_LINE.size();

        for (size_t i = 0; i < size; ++i)
        {
            KeyT key = REQUEST_LINE[i];
            if (CALL_TABLE.count(key) == 0)
            {
                std::list<size_t> templist;
                templist.emplace_back(i);
                CALL_TABLE[key] = templist;
            }
            else
            {
                CALL_TABLE[key].emplace_back(i);
            }
        }
        return;
    }

    auto determine_victim(KeyT new_key, int i_0) // if returns cache_.end() then there is no victim
    {
        auto victim = CACHE.end();

        assert(CALL_TABLE.count(new_key) != 0);

        if (CALL_TABLE[new_key].empty()) // no more mention of this key
        {
            return victim;
        }
        size_t farthest_request = CALL_TABLE[new_key].front();


        for (auto i = CACHE.begin(); i != CACHE.end(); ++i)
        {
            KeyT key = i->first;

            assert(CALL_TABLE.count(key) != 0);

            if (CALL_TABLE[key].empty()) // no more mention of this key
            {
                return i;
            }
            size_t next_request = CALL_TABLE[key].front();
            // std::cout << "key: " << key << " next_request: " << next_request << "\n";
            if (next_request >= farthest_request)
            {
                victim = i;
                farthest_request = next_request;
            }
        }
        return victim;
    }


    void set_requests(std::vector<KeyT> request_line)
    {
        REQUEST_LINE = request_line;
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
            CALL_TABLE[key].pop_front();
            return true;
        }

        if (!(is_full())) // there is free space
        {
            CACHE.emplace_front(key, slow_get_page(key));
            HASH.emplace(key, CACHE.begin());

            CALL_TABLE[key].pop_front();
            return false;
        }

        // else (gotta find the victim)
        auto victim = determine_victim(key, i);
        // print_map(CALL_TABLE);
        if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
        {
            // printf("each page in the cache is more valuable than a new page\n");
            CALL_TABLE[key].pop_front();
            return false;
        }

        CACHE.erase(victim);
        CACHE.emplace_front(key, slow_get_page(key));
        HASH.emplace(key, CACHE.begin());

        CALL_TABLE[key].pop_front();
        return false;
    }
}; // end of perfect_cache_t
} // namespace caches
