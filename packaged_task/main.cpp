#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <future>

template<typename TaskRes>
class task_pool {
  private:
	std::queue<std::packaged_task<TaskRes()>> internal_queue;
	std::mutex mutex;

  public:
	task_pool() = default;

	task_pool(const task_pool&) = delete;

	task_pool& operator=(const task_pool&) = delete;

	~task_pool() = default;

	void add_task(std::function<TaskRes()> f) {
		std::lock_guard<std::mutex> guard(mutex);
		internal_queue.push(std::packaged_task<TaskRes()>(f));
	};

	std::vector<std::future<TaskRes>> execute_sequentially() {
		std::vector<std::future<TaskRes>> result;
		result.reserve(internal_queue.size());

		std::lock_guard<std::mutex> guard(mutex);
		while (!internal_queue.empty()) {
			auto task = std::move(internal_queue.front());
			internal_queue.pop();
			result.push_back(task.get_future());
			task();
		}

		return result;
	}
};

int main() {
	task_pool<std::string> pool;

	auto print_func = [] (std::string id) -> std::string {
		std::cout << "Задача с идентификатором \"" << id << "\" выполнена." << std::endl;
		return id;
	};

	std::cout << "Добавляем задачи в пул." << std::endl;
	for (std::size_t i = 0; i < 5; ++i) {
		pool.add_task(std::bind(print_func, "task_" + std::to_string(i)));
	}

	std::cout << "Запускаем последовательное выполнение задач из пула." << std::endl;
	auto result = pool.execute_sequentially();
	std::cout << "Выполнение задач пула завершено." << std::endl;

	std::cout << "Проверяем результаты выполненных задач." << std::endl;
	for (auto& f : result) {
		const std::string completed_task_id = f.get();
		std::cout << "Получен результат задачи с идентификатором \"" << completed_task_id << "\"" << std::endl;
	}
}