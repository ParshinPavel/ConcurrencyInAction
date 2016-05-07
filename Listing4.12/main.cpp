#include <iostream>
#include <list>
#include <algorithm>
#include <sstream>
#include <future>

// Реализация алгоритма быстрой сортировки
// в духе функционального программирования.
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}

	std::list<T> result;

	// Извлекаем первый элемент списка и выбираем его
	// в качестве опорного. Переносим этот элемент сразу в
	// результирующий список.
	result.splice(result.begin(), input, input.begin());

	const T& pivot = result.front();

	// Производим разбиение входного списка относительно опорного элемента.
	auto divide_point = std::partition(input.begin(), input.end(), [&pivot](const T& em) {
		return em < pivot;
	});

	// Создаем новый список, в который переносим все элементы,
	// которые меньше опорного.
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	// Рекурсивная сортировка двух полученных частей входного списка.
	auto new_lower(sequential_quick_sort(std::move(lower_part)));
	auto new_higher(sequential_quick_sort(std::move(input)));

	// Объединяем полученные результаты в единый.
	result.splice(result.begin(), new_lower);
	result.splice(result.end(), new_higher);

	return result;
}

// Реализация быстрой сортировки с использованием
// параллелизма в духе ФП.
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}

	std::list<T> result;

	// Извлекаем первый элемент списка и выбираем его
	// в качестве опорного. Переносим этот элемент сразу в
	// результирующий список.
	result.splice(result.begin(), input, input.begin());

	const T& pivot = result.front();

	// Производим разбиение входного списка относительно опорного элемента.
	auto divide_point = std::partition(input.begin(), input.end(), [&pivot](const T& em) {
		return em < pivot;
	});

	// Создаем новый список, в который переносим все элементы,
	// которые меньше опорного.
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	// Рекурсивная сортировка двух полученных частей входного списка.
	// Первую часть списка сортируем в отдельном потоке,
	// вторую - в текущем, чтобы уменьшить количество создаваемых потоков.
	// Иначе при большом числе элементов в исходном списке может получить
	// слишком большое количество созданных потоков, что приведет
	// к снижению производительности.
	std::future<std::list<T>> new_lower_future = std::async(parallel_quick_sort<T>, std::move(lower_part));
	auto new_higher(parallel_quick_sort(std::move(input)));

	// Объединяем полученные результаты в единый.
	result.splice(result.begin(), new_lower_future.get());
	result.splice(result.end(), new_higher);

	return result;
}

template<typename T = int>
std::string to_string(const std::list<T>& list) {
	std::stringstream ss;
	for (const auto& em : list) {
		ss << em << " ";
	}
	return ss.str();
}

int main() {
	std::list<int> input{1, 4, 2, 5, 7, 3, 8, 5, 3, 21, 9, 0};
	std::cout << "Input list: " << to_string(input) << std::endl;

	const auto seq_sorted_list = sequential_quick_sort(input);
	std::cout << "After sequential sort: " << to_string(seq_sorted_list) << std::endl;

	const auto parallel_sorted_list = parallel_quick_sort(input);
	std::cout << "After parallel sort: " << to_string(parallel_sorted_list) << std::endl;
}