#pragma once

template <class T> class IEnumerator {
public:
  virtual bool move_next() = 0;
  virtual const T &get_current() const = 0;
  virtual void reset() = 0;
  virtual ~IEnumerator() {}
};

template <class T> class EnumeratorWrapper {
private:
  IEnumerator<T> *iter;
  bool has_current;

public:
  explicit EnumeratorWrapper(IEnumerator<T> *iter)
      : iter(iter), has_current(false) {}

  bool move_next() {
    has_current = iter->move_next();
    return has_current;
  }

  const T &get_current() const { return iter->get_current(); }

  void reset() {
    iter->reset();
    has_current = false;
  }

  ~EnumeratorWrapper() { delete iter; }
};
