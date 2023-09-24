#include <iostream>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <string>
#include <vector>


std::mutex printMtx;
void printOut(std::string message)
{
    std::lock_guard<std::mutex> lock(printMtx);
    std::cout << message << std::endl;
}

struct Node
{
    int _value;
    Node* _next;
    std::mutex _node_mutex;

    Node(int value) : _value(value), _next(nullptr) {}
};

class FineGrainedQueue
{
private:
    Node* _head;
    std::mutex _queue_mutex;

public:
    FineGrainedQueue() : _head(nullptr) {}

    void insertIntoMiddle(int value, int pos)
    {
        printOut("\033[45madding\033[0m: " + std::to_string(value) + " to position " + std::to_string(pos));

        if (pos < 0) {
            printOut("\033[1;31mSORRY\033[0m position " + std::to_string(pos) + " < 0");
            return;
        }

        Node* newNode = new Node(value);
        if (!_head && pos >= 0)
        {
            _queue_mutex.lock();
            _head = newNode;
            _queue_mutex.unlock();
            printOut("\033[46mDONE!\033[0m: " + std::to_string(value) + " to position " + std::to_string(pos));
            return;
        }

        _queue_mutex.lock();
        Node* current = _head;
        _queue_mutex.unlock();

        int queueLength = 1;

        while (current->_next != nullptr) 
        {
            current = current->_next;
            ++queueLength;
        }

        if (pos >= queueLength)
        {
            current->_node_mutex.lock();
            current->_next = newNode; 
            current->_node_mutex.unlock();
            printOut("\033[46mDONE!\033[0m: " + std::to_string(value) + " to position " + std::to_string(pos));
            return;
        }
        else
        {
            current = _head;
            int currentIndex = 0;
            Node* prev;

            while (current->_next != nullptr && currentIndex < pos - 1)
            {
                prev = current;
                current = current->_next;
                currentIndex++;
            }

            std::lock_guard<std::mutex> nodeLock(current->_node_mutex);
            newNode->_next = current->_next;
            current->_next = newNode;
        }
        printOut("\033[46mDONE!\033[0m: " + std::to_string(value) + " to position " + std::to_string(pos));
    }

    void printQueue()
    {
        Node* current = _head;
        int count = 0;
        while (current)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> color(30, 100);
            std::cout << "\033[" << color(gen) <<";" << color(gen) << "m" << current->_value << "\033[0m; ";
            current = current->_next;
            count++;
        }
        std::cout << std::endl;
    }
};


int main()
{
    FineGrainedQueue queue;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> value(0, 1000);
    std::uniform_int_distribution<> position(-10, 100);
    std::vector<std::thread> threads{};

    for (auto i = 0; i < 20; ++i)
        threads.push_back(std::thread (&FineGrainedQueue::insertIntoMiddle, std::ref(queue), value(gen), position(gen)));
    for (auto& thread : threads) 
        thread.join();
    for (auto i = 0; i < 10; ++i) //  just for color
        queue.printQueue();
}