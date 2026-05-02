#include "row_sorter.h"
#include "datetime_comp_ops.h"
#include "date_comp_ops.h"
#include "row.h"
#include "row_comparer.h"

int row_sorter::partition(std::vector<row>& v, int l, int h, const std::vector<size_t>& keys) {
    row pivot = v[l];
    int i = l - 1;
    int j = h + 1;
    while (true) {
        do {
            j--;
        } while (row_comparer::compare_rows(pivot, v[j], keys));
        do {
            i++;
        } while (row_comparer::compare_rows(v[i], pivot, keys));

        if (i < j) {
            std::swap(v[i], v[j]);
        }
        else {
            return j;
        }
    }
}

void row_sorter::quicksort(std::vector<row>& v, int l, int h, const std::vector<size_t>& keys) {
    if (l < h) {
        int mid = partition(v, l, h, keys);
        quicksort(v, l, mid, keys);
        quicksort(v, mid + 1, h, keys);
    }
}

void row_sorter::sort(std::vector<row>& rows, const std::vector<size_t>& keys) {
    quicksort(rows, 0, rows.size() - 1, keys);
}