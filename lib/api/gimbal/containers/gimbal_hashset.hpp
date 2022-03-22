#ifndef GIMBAL_HASHSET_HPP
#define GIMBAL_HASHSET_HPP

#include "gimbal_container.hpp"
#include "gimbal_hashset.h"
#include "../objects/gimbal_context.hpp"

namespace gimbal {

namespace tags {
    struct HashSet {};
}

template<typename K = void*,
         typename H = std::hash<K>,
         typename P = std::equal_to<K>>
class HashSet;

template<typename K = void*>
class HashSetView;

template<typename CRTP, typename K=void*>
class HashSetBase:
    public tags::HashSet
    //,public ForwardIterable<CRTP, K>
{
public:
    using HashSetBaseType           = HashSetBase<CRTP, K>;
    using DerivedType               = CRTP;
    using KeyType                   = K;
    //using IterableType            = ForwardIterable<CRTP, K>;

    using key_type                  = KeyType;
    using value_type                = KeyType;
    using reference                 = value_type&;
    using const_reference           = const value_type&;
    using pointer                   = value_type*;
    using const_pointer             = const value_type*;

    //using iterator                = IterableType::iterator;
    //using const_iterator          = IterableType::const_iterator;
    //using local_iterator          = IterableType::local_iterator;
    //using const_local_iterator    = IterableType::const_local_iterator;

    using allocator_type            = Context*;
    using size_type                 = Size;
    using difference_type           = ptrdiff_t;

public:

    operator const GblHashSet*() const;

    decltype(auto)  getSet(void) const;
    decltype(auto)  getSet(void);

    size_type       size(void) const noexcept;
    size_type       bucket_count(void) const noexcept;
    size_type       bucket_size(void) const noexcept;
    bool            empty(void) const noexcept;
    GblContext      context(void) const noexcept;

    const_pointer   get(const key_type& key) const noexcept;

    const_reference at(const key_type& key) const;

    size_type       count(const key_type& key) const noexcept;
    bool            contains(const key_type& key) const noexcept;
    const_pointer   probe(size_type position) const noexcept;

    bool            foreach(std::invocable<const key_type&> auto fn) const noexcept;
};



template<typename K>
class HashSetView final: public HashSetBase<HashSetView<K>, K> {
    friend class HashSetBase<HashSetView<K>, K>;
private:
    const GblHashSet* pGblHashSet_ = nullptr;
protected:
    const GblHashSet*   set_(void) const;
public:
                        HashSetView(void) = default;
                        HashSetView(const GblHashSet& gblHashSet);
                        HashSetView(const GblHashSet* pGblHashSet);

