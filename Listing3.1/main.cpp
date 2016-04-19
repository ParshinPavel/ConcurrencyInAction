#include <thread>
#include <list>
#include <mutex>
#include <algorithm>
#include <iostream>

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int value) {
	std::lock_guard<std::mutex> guard(some_mutex);
	some_list.push_back(value);
	std::cout << "Value \"" << value << "\" added to list." << std::endl;
}

bool list_contains(int value) {
	std::lock_guard<std::mutex> guard(some_mutex);
	bool found = std::find(some_list.cbegin(), some_list.cend(), value) != some_list.cend();
	std::cout << "Contains value \"" << value << "\": " << found << std::endl;
	return found;
}

int main() {
	std::list<std::thread> pool;
	for (int i = 0; i < 20; ++i) {
		pool.push_back(std::thread(add_to_list, i));
		pool.push_back(std::thread(list_contains, i));
	}
	std::for_each(pool.begin(), pool.end(), std::mem_fn(&std::thread::join));
}
