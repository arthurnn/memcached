/*
 * An example file showing the usage of the C++ libmemcached interface.
 */
#include <mem_config.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>

#include <string.h>

#include <libmemcached/memcached.hpp>

using namespace std;
using namespace memcache;

class DeletePtrs
{
public:
  template<typename T>
  inline void operator()(const T *ptr) const
  {
    delete ptr;
  }
};

class MyCache
{
public:

  static const uint32_t num_of_clients= 10;

  static MyCache &singleton()
  {
    static MyCache instance;
    return instance;
  }

  void set(const string &key,
           const vector<char> &value)
  {
    time_t expiry= 0;
    uint32_t flags= 0;
    getCache()->set(key, value, expiry, flags);
  }

  vector<char> get(const string &key)
  {
    vector<char> ret_value;
    getCache()->get(key, ret_value);
    return ret_value;
  }

  void remove(const string &key)
  {
    getCache()->remove(key);
  }

  Memcache *getCache()
  {
    /* 
     * pick a random element from the vector of clients. Obviously, this is
     * not very random but suffices as an example!
     */
    uint32_t index= rand() % num_of_clients;
    return clients[index];
  } 

private:

  /*
   * A vector of clients.
   */
  std::vector<Memcache *> clients;

  MyCache()
    :
      clients()
  {
    /* create clients and add them to the vector */
    for (uint32_t i= 0; i < num_of_clients; i++)
    {
      Memcache *client= new Memcache("127.0.0.1:11211");
      clients.push_back(client);
    }
  }

  ~MyCache()
  {
    for_each(clients.begin(), clients.end(), DeletePtrs());
    clients.clear();
  }

  MyCache(const MyCache&);

};

class Product
{
public:

  Product(int in_id, double in_price)
    :
      id(in_id),
      price(in_price)
  {}

  Product()
    :
      id(0),
      price(0.0)
  {}

  int getId() const
  {
    return id;
  }

  double getPrice() const
  {
    return price;
  }

private:

  int id;
  double price;

};

void setAllProducts(vector<Product> &products)
{
  vector<char> raw_products(products.size() * sizeof(Product));
  memcpy(&raw_products[0], &products[0], products.size() * sizeof(Product));
  MyCache::singleton().set("AllProducts", raw_products);
}

vector<Product> getAllProducts()
{
  vector<char> raw_products = MyCache::singleton().get("AllProducts");
  vector<Product> products(raw_products.size() / sizeof(Product));
  memcpy(&products[0], &raw_products[0], raw_products.size());
  return products;
}

Product getProduct(const string &key)
{
  vector<char> raw_product= MyCache::singleton().get(key);
  Product ret;
  if (! raw_product.empty())
  {
    memcpy(&ret, &raw_product[0], sizeof(Product));
  }
  else
  {
    /* retrieve it from the persistent store */
  }
  return ret;
}

void setProduct(const string &key, const Product &product)
{
  vector<char> raw_product(sizeof(Product));
  memcpy(&raw_product[0], &product, sizeof(Product));
  MyCache::singleton().set(key, raw_product);
}

int main()
{
  Memcache first_client("127.0.0.1:19191");
  map< string, map<string, string> > my_stats;
  first_client.getStats(my_stats);
  
  /*
   * Iterate through the retrieved stats.
   */
  map< string, map<string, string> >::iterator it=
    my_stats.begin();
  while (it != my_stats.end())
  {
    cout << "working with server: " << (*it).first << endl;
    map<string, string> serv_stats= (*it).second;
    map<string, string>::iterator iter= serv_stats.begin();
    while (iter != serv_stats.end())
    {
      cout << (*iter).first << ":" << (*iter).second << endl;
      ++iter;
    }
    ++it;
  }

  return EXIT_SUCCESS;
}
