#pragma once

template <class T> class Queue {
    struct Node {
        Node* next;
        Node* prev;
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
        newNode->prev = nullptr;

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;

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
        else
            head->prev = nullptr;

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

    class Iterator {
    public:
        T* value;

        Iterator(Queue<T>* queue) : queue(queue) {
            currentNode = queue->head;

            if (currentNode != nullptr)
                value = currentNode->val;
            else
                currentNode = nullptr;
        }

        bool Next() {
            if (currentNode == nullptr) {
                if (queue->head == nullptr)
                    return false;
                currentNode = queue->head;
            }

            if (currentNode->next == nullptr)
                return false;

            currentNode = currentNode->next;
            value = currentNode->val;
            return true;
        }

        bool Prev() {
            if (currentNode == nullptr) {
                if (queue->head == nullptr)
                    return false;
                currentNode = queue->head;
            }

            if (currentNode->prev == nullptr)
                return false;

            currentNode = currentNode->prev;
            value = currentNode->val;
            return true;
        }

        bool Remove() {
            Node* newNode = nullptr;
            if (currentNode->next != nullptr) {
                currentNode->next->prev = currentNode->prev;
                newNode = currentNode->next;
            }

            if (currentNode->prev != nullptr) {
                currentNode->prev->next = currentNode->next;
                if (newNode == nullptr)
                    newNode = currentNode->prev;
            }

            delete currentNode;

            currentNode = newNode;
            value = currentNode->val;

            return currentNode != nullptr;
        }

    private:
        Node* currentNode;
        Queue* queue;
    };
};