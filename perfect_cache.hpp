#pragma once

#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>
#include <set>

namespace caches
{

template <typename T, typename KeyT>
struct perfect_cache_cell
{
    T data;
    KeyT key;
    size_t next_call;
};


template <typename T, typename KeyT>
struct cell_compare
{
    bool operator() (const perfect_cache_cell<T, KeyT> &lhs, const perfect_cache_cell<T, KeyT> &rhs) const
    {
        return (lhs.next_call > rhs.next_call);
    }
};

template <typename T, typename KeyT = int>
class perfect_cache_t
{
private:
    size_t size_;
    std::vector<KeyT> REQUEST_LINE;

    std::set<perfect_cache_cell<T, KeyT>, cell_compare<T, KeyT>> CACHE;

    using CacheIt = typename std::set<perfect_cache_cell<T, KeyT>, cell_compare<T, KeyT>>::iterator;
    std::unordered_map<KeyT, CacheIt> HASH;
    using HashIt = typename std::unordered_map<KeyT, CacheIt>::iterator;

    std::unordered_map<KeyT, std::list<size_t>> CALL_TABLE;

public:
    perfect_cache_t(size_t sz) : //constructor
        size_(sz) {}

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

    void set_requests(std::vector<KeyT> request_line)
    {
        REQUEST_LINE = request_line;
    }

    bool is_full() const
    {
        return (CACHE.size() == size_);
    }

    void print_cache() const
    {
        std::cout << "CACHE:\n";
        for (auto& i: CACHE)
        {
            std::cout << i.key << " " << i.next_call << "\n";
        }
        std::cout << "\n";
        return;
    }

    void print_hash() const
    {
        std::cout << "HASH:\n";
        for (auto& i: HASH)
        {
            std::cout << i.first << " " << &*(i.second) << std::endl;
        }
        std::cout << "\n";
        return;
    }

    void print_call_table() const
    {
        std::cout << "CALL_TABLE:\n";
        for (const auto &v : CALL_TABLE)
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
    CacheIt determine_victim(KeyT new_key) // if returns CACHE.end() then there is no victim
    {
        assert(CALL_TABLE.count(new_key) != 0);

        if (CALL_TABLE[new_key].size() <= 1) // no more mentions of new_key
        {
            return CACHE.end();
        }
        if (*(CALL_TABLE[new_key].begin()) > CACHE.begin()->next_call)
        {
            return CACHE.end();
        }

        return CACHE.begin();
    }

    void insert_cache_cell(KeyT key, T data)
    {
        assert(CALL_TABLE[key].size() != 0);

        perfect_cache_cell<T, KeyT> temp_cell;
        temp_cell.key = key;
        temp_cell.data = data;
        temp_cell.next_call = *(CALL_TABLE[key].begin());

        auto emplace_log = CACHE.emplace(temp_cell);
        assert(emplace_log.second == true);

        HASH.emplace(key, emplace_log.first);
        return;
    }

    void process_found_page(KeyT key, HashIt page)
    {
        CALL_TABLE[key].pop_front();

        if (CALL_TABLE[key].size() == 0)
        {
            CACHE.erase(page->second);
            HASH.erase(page);
            return;
        }

        T data_buffer = HASH[key]->data;

        CACHE.erase(page->second);
        HASH.erase(page);

        insert_cache_cell(key, data_buffer);
        return;
    }

public:
    template <typename F>
    bool lookup_update(F slow_get_page, size_t i)
    {
        assert(HASH.size() == CACHE.size());
        KeyT key = REQUEST_LINE[i];

        HashIt page = HASH.find(key);

        if (page != HASH.end()) // found
        {
            process_found_page(key, page);
            return true;
        }

        if (CALL_TABLE[key].size() == 1) // this element won't appear later
        {
            CALL_TABLE[key].pop_front();
            return false;
        }

        if (is_full()) // gotta find the victim
        {
            auto victim = determine_victim(key);

            if (victim == CACHE.end()) // each page in the cache is more valuable than a new page
            {
                CALL_TABLE[key].pop_front();
                return false;
            }

            HASH.erase(victim->key);
            CACHE.erase(victim);
        }
        // now there is free space
        CALL_TABLE[key].pop_front();

        insert_cache_cell(key, slow_get_page(key));
        return false;
    }
}; // end of perfect_cache_t
} // namespace caches
