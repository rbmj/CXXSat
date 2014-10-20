#ifndef RANGE_H_INC
#define RANGE_H_INC

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

#endif
