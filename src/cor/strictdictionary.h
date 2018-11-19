#ifndef COR_UTILS_STRICTDICTIONARY_H
#define COR_UTILS_STRICTDICTIONARY_H

#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include "exception.h"

#include <iostream>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief Simple dictionary optimized for lookup times. The strict aspect of this dictionary comes from the fact
 *        that it will throw an exception if you try to do anything that doesn't make sense, IE, remove a key that doesn't exist
 *        or lookup an item's key that isn't in the dictionary. Keys are always std::string, but any type
 *        can be used for the items, provided they have a hash and an equal operator. Insertions will
 *        only be succesful if neither the item nor the key exists,. Lookups and removals run in constant runtime.
 */
template <typename T>
class StrictDictionary {
public:

    /// default constructor
    StrictDictionary() {}

    /// constructor
    StrictDictionary(const std::vector<std::pair<std::string, T>>& objectList)  {
        for (const auto& object : objectList) {
            insert(object.first, object.second);
        }
    }

    /*!
     * \brief item getter for a item based on a key. Lookup time is in constant runtime,
     *        This will throw an exception if the key does not exist
     * \param key key to request an item for
     * \return the item for the key
     */
    const T& item(const std::string& key) const {
        auto iterator = mKeyToItemMap.find(key);
        GUARD_EXCEPTION(iterator != mKeyToItemMap.end(), "Item not found.");
        return iterator->second;
    }

    /*!
     * \brief key getter for a key based on an item. Lookup time is in constant runtime,
     *        This will throw an exception if the item does not exist
     * \param item item to request a key for
     * \return the key for the item
     */
    const std::string& key(T item) const {
        auto iterator = mItemToKeyMap.find(item);
        GUARD_EXCEPTION(iterator != mItemToKeyMap.end(), "Key not found");
        return iterator->second;
    }

    /*!
     * \brief keys getter for a vector of the keys used by the Dictionary
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
     * \return a vector of all items stored in the dictionary.
     */
    std::vector<T> itemVector() const {
        std::vector<T> items;
        items.reserve(mItemToKeyMap.size());
        for (const auto& keyPair : mItemToKeyMap) {
            items.push_back(keyPair.first);
        }
        return items;
    }

    /*!
     * \brief itemList Getter for a list of all items stored in the dictionary
     * \return a list of all items stored in the dictionary.
     */
    std::list<T> itemList() const {
        std::list<T> items;
        //items.reserve(mItemToKeyMap.size());
        for (const auto& keyPair : mItemToKeyMap) {
            items.push_back(keyPair.first);
        }
        return items;
    }

    /// returns true if empty, false if it has any values
    bool empty() const noexcept {
        return mKeyToItemMap.empty();
    }

    /// getter for size of dictionary
    std::size_t size() const noexcept {
        return mKeyToItemMap.size();
    }

    /*!
     * \brief insert insert an item into the dictionary. This will return whether or not
     *        the insertion is sucessful. An insertion will not be successful if either the key
     *        or the item already exists in the dictionary
     * \param key The key to use for the item
     * \param item The item to store in the dictionary
     * \return true if the insertion is successful, false if it failed
     */
    void insert(const std::string& key, const T& item) {
        // search to see if key already exists
        auto keyIterator = mKeyToItemMap.find(key);
        // assert that key does not exist
        GUARD_EXCEPTION(keyIterator == mKeyToItemMap.end(), "Key already exists");

        // search to see if item already exists
        auto itemIterator = mItemToKeyMap.find(item);
        // assert that item already exists
        GUARD_EXCEPTION(itemIterator == mItemToKeyMap.end(), "Item already exists");

        auto insertIterator = mKeyToItemMap.emplace(key, item);
        mItemToKeyMap.emplace(item, std::ref(insertIterator.first->first));
    }

    /*!
     * \brief update update the given item, provided it is already in the dictionary
     * \param item the item to update
     */
    void update(const T& item) {
        const auto& keyResult = key(item);
        remove(item);
        insert(keyResult, item);
    }

    /*!
     * \brief remove removes an item from dictionary, removing both its key and the item,
     *        and decrementing the size of the dictionary by 1. This will throw if the item
     *        does not exist
     * \param item the item to remove from the dictionary
     */
    void remove(const T& i) {
        auto itemIterator = mItemToKeyMap.find(i);
        GUARD_EXCEPTION(itemIterator != mItemToKeyMap.end(), "Item not found.");
        auto keyIterator = mKeyToItemMap.find(itemIterator->second);
        mItemToKeyMap.erase(itemIterator);
        mKeyToItemMap.erase(keyIterator);
    }

    /*!
     * \brief removeKey removes an key from dictionary, removing both the key and its item,
     *        and decrementing the size of the dictionary by 1. This will throw if the key
     *        does not exist
     * \param key the key to remove from the dictionary
     */
    void removeKey(const std::string& key) {
        auto keyIterator = mKeyToItemMap.find(key);
        GUARD_EXCEPTION(keyIterator != mKeyToItemMap.end(), "Key not found.");
        auto itemIterator = mItemToKeyMap.find(keyIterator->second);
        mItemToKeyMap.erase(keyIterator->second);
        mKeyToItemMap.erase(itemIterator->second);
    }

private:

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
    std::unordered_map<T, std::reference_wrapper<const std::string>> mItemToKeyMap;
};

}

#endif // STRICTDICTIONARY_H
