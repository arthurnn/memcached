/*
 * Summary: C++ interface for memcached server
 *
 * Copy: See Copyright for the status of this software.
 *
 * Authors: Padraig O'Sullivan <osullivan.padraig@gmail.com>
 *          Patrick Galbraith <patg@patg.net>
 */

/**
 * @file memcached.hh
 * @brief Libmemcached C++ interface
 */

#ifndef LIBMEMCACHEDPP_H
#define LIBMEMCACHEDPP_H

#include <libmemcached/memcached.h>

#include <string.h>

#include <string>
#include <vector>

/**
 * This is the core memcached library (if later, other objects
 * are needed, they will be created from this class).
 */
class Memcached
{
public:

  Memcached() 
    : 
      memc(),
      result()
  {
    memcached_create(&memc);
  }

  Memcached(memcached_st *clone) 
    : 
      memc(),
      result()
  {
    memcached_clone(&memc, clone);
  }

  Memcached(const Memcached &rhs)
    :
      memc(),
      result()
  {
    memcached_clone(&memc, const_cast<memcached_st *>(&rhs.getImpl()));
  }

  ~Memcached()
  {
    memcached_free(&memc);
  }

  /**
   * Get the internal memcached_st *
   */
  memcached_st &getImpl()
  {
    return memc;
  }

  /**
   * Get the internal memcached_st *
   */
  const memcached_st &getImpl() const
  {
    return memc;
  }

  /**
   * Return an error string for the given return structure.
   *
   * @param[in] rc a memcached_return structure
   * @return error string corresponding to given return code in the library.
   */
  const std::string getError(memcached_return rc) const
  {
    /* first parameter to strerror is unused */
    return memcached_strerror(NULL, rc);
  }

  bool fetch(std::string &key, 
             std::vector<char> &ret_val,
             uint32_t *flags,
             memcached_return *rc)
  {
    char ret_key[MEMCACHED_MAX_KEY];
    size_t value_length= 0;
    size_t key_length= 0;
    char *value= memcached_fetch(&memc, ret_key, &key_length,
                                 &value_length, flags, rc);
    if (value)
    {
      ret_val.reserve(value_length);
      memcpy(&*ret_val.begin(), value, value_length);
      key.assign(ret_key);
      return true;
    }
    return false;
  }

  std::vector<char> &get(const std::string &key, 
                         std::vector<char> &ret_val)
  {
    uint32_t flags= 0;
    memcached_return rc;
    size_t value_length= 0;

    char *value= memcached_get(&memc, key.c_str(), key.length(),
                               &value_length, &flags, &rc);
    if (value != NULL)
    {
      ret_val.reserve(value_length);
      memcpy(&ret_val[0], value, value_length);
    }
    return ret_val;
  }

  std::vector<char> &getByKey(const std::string &master_key, 
                              const std::string &key, 
                              std::vector<char> &ret_val)
  {
    uint32_t flags= 0;
    memcached_return rc;
    size_t value_length= 0;

    char *value= memcached_get_by_key(&memc, 
                                      master_key.c_str(), master_key.length(), 
                                      key.c_str(), key.length(),
                                      &value_length, &flags, &rc);
    if (value)
    {
      ret_val.reserve(value_length);
      memcpy(&*ret_val.begin(), value, value_length);
    }
    return ret_val;
  }

  bool mget(std::vector<std::string> &keys)
  {
    std::vector<const char *> real_keys;
    std::vector<size_t> key_len;
    /*
     * Construct an array which will contain the length
     * of each of the strings in the input vector. Also, to
     * interface with the memcached C API, we need to convert
     * the vector of std::string's to a vector of char *.
     */
    real_keys.reserve(keys.size());
    key_len.reserve(keys.size());

    std::vector<std::string>::iterator it= keys.begin();

    while (it != keys.end())
    {
      real_keys.push_back(const_cast<char *>((*it).c_str()));
      key_len.push_back((*it).length());
      ++it;
    }

    /*
     * If the std::vector of keys is empty then we cannot 
     * call memcached_mget as we will get undefined behavior.
     */
    if (!real_keys.empty())
    {
      memcached_return rc= memcached_mget(&memc, &real_keys[0], &key_len[0], 
                                          real_keys.size());
      return (rc == MEMCACHED_SUCCESS);
    }

    return false;
  }

