#pragma once
#ifndef ROWS_BINARY_HEAP
#define ROWS_BINARY_HEAP
#include "row.h"

class rows_binary_heap {
private:
	std::vector<row> data;
	std::vector<size_t> keys;

	bool compare(const row&, const row&) const;
	bool equal(const row&, const row&) const;

	int get_parent(const size_t) const;
	size_t get_left(const size_t) const;
	size_t get_right(const size_t) const;
	void heapify(size_t);
	void soak_helper(const size_t, const size_t);
	void soak(const size_t);

public:
	rows_binary_heap() = default;
	rows_binary_heap(const size_t);
	rows_binary_heap(const std::vector<size_t>&);
	rows_binary_heap(const size_t, const std::vector<size_t>&);
	rows_binary_heap& operator=(const rows_binary_heap&);
	~rows_binary_heap() = default;
	row& get_min();
	const row& get_min() const;
	row extract_min();
	void insert(const row&);
	size_t size() const;
	bool empty() const;
	void clear();
	void build_heap();
	void swap(rows_binary_heap&) noexcept;
};


#endif // !ROWS_BINARY_HEAP

