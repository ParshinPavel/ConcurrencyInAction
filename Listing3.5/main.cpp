#include <exception>
#include <stack>
#include <mutex>
#include <memory>
#include <iostream>

struct empty_stack : public std::exception {
	const char* what() const throw() override {
		return "Stack is empty!";
	}
};

template<typename T>
class threadsafe_stack {
  protected:
	std::stack<T> data;
	mutable std::mutex mutex;

  public:
	threadsafe_stack() = default;

	threadsafe_stack(const threadsafe_stack& other) {
		std::lock_guard<std::mutex> guard(other.mutex);
		data = other.data;
	}

	threadsafe_stack& operator=(const threadsafe_stack&) = delete;

	~threadsafe_stack() = default;

	void push(const T& value) {
		std::lock_guard<std::mutex> guard(mutex);
		data.push(value);
	}

	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> guard(mutex);
		if (data.empty()) {
			throw empty_stack();
		}

		auto res = std::make_shared<T>(data.top());
		data.pop();
		return res;
	}

	void pop(T& value) {
		std::lock_guard<std::mutex> guard(mutex);
		if (data.empty()) {
			throw empty_stack();
		}
		value = data.top();
		data.pop();
	}

	bool empty() const {
		std::lock_guard<std::mutex> guard(mutex);
		return data.empty();
	}
};

int main() {
	threadsafe_stack<int> s;
	s.push(1);
	s.push(5);
	auto top = s.pop();
	std::cout << "Top value: " << *top << std::endl;
	std::cout << "Is empty: " << s.empty() << std::endl;
}