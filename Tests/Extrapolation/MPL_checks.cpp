
// Boost include(s)
#define BOOST_TEST_MODULE ParameterSet Tests
#include <ACTS/Extrapolation/AbortList.hpp>
#include <ACTS/Extrapolation/detail/boost_mpl_helper.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/test/included/unit_test.hpp>
#include <type_traits>
#include "ACTS/Extrapolation/detail/type_collector.hpp"

#include <iostream>

namespace Acts {

namespace Test {

  BOOST_AUTO_TEST_CASE(boost_set_merger_test)
  {
    typedef typename boost::mpl::set<float, int, char, bool>::type first;
    typedef typename boost::mpl::vector<long, int, void*>::type second;
    typedef typename detail::boost_set_merger<first, second>::type found;
    typedef typename boost::mpl::set<float, int, char, bool, long, void*>::type
        expected;

    static_assert(std::is_same<found, expected>::value,
                  "merging sequence into boost::mpl::set failed");
  }

  template <typename... args>
  struct variadic_struct
  {
  };

  BOOST_AUTO_TEST_CASE(unpack_boost_set_as_template_test)
  {
    typedef boost::mpl::set<float, int, char>::type boost_set;
    typedef variadic_struct<float, int, char>       expected;
    typedef typename detail::unpack_boost_set_as_tparams<variadic_struct,
                                                         boost_set>::type found;

    static_assert(std::is_same<found, expected>::value,
                  "using boost::mpl::set for variadic templates failed");
  }

  namespace {
    struct traits1
    {
      typedef int  result_type;
      typedef char observer_type;
    };

    template <bool>
    struct traits2;

    template <>
    struct traits2<false>
    {
      typedef bool  result_type;
      typedef float observer_type;
    };

    template <>
    struct traits2<true>
    {
      typedef float observer_type;
    };
  }

  BOOST_AUTO_TEST_CASE(type_collector_test)
  {
    typedef typename detail::type_collector<detail::result_type_extractor,
                                            traits1,
                                            traits2<true>,
                                            traits2<false>>::type found_results;

    typedef
        typename detail::type_collector<detail::observer_type_extractor,
                                        traits1,
                                        traits2<true>,
                                        traits2<false>>::type found_observers;

    typedef typename boost::mpl::set<int, bool>::type   expected_results;
    typedef typename boost::mpl::set<char, float>::type expected_observers;

    static_assert(std::is_same<found_results, expected_results>::value,
                  "collecting result types failed");
    static_assert(std::is_same<found_observers, expected_observers>::value,
                  "collecting observer types failed");
  }

  BOOST_AUTO_TEST_CASE(abort_list_test)
  {
    typedef AbortList<variadic_struct,
                      DestinationSurface,
                      MaxMaterial,
                      MaxPathLength>
                                      AList;
    typedef AList::result_type        found_result_type;
    typedef AList::observer_list_type found_observer_list_type;
    typedef variadic_struct<MaxMaterial::result_type,
                            MaxPathLength::result_type>
        expected_result_type;

    static_assert(std::is_same<found_result_type, expected_result_type>::value,
                  "creating abort condition list failed");

    static_assert(std::is_base_of<DestinationSurface, AList>::value, "");
    static_assert(std::is_base_of<MaxMaterial, AList>::value, "");
    static_assert(std::is_base_of<MaxPathLength, AList>::value, "");

    AList al;
    al.maxMaterial   = 3;
    al.maxPathLength = 17.8;

    AList c = al;
    std::cout << c.maxMaterial << std::endl;
    std::cout << c.maxPathLength << std::endl;
    std::cout << sizeof(c) << std::endl;
    std::cout << sizeof(double) << std::endl;
  }
}  // namespace Test

}  // namespace Acts
