template <typename T, bool is_multi_thread = false>
struct TypeAD;

template <typename T>
struct TypeAD<T, false>
{
    TypeAD(const T &val) : val(val), der(val)
    {
        der.setZero();
    }
    T &grad() { return der; }
    void zeroGrad() { der.setZero(); }
    T val;
    T der;
};

template <typename T>
struct TypeAD<T, true>
{
    TypeAD(const T &val, int nworker) : val(val), der(val)
    {
        der.setZero();
        grads = std::vector<T>(nworker, der);
    }
    T &grad()
    {
        merge();
        return der;
    }
    T &grad(int i) { return grads[i]; }
    void setZero()
    {
        der.setZero();
        for (auto &g : grads)
        {
            g.setZero();
        }
    }
    void merge()
    {
        for (auto &g : grads)
        {
            der += g;
        }
    }
    T val;
    T der;
    std::vector<T> grads;
};