  bool set(const std::string &key, 
           const std::vector<char> &value,
           time_t expiration,
           uint32_t flags)
  {
    memcached_return rc= memcached_set(&memc, 
                                       key.c_str(), key.length(),
                                       &value[0], value.size(),
                                       expiration, flags);
    return (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  }

  bool setAll(std::vector<std::string> &keys,
              std::vector< std::vector<char> > &values,
              time_t expiration,
              uint32_t flags)
  {
    if (keys.size() != values.size())
    {
      return false;
    }
    bool retval= true;
    std::vector<std::string>::iterator key_it= keys.begin();
    std::vector< std::vector<char> >::iterator val_it= values.begin();
    while (key_it != keys.end())
    {
      retval= set((*key_it), (*val_it), expiration, flags);
      if (retval == false)
      {
        return retval;
      }
      ++key_it;
      ++val_it;
    }
    return retval;
  }

  bool setByKey(const std::string &master_key, 
                const std::string &key, 
                const std::vector<char> &value,
                time_t expiration,
                uint32_t flags)
  {
    memcached_return rc= memcached_set_by_key(&memc, master_key.c_str(), 
                                              master_key.length(),
                                              key.c_str(), key.length(),
                                              &value[0], value.size(),
                                              expiration,
                                              flags);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool increment(const std::string &key, unsigned int offset, uint64_t *value)
  {
    memcached_return rc= memcached_increment(&memc, key.c_str(), key.length(),
                                             offset, value);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool decrement(const std::string &key, unsigned int offset, uint64_t *value)
  {
    memcached_return rc= memcached_decrement(&memc, key.c_str(), 
                                             key.length(),
                                             offset, value);
    return (rc == MEMCACHED_SUCCESS);
  }


  bool add(const std::string &key, const std::vector<char> &value)
  {
    memcached_return rc= memcached_add(&memc, key.c_str(), key.length(), 
                                       &value[0], value.size(), 0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool addByKey(const std::string &master_key, 
                const std::string &key, 
                const std::vector<char> &value)
  {
    memcached_return rc= memcached_add_by_key(&memc, 
                                              master_key.c_str(),
                                              master_key.length(),
                                              key.c_str(),
                                              key.length(),
                                              &value[0], 
                                              value.size(),
                                              0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool replace(const std::string &key, const std::vector<char> &value)
  {
    memcached_return rc= memcached_replace(&memc, key.c_str(), key.length(),
                                           &value[0], value.size(),
                                           0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool replaceByKey(const std::string &master_key, 
                    const std::string &key, 
                    const std::vector<char> &value)
  {
    memcached_return rc= memcached_replace_by_key(&memc, 
                                                  master_key.c_str(), 
                                                  master_key.length(),
                                                  key.c_str(), 
                                                  key.length(),
                                                  &value[0], 
                                                  value.size(), 
                                                  0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool prepend(const std::string &key, const std::vector<char> &value)
  {
    memcached_return rc= memcached_prepend(&memc, key.c_str(), key.length(),
                                           &value[0], value.size(), 0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool prependByKey(const std::string &master_key, 
                    const std::string &key, 
                    const std::vector<char> &value)
  {
    memcached_return rc= memcached_prepend_by_key(&memc, 
                                                  master_key.c_str(), 
                                                  master_key.length(),
                                                  key.c_str(), 
                                                  key.length(),
                                                  &value[0], 
                                                  value.size(),
                                                  0,
                                                  0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool append(const std::string &key, const std::vector<char> &value)
  {
    memcached_return rc= memcached_append(&memc, 
                                          key.c_str(), 
                                          key.length(),
                                          &value[0], 
                                          value.size(), 
                                          0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool appendByKey(const std::string &master_key, 
                   const std::string &key, 
                   const std::vector<char> &value)
  {
    memcached_return rc= memcached_append_by_key(&memc,
                                                 master_key.c_str(), 
                                                 master_key.length(),
                                                 key.c_str(), 
                                                 key.length(),
                                                 &value[0], 
                                                 value.size(), 
                                                 0, 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool cas(const std::string &key, 
           const std::vector<char> &value, 
           uint64_t cas_arg)
  {
    memcached_return rc= memcached_cas(&memc, key.c_str(), key.length(),
                                       &value[0], value.size(), 
                                       0, 0, cas_arg);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool casByKey(const std::string &master_key, 
                const std::string &key, 
                const std::vector<char> &value, 
                uint64_t cas_arg)
  {
    memcached_return rc= memcached_cas_by_key(&memc,
                                              master_key.c_str(), 
                                              master_key.length(),
                                              key.c_str(), 
                                              key.length(),
                                              &value[0], 
                                              value.size(),
                                              0, 0, cas_arg);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool remove(const std::string &key)
  {
    memcached_return rc= memcached_delete(&memc, key.c_str(), key.length(), 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool removeByKey(const std::string &master_key, 
                   const std::string &key)
  {
    memcached_return rc= memcached_delete_by_key(&memc, 
                                                 master_key.c_str(), 
                                                 master_key.length(),
                                                 key.c_str(), 
                                                 key.length(), 
                                                 0);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool flush(time_t expiration)
  {
    memcached_return rc= memcached_flush(&memc, expiration);
    return (rc == MEMCACHED_SUCCESS);
  }

  bool fetchExecute(memcached_execute_function *callback,
                    void *context,
                    unsigned int num_of_callbacks)
  {
    memcached_return rc= memcached_fetch_execute(&memc,
                                                 callback,
                                                 context,
                                                 num_of_callbacks);
    return (rc == MEMCACHED_SUCCESS);
  }

  /**
   * Get the library version string.
   * @return std::string containing a copy of the library version string.
   */
  const std::string libVersion() const
  {
    const char *ver= memcached_lib_version();
    const std::string version(ver);
    return version;
  }

private:

  memcached_st memc;
  memcached_result_st result;
};

#endif /* LIBMEMCACHEDPP_H */
