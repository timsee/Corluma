#ifndef COR_UTILS_DICTIONARY_H
#define COR_UTILS_DICTIONARY_H

#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/exception.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief Simple dictionary optimized for lookup times. Keys are always std::string, but any type
 * can be used for the items, provided they have a hash and an equal operator. Insertions only be
 * succesful if neither the item nor the key exists, and upon a failed insertion the insert function
 * returns false. Lookups and removals run in constant runtime, but will return false if an invalid
 * argument is given to them. For instance, if the user tries to remove a key that doesn't exist, it
 * will return false.
 */
template <typename T>
class Dictionary {
public:
    /// default constructor
    Dictionary() {}

    /// constructor
    Dictionary(const std::vector<std::pair<std::string, T>>& objectList) {
        for (const auto& object : objectList) {
            insert(object.first, object.second);
        }
    }

    /*!
     * \brief item getter for a item based on a key. Lookup time is in constant runtime,
     *        This will return false with an empty item if it does not exst.
     *
     * \param key key to request an item for.
     * \return a pair where the first value is the item, and the second is whether or not it was
     * successful.
     */
    std::pair<T, bool> item(const std::string& key) const {
        auto iterator = mKeyToItemMap.find(key);
        if (iterator == mKeyToItemMap.end()) {
            return std::make_pair(T{}, false);
        } else {
            return std::make_pair(iterator->second, true);
        }
    }

    /*!
     * \brief key getter for a key based on an item. Lookup time is in constant runtime,
     *        This will  return false with an empty string if it does not exist
     *
     * \param item item to request a key for
     * \return a pair where the first value is the item, and the second is whether or not it was
     * successful.
     */
    std::pair<std::string, bool> key(const T& item) const {
        const auto& iterator = mItemToKeyMap.find(item);
        if (iterator == mItemToKeyMap.end()) {
            return std::make_pair(std::string{}, false);
        } else {
            return std::make_pair(iterator->second, true);
        }
    }

    /*!
     * \brief insert insert an item into the dictionary. This will return whether or not
     *        the insertion is sucessful. An insertion will not be successful if either the key
     *        or the item already exists in the dictionary
     *
     * \param key The key to use for the item
     * \param item The item to store in the dictionary
     * \return true if the insertion is successful, false if it failed
     */
    bool insert(const std::string& key, const T& item) {
        // search to see if key already exists
        auto keyIterator = mKeyToItemMap.find(key);
        bool keyExists = (keyIterator != mKeyToItemMap.end());
        // search to see if item already exists
        auto itemIterator = mItemToKeyMap.find(item);
        bool itemExists = (itemIterator != mItemToKeyMap.end());
        // if either exists already, return false
        if (keyExists || itemExists) {
            return false;
        } else {
            // if neither exists, add to both.
            auto insertIterator = mKeyToItemMap.emplace(key, item);
            mItemToKeyMap.emplace(item, insertIterator.first->first);
            GUARD_EXCEPTION(mItemToKeyMap.size() == mKeyToItemMap.size(),
                            "key map and item map don't match in size");
            return true;
        }
    }

    /*!
     * \brief update update the value with the given key to the new provided value, removing the old
     * one.
     *
     * \param key key to use for update
     * \param item item to update the key to
     *
     * \return this will return false if the key is not found, or if the item
     */
    bool update(const std::string& key, T item) {
        bool result = removeKey(key);
        if (!result) {
            return false;
        }
        result = insert(key, item);
        return result;
    }

    /*!
     * \brief remove removes an item from dictionary, removing both its key and the item,
     *        and decrementing the size of the dictionary by 1. This will throw if the item
     *        does not exist
     *
     * \param item the item to remove from the dictionary
     */
    bool remove(const T& i) {
        auto itemIterator = mItemToKeyMap.find(i);
        if (itemIterator == mItemToKeyMap.end()) {
            return false;
        }
        auto keyIterator = mKeyToItemMap.find(itemIterator->second);
        if (keyIterator == mKeyToItemMap.end()) {
            return false;
        }
        mItemToKeyMap.erase(itemIterator);
        mKeyToItemMap.erase(keyIterator);
        GUARD_EXCEPTION(mItemToKeyMap.size() == mKeyToItemMap.size(),
                        "key map and item map don't match in size");
        return true;
    }

    /*!
     * \brief removeKey removes an key from dictionary, removing both the key and its item,
     *        and decrementing the size of the dictionary by 1. This will throw if the key
     *        does not exist
     *
     * \param key the key to remove from the dictionary
     */
    bool removeKey(const std::string& key) {
        auto keyIterator = mKeyToItemMap.find(key);
        if (keyIterator == mKeyToItemMap.end()) {
            return false;
        }
        auto itemIterator = mItemToKeyMap.find(keyIterator->second);
        if (itemIterator == mItemToKeyMap.end()) {
            return false;
        }
        mItemToKeyMap.erase(itemIterator->first);
        mKeyToItemMap.erase(keyIterator->first);
        GUARD_EXCEPTION(mItemToKeyMap.size() == mKeyToItemMap.size(),
                        "key map and item map don't match in size");
        return true;
    }

    /*!
     * \brief keys getter for a vector of the keys used by the Dictionary
     *
     * \return a vector of keys used by the Dictionary
     */
    std::vector<std::string> keys() const {
        std::vector<std::string> keys;
        keys.reserve(mKeyToItemMap.size());
        for (const auto& keyPair : mKeyToItemMap) {
            keys.emplace_back(keyPair.first);
        }
        return keys;
    }

    /*!
     * \brief itemVector Getter for a vector of all items stored in the dictionary
     *
     * \return a vector of all items stored in the dictionary.
     */
    std::vector<T> items() const {
        std::vector<T> items;
        items.reserve(mKeyToItemMap.size());
        for (const auto& keyPair : mItemToKeyMap) {
            items.push_back(keyPair.first);
        }
        return items;
    }

    /// returns true if empty, false if it has any values
    bool empty() const noexcept { return mKeyToItemMap.empty(); }

    /// getter for size of dictionary
    std::size_t size() const noexcept { return mKeyToItemMap.size(); }

protected:
    /*!
     * \brief mKeyToItemMap hash table that stores the key's string and the Item, while
     *        provided constant lookup of item from keys.
     */
    std::unordered_map<std::string, T> mKeyToItemMap;

    /*!
     * \brief mItemToKeyMap hash table using the item as its key, providing constant
     *        lookup of keys from items. To avoid memory duplication the keys
     *        are stored as reference_wrappers
     */
    std::unordered_map<T, std::string> mItemToKeyMap;
};

} // namespace cor

#endif // COR_UTILS_DICTIONARY_H