    bool                isValid(void) const;
};


template<typename K,
         typename H,
         typename P>
HashSetView(HashSet<K, H, P> base) -> HashSetView<K>;


template<typename K,
         typename H,
         typename P>
class HashSet:
        public GblHashSet,
        public HashSetBase<HashSet<K, H, P>, K>
{
    friend class HashSetBase<HashSet<K, H, P>, K>;

public:
    using BaseType          = HashSetBase<HashSet<K, H, P>, K>;
    using reference         = typename BaseType::reference;
    using const_reference   = typename BaseType::const_reference;
    using pointer           = typename BaseType::pointer;
    using const_pointer     = typename BaseType::const_pointer;
    using size_type         = typename BaseType::size_type;
    using key_type          = typename BaseType::key_type;
    using hasher            = H;
    using key_equal         = P;
private:
    const hasher&           hasher_;
    const key_equal&        comparator_;

    static Hash hasherCb_      (const GblHashSet* pSet, const void* pKey);
    static Int  comparatorCb_  (const GblHashSet* pSet, const void* pKey1, const void* pKey2);
    static void destructCb_    (const GblHashSet* pSet, void* pKey);

protected:
    const GblHashSet*   set_(void) const;
    GblHashSet*         set_(void);

public:

                HashSet(size_type   capacity    = 0,
                        H&          hash        = H(),
                        P&          pred        = P(),
                        Context*    pCtx        = nullptr);

    pointer     get(const K& key) noexcept;

    pointer     set(K& key) noexcept;
    bool        insert(K& key) noexcept;
    void        insert_or_assign(K& key) noexcept;

    bool        erase(const K& key) noexcept;
    pointer     extract(const K& key) noexcept;
    void        clear(void) noexcept;

    pointer     probe(Size position) noexcept;

    /*
    template<typename... Args>
    reference   emplace(Args&&... args) noexcept ;

    template<typename... Args>
    pointer     tryEmplace(Args&&... args) noexcept;
    */


};






// ========== INLINE IMPLEMENTATIONS ==========

// --------- HashSetBase ---------


template<typename CRTP, typename K>
inline decltype(auto) HashSetBase<CRTP, K>::getSet(void) const { return static_cast<const CRTP*>(this)->set_(); }

template<typename CRTP, typename K>
inline decltype(auto) HashSetBase<CRTP, K>::getSet(void) { return static_cast<CRTP*>(this)->set_(); }

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::size(void) const noexcept -> size_type { return GblHashSet_size(getSet()); }

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::bucket_count(void) const noexcept -> size_type { return GblHashSet_bucketCount(getSet()); }

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::bucket_size(void) const noexcept -> size_type { return GblHashSet_bucketSize(getSet()); }

template<typename CRTP, typename K>
inline bool HashSetBase<CRTP, K>::empty(void) const noexcept { return GblHashSet_empty(getSet()); }

template<typename CRTP, typename K>
inline GblContext HashSetBase<CRTP, K>::context(void) const noexcept { return GblHashSet_context(getSet()); }

template<typename CRTP, typename K>
inline HashSetBase<CRTP, K>::operator const GblHashSet*() const { return getSet(); }

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::get(const key_type& key) const noexcept -> const_pointer {
    return static_cast<const_pointer>(GblHashSet_get(getSet(), &key));
}

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::at(const key_type& key) const -> const_reference {
    const GblCallRecord* pRecord = nullptr;
    const key_type* pKey = nullptr;
    gblThreadCallRecordSet(NULL, NULL);
    pKey = reinterpret_cast<const key_type*>(GblHashSet_at(getSet(), &key));
    gblThreadCallRecordGet(NULL, &pRecord);
    Exception::checkThrow(*pRecord);
    return *pKey;
}

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::count(const key_type& key) const noexcept -> size_type {
    return GblHashSet_count(getSet(), &key);
}

template<typename CRTP, typename K>
inline bool HashSetBase<CRTP, K>::contains(const key_type& key) const noexcept {
    return GblHashSet_contains(getSet(), &key);
}

template<typename CRTP, typename K>
inline auto HashSetBase<CRTP, K>::probe(size_type position) const noexcept -> const_pointer {
    return GblHashSet_probe(getSet(), position);
}

template<typename CRTP, typename K>
inline bool HashSetBase<CRTP, K>::foreach(std::invocable<const key_type&> auto fn) const noexcept {
    return GblHashSet_foreach(getSet(), [](const GblHashSet* pSelf,
                                         void*             pKey,
                                         void*             pUd)
                                      {
                                          GBL_UNUSED(pSelf);
                                          return std::invoke(*static_cast<decltype(fn)>(pUd), *static_cast<const_pointer>(pKey));
                                      },
                                      &fn);
}


// -------- HashSetView ---------



template<typename K>
inline const GblHashSet* HashSetView<K>::set_(void) const { return pGblHashSet_; }

template<typename K>
inline HashSetView<K>::HashSetView(const GblHashSet& gblHashSet):
        pGblHashSet_(&gblHashSet) {}

template<typename K>
inline HashSetView<K>::HashSetView(const GblHashSet* pGblHashSet):
        pGblHashSet_(pGblHashSet) {}

template<typename K>
inline bool HashSetView<K>::isValid(void) const { return pGblHashSet_; }

// ------- HashSet --------
template<typename K, typename H, typename P>
inline const GblHashSet* HashSet<K, H, P>::set_(void) const { return static_cast<const GblHashSet*>(this); }

template<typename K, typename H, typename P>
inline GblHashSet* HashSet<K, H, P>::set_(void) { return static_cast<GblHashSet*>(this); }

template<typename K, typename H, typename P>
inline HashSet<K, H, P>::HashSet(size_type   capacity,
                                 H&          hash,
                                 P&          pred,
                                 Context*    pCtx):
    hasher_(hash),
    comparator_(pred)
{
    GblHashSet_construct(this,
                         sizeof(K),
                         hasherCb_,
                         comparatorCb_,
                         destructCb_,
                         capacity,
                         pCtx,
                         this);
}

template<typename K, typename H, typename P>
inline auto HashSet<K, H, P>::get(const K& key) noexcept -> pointer {
    return static_cast<pointer>(GblHashSet_get(this, &key));
}

template<typename K, typename H, typename P>
inline auto HashSet<K, H, P>::set(K& key) noexcept -> pointer {
    return static_cast<pointer>(GblHashSet_set(this, &key));
}

template<typename K, typename H, typename P>
inline bool HashSet<K, H, P>::insert(K& key) noexcept {
    return GblHashSet_insert(this, &key);
}

template<typename K, typename H, typename P>
inline void HashSet<K, H, P>::insert_or_assign(K& key) noexcept {
    GblHashSet_insertOrAssign(this, &key);
}

template<typename K, typename H, typename P>
inline bool HashSet<K, H, P>::erase(const K& key) noexcept {
    return GblHashSet_erase(this, &key);
}

template<typename K, typename H, typename P>
inline auto HashSet<K, H, P>::extract(const K& key) noexcept -> pointer {
    return GblHashSet_extract(this, &key);
}

template<typename K, typename H, typename P>
inline void HashSet<K, H, P>::clear(void) noexcept {
    GblHashSet_clear(this);
}

template<typename K, typename H, typename P>
inline auto HashSet<K, H, P>::probe(Size position) noexcept -> pointer{
    return GblHashSet_probe(this, position);
}

template<typename K, typename H, typename P>
inline Hash HashSet<K, H, P>::hasherCb_(const GblHashSet* pSet, const void* pKey) {
    static_cast<HashSet<K, H, P>*>(pSet)->hasher_(*static_cast<const K*>(pKey));
}

template<typename K, typename H, typename P>
inline Int HashSet<K, H, P>::comparatorCb_ (const GblHashSet* pSet, const void* pKey1, const void* pKey2) {
    static_cast<HashSet<K, H, P>*>(pSet)->comparator_(*static_cast<const K*>(pKey1), *static_cast<const K*>(pKey2));
}

template<typename K, typename H, typename P>
inline void HashSet<K, H, P>::destructCb_(const GblHashSet* pSet, void* pKey) {
    GBL_UNUSED(pSet);
    static_cast<K*>(pKey)->~K();
}



}

#endif // GIMBAL_HASHSET_HPP
