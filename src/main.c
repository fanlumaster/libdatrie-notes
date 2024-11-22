typedef struct _AlphaRange {
    struct _AlphaRange *next;

    AlphaChar           begin;
    AlphaChar           end;
} AlphaRange;

struct _AlphaMap {
    AlphaRange     *first_range;

    /* work area */
    /* alpha-to-trie map */
    AlphaChar  alpha_begin;
    AlphaChar  alpha_end;
    int        alpha_map_sz;
    TrieIndex *alpha_to_trie_map;

    /* trie-to-alpha map */
    int        trie_map_sz;
    AlphaChar *trie_to_alpha_map;
};

typedef struct _AlphaMap    AlphaMap;


struct _Trie {
    AlphaMap   *alpha_map;
    DArray     *da;
    Tail       *tail;

    Bool        is_dirty;
};

typedef struct _Trie   Trie;

typedef uint32         AlphaChar;
typedef int32          TrieData;
typedef int32          TrieIndex;

TrieIndex
da_get_root (const DArray *d)
{
    /* can be calculated value for multi-index trie */
    return 2;
}

typedef struct {
    TrieIndex   base;
    TrieIndex   check;
} DACell;

struct _DArray {
    TrieIndex   num_cells;
    DACell     *cells;
};
typedef struct _DArray  DArray;

// 编译器优化
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define LIKELY(expr) (__builtin_expect (!!(expr), 1))
#define UNLIKELY(expr) (__builtin_expect (!!(expr), 0))
#else
#define LIKELY(expr) (expr)
#define UNLIKELY(expr) (expr)
#endif

#define DA_SIGNATURE 0xDAFCDAFC
#define DA_POOL_BEGIN 3

DArray *
da_new (void)
{
    DArray     *d;

    d = (DArray *) malloc (sizeof (DArray));
    if (UNLIKELY (!d))
        return NULL;

    d->num_cells = DA_POOL_BEGIN;
    d->cells     = (DACell *) malloc (d->num_cells * sizeof (DACell));
    if (UNLIKELY (!d->cells))
        goto exit_da_created;
    d->cells[0].base = DA_SIGNATURE;
    d->cells[0].check = d->num_cells;
    d->cells[1].base = -1;
    d->cells[1].check = -1;
    d->cells[2].base = DA_POOL_BEGIN;
    d->cells[2].check = 0;

    return d;

exit_da_created:
    free (d);
    return NULL;
}

Trie *
trie_new (const AlphaMap *alpha_map)
{
    Trie *trie;

    trie = (Trie *) malloc (sizeof (Trie));
    if (UNLIKELY (!trie))
        return NULL;

    trie->alpha_map = alpha_map_clone (alpha_map);
    if (UNLIKELY (!trie->alpha_map))
        goto exit_trie_created;

    trie->da = da_new ();
    if (UNLIKELY (!trie->da))
        goto exit_alpha_map_created;

    trie->tail = tail_new ();
    if (UNLIKELY (!trie->tail))
        goto exit_da_created;

    trie->is_dirty = TRUE;
    return trie;

exit_da_created:
    da_free (trie->da);
exit_alpha_map_created:
    alpha_map_free (trie->alpha_map);
exit_trie_created:
    free (trie);
    return NULL;
}

TrieIndex
da_get_base (const DArray *d, TrieIndex s)
{
    return LIKELY (s < d->num_cells) ? d->cells[s].base : TRIE_INDEX_ERROR;
}
#define trie_da_is_separate(da,s)      (da_get_base ((da), (s)) < 0)


