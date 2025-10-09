#include <iostream>
#include <cstring>
#include "core/SegmentTree.h"

using namespace std;

void SegmentTree::build(int p,int l,int r,const int* arr, int len) { // đệ quy xây dựng cây
    if(l == r) {
        *(tree + p) = l < len ? *(arr + l) : 0;
        return;
    }
    int mid = (l + r) / 2;
    build(p * 2, l, mid, arr, len);
    build(p * 2 + 1, mid + 1, r, arr, len);
    this->tree[p] = this->tree[p * 2] + this->tree[p * 2 + 1];
}

void SegmentTree::update(int p,int l,int r,int index,int delta) { // dệ quy upd điểm (+ delta vào a[index])
    if(l == r) {
        *(tree + p) += delta;
        return;
    }
    int mid = (l + r) / 2;
    if(index <= mid) update(p * 2, l, mid, index, delta);
    else update(p * 2 + 1, mid + 1, r, index, delta);
    tree[p] = tree[p * 2] + tree[p * 2 + 1];
}

long long SegmentTree::query(int p,int l,int r,int L, int R) const { // đệ quy lấy tổng L->R
    if(R < l || r < L) return 0;
    if(L <= l && r <= R) return tree[p];
    int mid = (l + r) / 2;
    return query(p * 2, l, mid, L, R) + query(p * 2 + 1, mid + 1, r, L, R);
}

SegmentTree::SegmentTree()
    : n(0), tree(nullptr) {} //constructor

SegmentTree::~SegmentTree() { //destructor
    if(this->tree) delete [] this->tree;
}

void SegmentTree::reset(int n) { //khởi tạo lại cây
    SegmentTree::~SegmentTree();
    this->tree = nullptr;
    if(n > 0) {
        this->tree = new long long[4 * n];
        memset(this->tree,0,sizeof(long long) * 4 * n);
    }
    else n = 0;
}

void SegmentTree::buildFromArray(const int* arr, int len) { //xây cây từ mảng (cop cây vào)
    if(n <= 0) reset(len);
    else build(1, 0, n - 1, arr, len);
}

void SegmentTree::pointAdd(int index,int delta) { //upd 1 điểm
    if(this->n <= 0 || index < 0 || index >= n) return;
    update(1, 0, n - 1, index, delta);
}

long long SegmentTree::rangeSum(int L, int R) const {
    if(n <= 0 || L > R) return 0;
    if(L < 0) L = 0;
    if(R >= n) R = n - 1;
    return query( 1, 0, n - 1, L, R);
}

int SegmentTree::size() const {
    return this->n;
}