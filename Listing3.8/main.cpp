#include <mutex>
#include <climits>
#include <thread>

class hierarchical_mutex {
  private:
	std::mutex internal_mutex;

	// Уровень мьютекса в иерархии,
	// устанавливается в конструкторе.
	const unsigned long hierarchy_value;

	// Предыдущий уровень иерархии,
	// при разблокировке возвращаемся к нему.
	unsigned long previous_hierarchy_value = 0;

	// Текущий захваченный уровень в иерархии,
	// поле имеет потоковую продолжительность хранения.
	static thread_local unsigned long this_thread_hierarchy_value;

	void check_for_hierarchy_violation() {
		// Если при попытке захвата мьютекса
		// его уровень выше текущего уровня
		// уже захваченного мьютекса, то выкидываем исключение,
		// поскольку мы нарушаем иерархию.
		// Мы можем захватывать только мьютексы более низкого уровня.
		if (hierarchy_value >= this_thread_hierarchy_value) {
			throw std::logic_error("mutex hierarchy violated!");
		}
	}

	void update_hierarchy_value() {
		previous_hierarchy_value = this_thread_hierarchy_value;
		this_thread_hierarchy_value = hierarchy_value;
	}

  public:
	explicit hierarchical_mutex(unsigned long value) : hierarchy_value(value) {
		/*NOP*/
	};

	void lock() {
		check_for_hierarchy_violation();
		internal_mutex.lock();
		update_hierarchy_value();
	}

	void unlock() {
		this_thread_hierarchy_value = previous_hierarchy_value;
		internal_mutex.unlock();
	}

	bool try_lock() {
		check_for_hierarchy_violation();
		if (!internal_mutex.try_lock()) {
			return false;
		}
		update_hierarchy_value();
		return true;
	}
};

thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);

// Проверяем работу синхронизации с помощью иерархии мьютексов.

hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

int do_low_level_stuff() {
	return 1;
}

int low_level_func() {
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
	return do_low_level_stuff();
}

void high_level_stuff(int some_param) {
	/*NOP*/
}

void high_level_func() {
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
	high_level_stuff(low_level_func());
}

void thread_a() {
	high_level_func();
}

hierarchical_mutex other_mutex(100);

void do_other_stuff() {
	/*NOP*/
}

void other_stuff() {
	high_level_func();
	do_other_stuff();
}

void thread_b() {
	std::lock_guard<hierarchical_mutex> lk(other_mutex);
	other_stuff();
}

int main() {
	std::thread a(thread_a);
	a.join();
	// Пример потока, который нарушает иерархию мьютексов.
//	std::thread b(thread_b);
//	b.join();
}