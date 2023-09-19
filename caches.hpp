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

template <typename Container>
void print_map(const Container &container)
{
    for (const auto &v : container)
    {
        std::cout << v.first << ": ";
        print_container(v.second);
    }
}


template <typename T, typename KeyT = int>
class LFU_cache_t
{
private:
    size_t sz_;
    std::unordered_map<KeyT, size_t> HIST;
    std::unordered_map<size_t, std::list<KeyT>> FREQUENCY_LIST;

    using FreqIt = typename std::list<KeyT>::iterator;
    std::list<std::tuple<KeyT, T, FreqIt>> CACHE;

    using ListIt = typename std::list<std::tuple<KeyT, T, FreqIt>>::iterator;
    std::unordered_map<KeyT, ListIt> HASH;

    // auto determine_victim(size_t given_frequency) const;
    // bool is_full() const;

public:
    LFU_cache_t(size_t sz) : //constructor
        sz_(sz) {}

    // void print_cache() const;
    // void print_hist()  const;

    // template <typename F>
    // bool lookup_update(KeyT key, F slow_get_page);


    ListIt determine_victim(size_t given_frequency) // if returns cache_.end() then there is no victim
    {
        if (given_frequency == 0)
        {
            return CACHE.end();
        }

        size_t i = 1;
        while (FREQUENCY_LIST.find(i) == 0 || FREQUENCY_LIST.at(i).empty())
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

        assert(HASH.find(key) != 0);
        return HASH.at(key);
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
            std::cout << std::get<0>(i) << " ";
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
        return;
    }

    void print_freq_list() const
    {
        std::cout << "FREQUENCY_LIST:\n";
        print_map(FREQUENCY_LIST);
        std::cout << "\n";
        return;
    }

    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page)
    {
        if (HIST.count(key) == 0) // add to HIST
        {
            HIST.emplace(key, 0);
        }

        if (HASH.count(key) != 0) // found
        {
            size_t freq = HIST[key];
            HIST[key] += 1; // increase frequency

            auto iterator = std::get<2>(*HASH[key]);
            FREQUENCY_LIST[freq].erase(iterator);

            if (FREQUENCY_LIST.count(freq + 1) == 0)
            {
                std::list<KeyT> temp_list;
                FREQUENCY_LIST.emplace(freq + 1, temp_list);
            }
            FREQUENCY_LIST[freq + 1].emplace_front(key);

            std::get<2>(*HASH[key]) = FREQUENCY_LIST[freq + 1].begin();
            return true;
        }


        size_t freq = HIST[key];

        if (is_full()) // gotta find the victim
        {
            auto victim = determine_victim(freq);
            if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
            {
                HIST[key] += 1; // increase frequency
                return false;
            }

            // erase victim from CACHE and from FREQUENCY_LIST

            // std::cout << "victim: " << std::get<0>(*victim) << "\n";


            KeyT victim_key = std::get<0>(*victim);
            size_t victim_freq = HIST[victim_key];
            auto iterator = std::get<2>(*victim);
            FREQUENCY_LIST[victim_freq].erase(iterator);
            HASH.erase(victim_key);
            CACHE.erase(victim);
        }
        // now there is free space

        // increase frequency and add new page to FREQUENCY_LIST[freq + 1] and to CACHE
        HIST[key] += 1;

        if (FREQUENCY_LIST.count(freq + 1) == 0)
        {
            std::list<KeyT> temp_list;
            FREQUENCY_LIST.emplace(freq + 1, temp_list);
        }
        FREQUENCY_LIST[freq + 1].emplace_front(key);

        CACHE.emplace_front(key, slow_get_page(key), FREQUENCY_LIST[freq + 1].begin());
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

    // void analyze_request_line();
    // void set_requests(std::vector<KeyT> request_line);
    // void print_cache() const;

    // template <typename F>
    // bool lookup_update(F slow_get_page, int i);



    void analyze_request_line()
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

        if (CALL_TABLE[new_key].size() <= 1) // no more mention of this key
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

    void print_call_table() const
    {
        print_map(CALL_TABLE);
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

        KeyT victim_key = victim->first;
        HASH.erase(victim_key);
        CACHE.erase(victim);
        CACHE.emplace_front(key, slow_get_page(key));
        HASH.emplace(key, CACHE.begin());

        CALL_TABLE[key].pop_front();
        return false;
    }
}; // end of perfect_cache_t
} // namespace caches
