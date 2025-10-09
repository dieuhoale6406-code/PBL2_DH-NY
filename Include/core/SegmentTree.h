class SegmentTree {
    private:
        int n;
        long long* tree;

        void build(int,int,int,const int*, int);
        void update(int,int,int,int,int);
        long long query(int,int,int,int,int) const;
    public:
        SegmentTree();
        ~SegmentTree();

        void reset(int);
        void buildFromArray(const int*, int);
        void pointAdd(int,int);
        long long rangeSum(int, int) const;
        int size() const;
};