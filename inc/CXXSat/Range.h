#ifndef RANGE_H_INC
#define RANGE_H_INC

#include <type_traits>

template <class It>
class Range {
public:
    Range(It b, It e) : begin_it(b), end_it(e) {}
    Range(Range<It>&& r) = default;
    Range(const Range<It>& r) = default;
    It begin() const {
        return begin_it;
    }
    It end() const {
        return end_it;
    }
private:
    It begin_it;
    It end_it;
};

template <class It>
Range<It> make_range(It a, It b) {
    return Range<It>(a, b);
}

template <class Op, std::size_t... I>
constexpr auto make_array_helper(std::index_sequence<I...> seq, Op op) 
-> std::array<typename std::result_of<Op(std::size_t)>::type, sizeof...(I)>
{
    return {{op(I)...}};
}

template <unsigned N, class Op>
constexpr auto make_array(Op op) 
-> std::array<typename std::result_of<Op(std::size_t)>::type, N>
{
    return make_array_helper(std::make_index_sequence<N>{}, op);
}

#endif
