#include <concepts>
#include <iostream>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>

template <std::integral T> class ThreadSafeStack {
  private:
    std::stack<int> priStack{};
    std::mutex stackMutex;

  public:
    void push(T newValue) {
        std::scoped_lock stackGuard(stackMutex);
        priStack.push(newValue);
    }
    bool try_pop(T& value) {
        std::scoped_lock stackGuard(stackMutex);
        if(priStack.empty())
            return false;
        value = priStack.top();
        priStack.pop();
        return true;
    }
    bool empty() {
        std::scoped_lock stackGuard(stackMutex);
        return priStack.empty();
    }
};

void consumerJob(ThreadSafeStack<int>& ourStack, int& itemsProcessed) {
    int val;
    while(ourStack.try_pop(val)) {
        ++itemsProcessed;
    }
}

int main() {
    ThreadSafeStack<int> testSubject{};

    std::thread t([&testSubject]() {
        for(int i = 0; i <= 9999; ++i) {
            testSubject.push(i);
        }
    });
    t.join();
    std::vector<int> itemsProcessedHouse(4, 0);
    std::vector<std::thread> thread_pool;
    for(int i{}; i < 4; i++) {
        thread_pool.emplace_back(consumerJob, std::ref(testSubject),
                                 std::ref(itemsProcessedHouse[i]));
    }
    for(auto& t : thread_pool) {
        if(t.joinable()) {
            t.join();
        }
    }
    int final{};
    for(int i{0}; i < 4; i++) {
        final += itemsProcessedHouse[i];
    }
    std::cout << final;
}
