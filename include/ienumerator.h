#pragma once

template <class T> class IEnumerator {
  public:
    virtual bool move_next() = 0;
    virtual const T &get_current() const = 0;
    virtual ~IEnumerator() {}
};
