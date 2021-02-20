#pragma once

template <class T> class Queue {
    struct Node {
        Node* next;
        T* val;
    };

    Node* head;
    Node* tail;

public:
    Queue() : head(nullptr), tail(nullptr){};

    inline void push(T* value) {
        Node* newNode = new Node;
        newNode->val = value;
        newNode->next = nullptr;

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;

            tail = newNode;
        }
    }

    inline void pop() {
        if (head == nullptr)
            return;

        Node* node = head;

        head = head->next;
        if (head == nullptr)
            tail = nullptr;

        delete node;
    }

    inline T* front() {
        if (head == nullptr)
            return nullptr;

        return head->val;
    };

    inline T* back() {
        if (tail == nullptr)
            return nullptr;

        return tail->val;
    }
};