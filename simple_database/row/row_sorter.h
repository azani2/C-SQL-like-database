#pragma once
#ifndef ROW_SORTER_HEADER
#define ROW_SORTER_HEADER

#include "row_comparer.h"
#include <vector>

class row_sorter {
private:
	static int partition(std::vector<row>&, int, int, const std::vector<size_t>&);
	static void quicksort(std::vector<row>&, int, int, const std::vector<size_t>&);
public: 
	static void sort(std::vector<row>&, const std::vector<size_t>&);
};

#endif // !ROW_SORTER_HEADER



