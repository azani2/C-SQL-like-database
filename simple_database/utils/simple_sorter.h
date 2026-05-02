#pragma once
#ifndef SIMPLE_SORTER_HEADER
#define SIMPLE_SORTER_HEADER
#include <vector>

template<typename T>
class simple_sorter {
private:
	static int partition(std::vector<T>& v, int l, int h) {
        T pivot = v[l];
        int i = l - 1;
        int j = h + 1;
        while (true) {
            do {
                j--;
            } while (v[j] > pivot);
            do {
                i++;
            } while (v[i] < pivot);

            if (i < j) {
                std::swap(v[i], v[j]);
            }
            else {
                return j;
            }
        }
    }

	static void quicksort(std::vector<T>& v, int l, int h) {
		if (l < h) {
			int mid = partition(v, l, h);
			quicksort(v, l, mid);
			quicksort(v, mid + 1, h);
		}
	}

public:
	static void sort(std::vector<T>& v) {
		quicksort(v, 0, v.size() - 1);
	}
};

#endif


