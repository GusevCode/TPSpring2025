#include <iostream>
#include <algorithm>
#include <cassert>

int exponentialSearch(int A[], int n, int x, int& left) {
    if (left >= n) return -1;

    int right = left;
    if (A[right] == x) {
        int found_pos = right;
        left = right + 1; 
        return found_pos;
    }

    int step = 1;
    while (right + step < n && A[right + step] < x) {
        right += step;
        step *= 2;
    }

    right = std::min(right + step, n - 1);

    int l = left;
    int r = right;
    while (l <= r) {
        int mid = l + (r - l)/2;
        if (A[mid] == x) {
            left = mid + 1; 
            return mid;
        } else if (A[mid] < x) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }

    return -1;
}

void findAndPrintIntersection(int A[], int n, int B[], int m, std::ostream& out) {
    int left = 0;
    bool first = true;

    for (int i = 0; i < m; ++i) {
        int pos = exponentialSearch(A, n, B[i], left);
        if (pos != -1) {
            if (!first) out << " ";
            out << B[i];
            first = false;
        }
    }
    out << std::endl;
}

void run(std::istream& in, std::ostream& out) {
    int n, m;
    in >> n >> m;

    int* A = new int[n];
    for (int i = 0; i < n; ++i) in >> A[i];

    int* B = new int[m];
    for (int i = 0; i < m; ++i) in >> B[i];

    findAndPrintIntersection(A, n, B, m, out);

    delete[] A;
    delete[] B;
}

int main() {
    run(std::cin, std::cout);
    return 0;
}
