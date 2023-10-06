#pragma once

#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>

namespace caches
{

template <typename T, typename KeyT = int>
class LFU_cache_t
{
private:
    size_t size_;
    size_t hist_max_size_;
    const float hist_clean_mult_ = 0.4;

    std::unordered_map<KeyT, size_t> HIST;
    std::unordered_map<size_t, std::list<KeyT>> FREQUENCY_LIST;

    using FreqIt = typename std::list<KeyT>::iterator;
    std::list<std::tuple<KeyT, T, FreqIt>> CACHE;

    using CacheIt = typename std::list<std::tuple<KeyT, T, FreqIt>>::iterator;
    std::unordered_map<KeyT, CacheIt> HASH;


public:
    LFU_cache_t(size_t sz) : //constructor
        size_(sz) { hist_max_size_ = sz * 5; }


    bool is_full() const
    {
        return (CACHE.size() == size_);
    }

    void print_cache() const
    {
        std::cout << "CACHE:\n";
        for (auto& i: CACHE)
        {
            std::cout << std::get<0>(i) << " ";
        }
        std::cout << "\n";
        return;
    }

    void print_hist_size() const
    {
        std::cout << "HIST size = " << HIST.size() << "\n";
    }

    void print_hist() const
    {
        std::cout << "HIST:\n";
        for (auto& i: HIST)
        {
            std::cout << '{' << i.first << ", " << i.second << "}\n";
        }
        return;
    }

    void print_freq_list() const
    {
        std::cout << "FREQUENCY_LIST:\n";
        for (const auto &v : FREQUENCY_LIST)
        {
            std::cout << v.first << ": ";
            for (const auto &k : v.second)
            {
                std::cout << k << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        return;
    }

private:
    CacheIt determine_victim(size_t given_frequency) // if returns cache_.end() then there is no victim
    {
        if (given_frequency == 0)
        {
            return CACHE.end();
        }

        size_t i = 1;
        while (FREQUENCY_LIST.count(i) == 0 || FREQUENCY_LIST.at(i).empty())
        {
            if (i >= given_frequency)
            {
                return CACHE.end();
            }
            ++i;
        }

        if (i > given_frequency)
        {
            return CACHE.end();
        }

        KeyT key = FREQUENCY_LIST.at(i).front();

        assert(HASH.count(key) != 0);
        return HASH.at(key);
    }

    void clean_HIST()
    {
        using HistIt = typename std::unordered_map<KeyT, size_t>::iterator;
        std::vector<HistIt> to_erase_list;

        size_t target_size = hist_max_size_ * hist_clean_mult_;

        assert(target_size >= size_);
        while (HIST.size() > target_size)
        {
            for (auto i = HIST.begin(); i != HIST.end(); ++i)
            {
                if ((i->second <= 1) && (HASH.count(i->first) == 0))
                {
                    to_erase_list.push_back(i);
                }
                else
                {
                    i->second -= 1;
                }
            }
            while (to_erase_list.size() > 0)
            {
                HIST.erase(to_erase_list.back());
                to_erase_list.pop_back();
            }
        }
    }

    void increase_freq(KeyT key)
    {
        HIST[key] += 1;
        size_t new_freq = HIST[key];

        if (FREQUENCY_LIST.count(new_freq) == 0)
        {
            std::list<KeyT> temp_list;
            FREQUENCY_LIST.emplace(new_freq, temp_list);
        }
        FREQUENCY_LIST[new_freq].emplace_front(key);
        return;
    }

    void erase_from_cache(CacheIt victim)
    {
        KeyT victim_key = std::get<0>(*victim);
        size_t victim_freq = HIST[victim_key];
        auto iterator = std::get<2>(*victim);
        FREQUENCY_LIST[victim_freq].erase(iterator);
        HASH.erase(victim_key);
        CACHE.erase(victim);

        return;
    }

public:
    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page)
    {
        if (HIST.size() > hist_max_size_)
        {
            clean_HIST();
        }

        if (HIST.count(key) == 0) // add to HIST
        {
            HIST.emplace(key, 0);
        }

        if (HASH.count(key) != 0) // found in CACHE
        {
            auto iterator = std::get<2>(*HASH[key]);
            FREQUENCY_LIST[HIST[key]].erase(iterator);

            increase_freq(key);
            size_t new_freq = HIST[key];
            std::get<2>(*HASH[key]) = FREQUENCY_LIST[new_freq].begin();
            return true;
        }

        if (is_full()) // gotta find the victim
        {
            CacheIt victim = determine_victim(HIST[key]);
            if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
            {
                HIST[key] += 1; // increase frequency
                return false;
            }
            erase_from_cache(victim); // erase victim from CACHE and from FREQUENCY_LIST
        }
        // now there is free space

        increase_freq(key); // increase frequency and add new page to FREQUENCY_LIST[new_freq] and to CACHE
        size_t new_freq = HIST[key];
        CACHE.emplace_front(key, slow_get_page(key), FREQUENCY_LIST[new_freq].begin());
        HASH.emplace(key, CACHE.begin());
        return false;
    }
}; // end of LFU_cache_t

} // namespace caches
