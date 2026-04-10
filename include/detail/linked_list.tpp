#include "linked_list.h"

#include <stdexcept>

template <class T> LinkedList<T>::LinkedList() : head(nullptr), tail(nullptr), length(0) {}

template <class T>
LinkedList<T>::LinkedList(const T *items, int count) : head(nullptr), tail(nullptr), length(0) {
    if (count < 0) {
        throw std::invalid_argument("Count cannot be negative");
    }
    if (count > 0 && items == nullptr) {
        throw std::invalid_argument("Items cannot be nullptr");
    }

    for (int index = 0; index < count; index++) {
        append(items[index]);
    }
}

template <class T>
LinkedList<T>::LinkedList(const LinkedList<T> &other) : head(nullptr), tail(nullptr), length(0) {
    for (Node *current = other.head; current != nullptr; current = current->next) {
        append(current->data);
    }
}

template <class T> LinkedList<T> &LinkedList<T>::operator=(const LinkedList<T> &other) {
    if (this == &other) {
        return *this;
    }

    Node *current = head;
    while (current != nullptr) {
        Node *tmp = current;
        current = current->next;
        delete tmp;
    }

    head = nullptr;
    tail = nullptr;
    length = 0;

    Node *other_current = other.head;
    while (other_current != nullptr) {
        append(other_current->data);
        other_current = other_current->next;
    }

    return *this;
}

template <class T> const T &LinkedList<T>::get_first() const {
    if (head == nullptr) {
        throw std::out_of_range("Index out of range");
    }
    return head->data;
}

template <class T> const T &LinkedList<T>::get_last() const {
    if (tail == nullptr) {
        throw std::out_of_range("Index out of range");
    }
    return tail->data;
}

template <class T> int LinkedList<T>::get_length() const { return length; }

template <class T> LinkedList<T> *LinkedList<T>::get_sub_list(int start, int end) {
    if (start < 0 || end < 0 || start >= length || end >= length || start > end) {
        throw std::out_of_range("Index out of range");
    }

    LinkedList<T> *result = new LinkedList<T>();
    Node *current = head;
    for (int index = 0; index < start; index++) {
        current = current->next;
    }
    for (int index = start; index <= end; index++) {
        result->append(current->data);
        current = current->next;
    }
    return result;
}

template <class T> void LinkedList<T>::append(const T &item) {
    Node *node = new Node{item, nullptr};

    if (tail == nullptr) {
        head = node;
        tail = node;
    } else {
        tail->next = node;
        tail = node;
    }

    length++;
}

template <class T> void LinkedList<T>::prepend(const T &item) {
    Node *node = new Node{item, head};
    head = node;
    if (tail == nullptr) {
        tail = node;
    }
    length++;
}

template <class T> void LinkedList<T>::insert_at(const T &item, int index) {
    if (index < 0 || index > length) {
        throw std::out_of_range("Index out of range");
    }

    if (index == 0) {
        prepend(item);
        return;
    }
    if (index == length) {
        append(item);
        return;
    }

    Node *previous = head;
    for (int step = 1; step < index; step++) {
        previous = previous->next;
    }

    Node *node = new Node{item, previous->next};
    previous->next = node;
    length++;
}

template <class T> LinkedList<T>::~LinkedList() {
    Node *current = head;
    while (current != nullptr) {
        Node *next = current->next;
        delete current;
        current = next;
    }
}
