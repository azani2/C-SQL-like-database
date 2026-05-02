#include "rows_binary_heap.h"
#include "row_comparer.h"
#include <stdexcept>

bool rows_binary_heap::compare(const row& r1, const row& r2) const {
	return row_comparer::compare_rows(r1, r2, keys);
}

bool rows_binary_heap::equal(const row& r1, const row& r2) const {
	return row_comparer::eq_rows(r1, r2, keys);
}

int rows_binary_heap::get_parent(const size_t i) const {
	return (i - 1) / 2;
}

size_t rows_binary_heap::get_left(const size_t i) const {
	return 2 * i + 1;
}

size_t rows_binary_heap::get_right(const size_t i) const {
	return 2 * i + 2;
}

void rows_binary_heap::heapify(size_t last_i) {
	if (last_i == 0) return;

	int parent = get_parent(last_i);
	while (parent >= 0 && compare(data[last_i], data[parent])) {
		std::swap(data[parent], data[last_i]);
		last_i = parent;
		parent = get_parent(last_i);
	}
}

void rows_binary_heap::soak_helper(const size_t i, const size_t size) {
	size_t l_child = get_left(i);
	size_t r_child = get_right(i);

	if (l_child >= size) {
		return;
	}
	size_t smaller_child = l_child;
	if (r_child < size && compare(data[r_child], data[l_child])) {
		smaller_child = r_child;
	}

	if (compare(data[i], data[smaller_child]) || equal(data[i], data[smaller_child])) {
		return;
	}

	std::swap(data[i], data[smaller_child]);
	soak_helper(smaller_child, size);
}

void rows_binary_heap::soak(const size_t size) {
	soak_helper(0, size);
}

rows_binary_heap::rows_binary_heap(const size_t capacity) : data(capacity) {}

rows_binary_heap::rows_binary_heap(const std::vector<size_t>& keys) : keys(keys) {}

rows_binary_heap::rows_binary_heap(const size_t capacity, const std::vector<size_t>& keys) : data(capacity), keys(keys) {}

rows_binary_heap& rows_binary_heap::operator=(const rows_binary_heap& other) {
	if (this != &other) {
		data = other.data;
		keys = other.keys;
	}
	return *this;
}

row& rows_binary_heap::get_min() {
	if (empty()) {
		throw std::underflow_error("Heap is empty.");
	}
	return data[0];
}

const row& rows_binary_heap::get_min() const {
	if (empty()) {
		throw std::underflow_error("Heap is empty.");
	}
	return data[0];
}

row rows_binary_heap::extract_min() {
	if (empty()) {
		throw std::underflow_error("Heap is empty.");
	}
	std::swap(data[0], data.back());
	soak(data.size() - 1);
	row temp = data.back();
	data.pop_back();
	return temp;
}

void rows_binary_heap::insert(const row& r) {
	data.push_back(r);
	heapify(data.size() - 1);
}

size_t rows_binary_heap::size() const {
	return data.size();
}

bool rows_binary_heap::empty() const {
	return data.empty();
}

void rows_binary_heap::clear() {
	data.clear();
}

void rows_binary_heap::build_heap() {
	for (int i = data.size() / 2 - 1; i >= 0; i--) {
		soak_helper(i, data.size());
	}
}

void rows_binary_heap::swap(rows_binary_heap& other) noexcept {
	data.swap(other.data);
	keys.swap(other.keys);
}