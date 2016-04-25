#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

template<typename T>
class threadsafe_queue {
  private:
	std::condition_variable cond_var;
	std::queue<T> queue;
	mutable std::mutex mutex;

  public:
	threadsafe_queue() = default;

	threadsafe_queue(const threadsafe_queue& other) {
		std::lock_guard<std::mutex> guard(other.mutex);
		queue = other.queue;
	}

	~threadsafe_queue() = default;

	std::size_t size() const {
		std::lock_guard<std::mutex> guard(mutex);
		return queue.size();
	}

	bool empty() const {
		std::lock_guard<std::mutex> guard(mutex);
		return queue.empty();
	}

	void push(T& data) {
		std::lock_guard<std::mutex> guard(mutex);
		queue.push(data);
		cond_var.notify_one();
	}

	void wait_and_pop(T& data) {
		std::unique_lock<std::mutex> lock(mutex);
		cond_var.wait(lock, [this] () -> bool {
			return !queue.empty();
		});
		data = queue.front();
		queue.pop;
	}

	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock<std::mutex> lock(mutex);
		cond_var.wait(lock, [this] () -> bool {
			return !queue.empty();
		});
		auto data = std::make_shared<T>(queue.front());
		queue.pop();
		return data;
	}

	bool try_pop(T& data) {
		std::unique_lock<std::mutex> lock(mutex);
		if (queue.empty()) {
			return false;
		} else {
			data = queue.front();
			queue.pop;
			return true;
		}
	}

	std::shared_ptr<T> try_pop() {
		std::unique_lock<std::mutex> lock(mutex);
		if (queue.empty()) {
			return nullptr;
		} else {
			auto data = std::make_shared<T>(queue.front());
			queue.pop;
			return data;
		}
	}
};

static volatile bool last_data_received = false;

void producer_thread(threadsafe_queue<int>& q);
void producer_thread(threadsafe_queue<int>& q) {
	for (int i = 0; i< 10; ++i) {
		q.push(i);
		std::cout << "Pushed new data: " << i << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	std::cout << "Producer thread stopped." << std::endl;
	last_data_received = true;
}

void consumer_thread(threadsafe_queue<int>& q);
void consumer_thread(threadsafe_queue<int>& q) {
	while (!last_data_received || !q.empty()) {
		auto data = q.wait_and_pop();
		std::cout << "Received data chunk: " << *data << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::cout << "Consumer thread stopped." << std::endl;
}

int main() {
	threadsafe_queue<int> q;

	std::thread producer(producer_thread, std::ref(q));
	std::thread consumer(consumer_thread, std::ref(q));
	producer.join();
	consumer.join();
}