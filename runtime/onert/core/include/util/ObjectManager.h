/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ONERT_UTIL_OBJECT_MANAGER_H__
#define __ONERT_UTIL_OBJECT_MANAGER_H__

#include <unordered_map>
#include <memory>
#include <list>
#include <functional>

#include <memory>

namespace onert
{
namespace util
{

/**
 * @brief Class that owns objects and maps them with indices as a handle for them
 *
 */
template <typename Index, typename Object> class ObjectManager
{
public:
  ObjectManager() : _index_count{0u} {}

public:
  /**
   * @brief Create an object with args and put it in the container with a new Index for that
   *
   * @param[in] args Arguments for creating Operand object
   * @return Created index that is associated to the object
   */
  template <class... Args> Index emplace(Args &&... args)
  {
    auto index = generateIndex();
    _objects.emplace(index, std::make_unique<Object>(std::forward<Args>(args)...));
    return index;
  }

  /**
   * @brief Put object in the container with a new Index for that
   *
   * @param[in] object Object to be pushed
   * @return Created index that is associated to the object
   */
  Index push(std::unique_ptr<Object> &&object)
  {
    auto index = generateIndex();
    _objects.emplace(index, std::move(object));
    return index;
  }

  /**
   * @brief Remove the object that is associated with the given index
   *
   * @param[in] index Index of the object to be removed
   * @return N/A
   */
  void remove(const Index &index) { _objects.erase(index); }

  /**
   * @brief Get the object that is associated with the given index
   *
   * @param[in] index Index of the object to be returned
   * @return Object
   */
  const Object &at(const Index &index) const { return *(_objects.at(index)); }
  /**
   * @brief Get the object that is associated with the given index
   *
   * @param[in] index Index of the object to be returned
   * @return Object
   */
  Object &at(const Index &index) { return *(_objects.at(index)); }
  /**
   * @brief Get the object that is associated with the given index
   *
   * @param[in] index Index of the object to be returned
   * @return true if such entry exists otherwise false
   */
  bool exist(const Index &index) const
  {
    auto it = _objects.find(index);
    return it != _objects.end();
  }
  /**
   * @brief Iterate over the container with given function
   *
   * @param[in] fn Function to be run for every container entry
   * @return N/A
   */
  void iterate(const std::function<void(const Index &, const Object &)> &fn) const
  {
    for (const auto &e : _objects)
    {
      fn(e.first, *e.second);
    }
  }
  /**
   * @brief Iterate over the container with given function
   *
   * @param[in] fn Function to be run for every container entry
   * @return N/A
   */
  void iterate(const std::function<void(const Index &, Object &)> &fn)
  {
    // TODO Remove this workaround
    // This implementation is a workaround in case of adding operands while iteration
    std::list<Index> l;

    for (auto &e : _objects)
    {
      l.push_back(e.first);
    }

    for (auto index : l)
    {
      fn(index, *_objects[index]);
    }
  }

private:
  Index generateIndex() { return Index{_index_count++}; }

protected:
  std::unordered_map<Index, std::unique_ptr<Object>> _objects;
  uint32_t _index_count;
};

} // namespace util
} // namespace onert

#endif // __ONERT_UTIL_OBJECT_MANAGER_H__
