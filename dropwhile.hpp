#ifndef ITER_DROPWHILE_H_
#define ITER_DROPWHILE_H_

#include "internal/iterbase.hpp"
#include "filter.hpp"

#include <utility>
#include <iterator>

namespace iter {
  namespace impl {
    template <typename FilterFunc, typename Container>
    class Dropper;

    using DropWhileFn = IterToolFnOptionalBindFirst<Dropper, BoolTester>;
  }
  constexpr impl::DropWhileFn dropwhile{};
}

template <typename FilterFunc, typename Container>
class iter::impl::Dropper {
 private:
  Container container;
  FilterFunc filter_func;

  friend DropWhileFn;

  Dropper(FilterFunc in_filter_func, Container&& in_container)
      : container(std::forward<Container>(in_container)),
        filter_func(in_filter_func) {}

 public:
  Dropper(Dropper&&) = default;
  class Iterator : public std::iterator<std::input_iterator_tag,
                       iterator_traits_deref<Container>> {
   private:
    using Holder = DerefHolder<iterator_deref<Container>>;
    iterator_type<Container> sub_iter;
    iterator_type<Container> sub_end;
    Holder item;
    FilterFunc* filter_func;

    void inc_sub_iter() {
      ++this->sub_iter;
      if (this->sub_iter != this->sub_end) {
        this->item.reset(*this->sub_iter);
      }
    }

    // skip all values for which the predicate is true
    void skip_passes() {
      while (this->sub_iter != this->sub_end
             && (*this->filter_func)(this->item.get())) {
        this->inc_sub_iter();
      }
    }

   public:
    Iterator(iterator_type<Container>&& iter, iterator_type<Container>&& end,
        FilterFunc& in_filter_func)
        : sub_iter{std::move(iter)},
          sub_end{std::move(end)},
          filter_func(&in_filter_func) {
      if (this->sub_iter != this->sub_end) {
        this->item.reset(*this->sub_iter);
      }
      this->skip_passes();
    }

    typename Holder::reference operator*() {
      return this->item.get();
    }

    typename Holder::pointer operator->() {
      return this->item.get_ptr();
    }

    Iterator& operator++() {
      this->inc_sub_iter();
      return *this;
    }

    Iterator operator++(int) {
      auto ret = *this;
      ++*this;
      return ret;
    }

    bool operator!=(const Iterator& other) const {
      return this->sub_iter != other.sub_iter;
    }

    bool operator==(const Iterator& other) const {
      return !(*this != other);
    }
  };

  Iterator begin() {
    return {std::begin(this->container), std::end(this->container),
        this->filter_func};
  }

  Iterator end() {
    return {std::end(this->container), std::end(this->container),
        this->filter_func};
  }
};

#endif
