#include <iterator>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
	const auto length = std::distance(first, last);
	if (!length) {
		return init;
	}

	// Минимальный размер блока для обработки одним потоком.
	const unsigned long min_per_thread = 5;

	// На основе размера исходного вектора определяем оптимальное
	// значение количества потоков.
	const unsigned long max_threads = (length + min_per_thread - 1) / min_per_thread;

	// Количество потоков, которое реально может выполняться конкурентно
	// в текущих условиях выполнения приложения.
	const unsigned long hardware_threads = std::thread::hardware_concurrency();

	const unsigned long num_threads = std::min(hardware_threads == 0 ? 2 : hardware_threads, max_threads);

	// Максимальное количество элементов, которое будет обрабатывать каждый поток.
	const unsigned long block_size = length / num_threads;

	std::vector<T> results(num_threads);
	// Мы уже имеем один запущенный поток, поэтому запускаем на один поток меньше,
	// чем вычисленное значение оптимального числа потоков.
	std::vector<std::thread> threads(num_threads - 1);

	Iterator block_start = first;
	for (unsigned long i = 0; i < num_threads - 1; ++i) {
		Iterator block_end = block_start;
		std::advance(block_end, block_size);
		threads[i] = std::thread([i, block_start, block_end, &results] () {
			results[i] = std::accumulate(block_start, block_end, 0);
		});
		block_start = block_end;
	}

	results[num_threads - 1] = std::accumulate(block_start, last, 0);
	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

	return std::accumulate(results.begin(), results.end(), init);
};

int main() {
	std::vector<int> range {1, 4, 5, 6, 2, 10, 34, 22, 5, 76, 3, 12, 45};
	const auto p_sum = parallel_accumulate(range.begin(), range.end(), 0);
	std::cout << "parallel_accumulate: sum = " << p_sum << std::endl;
	const auto sum = std::accumulate(range.begin(), range.end(), 0);
	std::cout << "std_accumulate: sum = " << sum << std::endl;
}
