template<u32 n_words>
struct Static_Bitset {
    static constexpr u32 WORD_BITS = 64;

    u64 words[n_words];

    bool get(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);
        
        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        return (words[w] & (1L << b)) != 0;
    }

    void set(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);

        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        words[w] |= (1L << b);
    }

    void clear(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);
        
        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        words[w] &= ~(1L << b);
    }
};