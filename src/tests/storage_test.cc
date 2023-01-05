#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <cstdint>
#include <storage/storage.hpp>

#define private public
#include <storage/rbtree.hpp>

#define BOOST_TEST_MODULE StorageTests
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(RBTreeStorageTest)
{
  std::string path = "files/rbt_example.db";
  RBTreeStore store;
  BOOST_CHECK_EQUAL(store.init(path), RC::SUCCESS);
  auto k1 = str_to_vector("Hello");
  auto k2 = str_to_vector("Fuck");
  std::string_view v1 = "World";
  BOOST_CHECK_EQUAL(vec_to_str(store.kv_[k1]), v1);
  BOOST_CHECK_EQUAL(vec_to_str(store.kv_[k2]), v1);
}
