#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>

void background_work(int i) {
	std::cout << i << std::endl;
}

int main() {
	std::vector<std::thread> pool;
	for (unsigned i = 0; i < 20; ++i) {
		std::cout << "Background task #" << i << " started." << std::endl;
		pool.push_back(std::thread(background_work, i));
	}
	std::for_each(pool.begin(), pool.end(), std::mem_fn(&std::thread::join));
	std::cout << "Background tasks completed." << std::endl;
	return 0;
}
