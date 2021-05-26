#ifndef TEST_GIMBAL_HPP
#define TEST_GIMBAL_HPP

#include <gimbal/gimbal_types.hpp>
#include <gimbal/gimbal_context.hpp>
#include <elysian_qtest.hpp>
#include <QHash>
#include <concepts>
#include <type_traits>

#define GBL_TEST_VERIFY_EXCEPTION_THROWN(expr, result)              \
    do {                                                            \
        QVERIFY_EXCEPTION_THROWN((expr), gimbal::Exception);  \
        try {                                                       \
            (expr);                                                 \
        } catch(const gimbal::Exception& except) {            \
            QCOMPARE(except.getResult(), result);                   \
        }                                                           \
    } while(0)

#define GBL_TEST_VERIFY_RESULT(expr)    \
    QVERIFY(GBL_RESULT_SUCCESS((expr)))

namespace gimbal::test {

#define GBL_CONFIG_OPTIONS_ASSERT_ENABLED_DECL()  \
{   \
    false, GBL_CONFIG_ASSERT_PARTIAL_ENABLED, GBL_CONFIG_ASSERT_ERROR_ENABLED, GBL_CONFIG_ASSERT_UNKNOWN_ENABLED    \
}

#define GBL_CONFIG_OPTIONS_LOG_ENABLED_DECL()  \
{   \
    GBL_CONFIG_LOG_SUCCESS_ENABLED, GBL_CONFIG_LOG_PARTIAL_ENABLED, GBL_CONFIG_LOG_ERROR_ENABLED, GBL_CONFIG_LOG_UNKNOWN_ENABLED \
}

#define GBL_CONFIG_OPTIONS_RECORD_ENABLED_DECL()  \
{ \
    GBL_CONFIG_LAST_CALL_RECORD_SUCCESS_ENABLED, GBL_CONFIG_LAST_CALL_RECORD_PARTIAL_ENABLED, GBL_CONFIG_LAST_CALL_RECORD_ERROR_ENABLED, GBL_CONFIG_LAST_CALL_RECORD_UNKNOWN_ENABLED \
}

#define GBL_CONFIG_OPTIONS_LOG_LEVEL_DECL()  \
{ \
    GBL_CONFIG_LOG_SUCCESS_LEVEL, GBL_CONFIG_LOG_PARTIAL_LEVEL, GBL_CONFIG_LOG_ERROR_LEVEL, GBL_CONFIG_LOG_UNKNOWN_LEVEL \
}

#define GBL_CONFIG_OPTIONS_DECL() \
{ \
    GBL_CONFIG_OPTIONS_ASSERT_ENABLED_DECL(), \
    GBL_CONFIG_OPTIONS_LOG_ENABLED_DECL(), \
    GBL_CONFIG_OPTIONS_RECORD_ENABLED_DECL(), \
    GBL_CONFIG_OPTIONS_LOG_LEVEL_DECL() \
}

struct ConfigOptions {
    std::array<bool, static_cast<uint8_t>(Result::Type::Count)> assertEnabled = GBL_CONFIG_OPTIONS_ASSERT_ENABLED_DECL();
    std::array<bool, static_cast<uint8_t>(Result::Type::Count)> logEnabled = GBL_CONFIG_OPTIONS_LOG_ENABLED_DECL();
    std::array<bool, static_cast<uint8_t>(Result::Type::Count)> recordEnabled = GBL_CONFIG_OPTIONS_RECORD_ENABLED_DECL();
    std::array<LogLevel, static_cast<uint8_t>(Result::Type::Count)> logLevels = GBL_CONFIG_OPTIONS_LOG_LEVEL_DECL();
};

class AllocationTracker {
private:
    struct Allocation {
        StackFrame  frame;
        Size        size;
        Size        alignment;
        QString     pDebugInfoStr;
        void*       pPtr;
        size_t      reallocCount;
    };

    QHash<void*, Allocation> allocations_;

public:

    size_t getActiveAllocationCount(void) const { return allocations_.size(); }
    const auto& getActiveAllocations(void) const { return allocations_; }
    size_t getActiveBytes(void) const {
        size_t bytes = 0;
        for(const auto& alloc : allocations_) bytes += alloc.size;
        return bytes;
    }

    void clear(void) { allocations_.clear(); }

    void allocEvent(const StackFrame& frame, Size size, Size alignment, const char* pDebugInfoStr, void* pNewPtr) {
        auto allocation = Allocation {
            frame,
            size,
            alignment,
            pDebugInfoStr,
            pNewPtr,
            0
        };

        Q_ASSERT(allocations_.constFind(pNewPtr) == allocations_.constEnd());
        allocations_.insert(pNewPtr, std::move(allocation));
    }

    void reallocEvent(const StackFrame& frame, void* pPtr, Size newSize, Size newAlign, void* pNewPtr) {
        GBL_UNUSED(frame);

        auto it = allocations_.find(pPtr);
        if(it != allocations_.end()) {
            Allocation newAlloc = *it;
            newAlloc.size = newSize;
            newAlloc.alignment = newAlign;
            newAlloc.pPtr = pNewPtr;
            ++newAlloc.reallocCount;

            allocations_.erase(it);
            allocations_.insert(pNewPtr, std::move(newAlloc));
        }
    }

    void freeEvent(const StackFrame& frame, void* pPtr) {
        GBL_UNUSED(frame);
        auto it = allocations_.find(pPtr);
        if(it != allocations_.end()) {
            allocations_.erase(it);
        }
    }

};

class ProxyContext: public gimbal::Context {

public:
    ProxyContext(Context* pParent):
        pCtxParent_(pParent)
    {
        GBL_ASSERT(pParent);
    }

    virtual ~ProxyContext(void) {
        delete pCtxParent_;
    }

    Context* getParentContext(void) const { return pCtxParent_; }

    virtual void logPush(const StackFrame& frame) override {
        pCtxParent_->logPush(frame);
    }

    virtual void logPop(const StackFrame& frame, uint32_t count) override {
        pCtxParent_->logPop(frame, count);
    }

    virtual void logWrite(const StackFrame& frame, LogLevel level, const char* pFmt, va_list varArgs) override {
        pCtxParent_->logWrite(frame, level, pFmt, varArgs);
    }

    virtual void* memAlloc(const StackFrame& frame, Size size, Size alignment, const char* pDebugInfoStr) override {
        return pCtxParent_->memAlloc(frame, size, alignment, pDebugInfoStr);
    }
    virtual void* memRealloc(const StackFrame& frame, void* pPtr, Size newSize, Size newAlign) override {
        return pCtxParent_->memRealloc(frame, pPtr, newSize, newAlign);
    }

    virtual void memFree(const StackFrame& frame, void* pPtr) override {
        pCtxParent_->memFree(frame, pPtr);
    }

private:
    Context* pCtxParent_     = nullptr;
};


struct ContextCounters {
    enum class ApiExtCall: uint8_t {
        LogPush,
        LogPop,
        LogWrite,
        MemAlloc,
        MemRealloc,
        MemFree,
        Count
    } ExtOverride;

    int64_t ext[static_cast<size_t>(ApiExtCall::Count)] = { 0 };
    int64_t log[static_cast<size_t>(LogLevel::Count)] = { 0 };
    int64_t logDepth = 0;


    void clear(void) {
        memset(this, 0, sizeof(ContextCounters));
    }

    int64_t getExt(ApiExtCall counter) const {
        return ext[static_cast<size_t>(counter)];
    }

    int64_t getLog(LogLevel level) const {
        return log[static_cast<unsigned>(level)];
    }

    int64_t getLogDepth(void) const { return logDepth; }

    friend ContextCounters operator-(const ContextCounters& lhs, const ContextCounters& rhs) {
        ContextCounters result = lhs;
        result.logDepth -= rhs.logDepth;
        for(size_t c = 0; c < static_cast<size_t>(ApiExtCall::Count); ++c)
            result.ext[c] -= rhs.ext[c];
        for(size_t c = 0; c < static_cast<size_t>(LogLevel::Count); ++c)
            result.log[c] -= rhs.log[c];
        return result;
    }
};

//optionally log allocation events and shit
class MonitorableContext: public ProxyContext {
public:

    MonitorableContext(Context* pParent): ProxyContext(pParent) {}

    //Counters& getCounters(void) { return counters_; }
    const ContextCounters& getCounters(void) const { return counters_; }
    const AllocationTracker& getAllocTracker(void) const { return allocTracker_; }


protected:

    void incExtCounter(ContextCounters::ApiExtCall counter) {
        ++counters_.ext[static_cast<size_t>(counter)];
    }

    void incLogCounter(LogLevel level) {
        ++counters_.log[static_cast<unsigned>(level)];
    }

    virtual void    logPush(const StackFrame& frame) override {
        incExtCounter(ContextCounters::ApiExtCall::LogPush);
        ++counters_.logDepth;
       ProxyContext::logPush(frame);
    }

    virtual void logPop(const StackFrame& frame, uint32_t count) override {
        incExtCounter(ContextCounters::ApiExtCall::LogPop);
        counters_.logDepth -= count;
        ProxyContext::logPop(frame, count);
    }

    virtual void logWrite(const StackFrame& frame, LogLevel level, const char* pFmt, va_list varArgs) override {
        incExtCounter(ContextCounters::ApiExtCall::LogWrite);
        incLogCounter(level);
        ProxyContext::logWrite(frame, level, pFmt, varArgs);
    }

    virtual void* memAlloc(const StackFrame& frame, Size size, Size alignment, const char* pDebugInfoStr) override {
        incExtCounter(ContextCounters::ApiExtCall::MemAlloc);
        void* pRetPtr = ProxyContext::memAlloc(frame, size, alignment, pDebugInfoStr);
        allocTracker_.allocEvent(frame, size, alignment, pDebugInfoStr, pRetPtr);
        return pRetPtr;
    }
    virtual void* memRealloc(const StackFrame& frame, void* pPtr, Size newSize, Size newAlign) override {
        incExtCounter(ContextCounters::ApiExtCall::MemRealloc);
        void* pRetPtr = ProxyContext::memRealloc(frame, pPtr, newSize, newAlign);
        allocTracker_.reallocEvent(frame, pPtr, newSize, newAlign, pRetPtr);
        return pRetPtr;
    }

    virtual void memFree(const StackFrame& frame, void* pPtr) override {
        incExtCounter(ContextCounters::ApiExtCall::MemFree);
        ProxyContext::memFree(frame, pPtr);
        allocTracker_.freeEvent(frame, pPtr);
    }

private:

    AllocationTracker   allocTracker_;
    ContextCounters     counters_;
};



struct MonitorBase{};

template<typename T>
concept Monitorable =
    std::is_base_of_v<MonitorBase, T> &&
//    std::is_default_constructible_v<T> &&
    std::is_copy_constructible_v<T> &&
    std::is_assignable_v<T, T> &&
    requires(T t1, typename T::ValueType v) {
        { t1.getCurrentValue() } -> std::same_as<typename T::ValueType>;
        { v - v }                -> std::same_as<typename T::ValueType>;
    };

template<typename CRTP, typename T>
class Monitor: MonitorBase {
protected:

    Monitor(T beginVal={}, T endVal={}):
        beginValue_(std::move(beginVal)),
        endValue_(std::move(endVal))
    {}

public:
    using ValueType     = T;
    using DerivedType   = CRTP;

    T getBeginValue(void) const;
    T getEndValue(void) const;
    T getCurrentDelta(void) const;
    T getEndDelta(void) const;
    bool isCurrentlyBalanced(void) const;
    bool wasEndBalanced(void) const;

    T begin(void);
    T end(void);

private:

    template<typename D=CRTP>
        requires requires (D d) {
            { d.getCurrentValue() } -> std::same_as<T>;
        }
    T getCurrentValue(void) const
    {
        return static_cast<const CRTP*>(this)->getCurrentValue();
    }

    T beginValue_  = {};
    T endValue_    = {};
};

template<typename CRTP, typename T>
class ContextMonitor: public Monitor<CRTP, T> {
protected:
    const MonitorableContext* pCtx_ = nullptr;
public:
    ContextMonitor(const MonitorableContext* pCtx):
        pCtx_(pCtx) {}

    const MonitorableContext* getContext(void) const { return pCtx_; }
};



class ContextCountersMonitor:
        public ContextMonitor<ContextCountersMonitor, ContextCounters>
{
public:
    using ContextMonitor::ContextMonitor;
    ContextCounters getCurrentValue(void) const { return getContext()->getCounters(); }
};

class ContextActiveAllocMonitor:
        public ContextMonitor<ContextActiveAllocMonitor, int64_t>
{
public:
    using ContextMonitor::ContextMonitor;
    int64_t getCurrentValue(void) const { return static_cast<int64_t>(getContext()->getAllocTracker().getActiveAllocationCount()); }
};


template<Monitorable M, bool RAII=true>
class ScopeGuard: public M  {
public:
    using ValueType = typename M::ValueType;

    ScopeGuard(M monitor={}, ValueType expectedDelta={}, SourceLocation srcLoc=SourceLocation());
    ~ScopeGuard(void);

    ValueType getExpectedDelta(void) const;
    const SourceLocation& getSourceLocation(void) const;
    ValueType end(void);

private:
    SourceLocation srcLoc_;
    ValueType expectedDelta_ = {};
};


// add logging and erroring and shit
template<Monitorable M, bool RAII=true>
class ContextScopeGuard: public ScopeGuard<M, RAII> {
public:
    using ValueType = typename ScopeGuard<M, RAII>::ValueType;
    using ScopeGuard<M, RAII>::ScopeGuard;

    ~ContextScopeGuard(void);

    ValueType end(void);
};

//using LogStackScopeGuard = ContextScopeGuard<LogDepthMonitor>;




template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getBeginValue(void) const { return beginValue_; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getEndValue(void) const { return endValue_; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getCurrentDelta(void) const { return getCurrentValue() - getBeginValue(); }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getEndDelta(void) const { return getEndValue() - getBeginValue(); }
template<typename CRTP, typename T>
inline bool Monitor<CRTP, T>::isCurrentlyBalanced(void) const { return getCurrentDelta() == 0; }
template<typename CRTP, typename T>
inline bool Monitor<CRTP, T>::wasEndBalanced(void) const { return getEndDelta() == 0; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::begin(void) { return beginValue_ = getCurrentValue(); }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::end(void) {
    endValue_ = getCurrentValue();
    return getEndDelta();
}


template<Monitorable M, bool RAII>
inline ScopeGuard<M, RAII>::ScopeGuard(M monitor, ValueType expectedDelta, SourceLocation srcLoc):
    M(std::move(monitor)),
    expectedDelta_(std::move(expectedDelta)),
    srcLoc_(std::move(srcLoc))
{
    if constexpr(RAII) this->begin();
}

template<Monitorable M, bool RAII>
inline auto ScopeGuard<M, RAII>::getExpectedDelta(void) const -> ValueType { return this->expectedDelta_; }

template<Monitorable M, bool RAII>
inline ScopeGuard<M, RAII>::~ScopeGuard(void) {
    if constexpr(RAII) end();
}

template<Monitorable M, bool RAII>
inline ContextScopeGuard<M, RAII>::~ContextScopeGuard(void) {
    if constexpr(RAII) end();
}

template<Monitorable M, bool RAII>
inline const SourceLocation& ScopeGuard<M, RAII>::getSourceLocation(void) const { return this->srcLoc_; }

template<Monitorable M, bool RAII>
inline auto ScopeGuard<M, RAII>::end(void) -> ValueType {
    const auto delta = M::end();

    GBL_UNLIKELY if(delta != getExpectedDelta()) {
        Q_ASSERT(false);
#if 0
        this->getThread()->setCurrentCppExecutionContext(m_cppCtx);
#if ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_ERROR
        this->getThread()->error("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_WARNING
        this->getThread()->warn("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_ASSERT
    assert(false);
#endif
        this->getThread()->syncCppCallerContexts();
#endif
    }
    return delta;
}



template<Monitorable M, bool RAII>
inline auto ContextScopeGuard<M, RAII>::end(void) -> ValueType {
    const auto delta = M::end();

    GBL_UNLIKELY if(delta != this->getExpectedDelta()) {

        [&]() {
            GBL_API_BEGIN(this->getContext());
            GBL_API_VERIFY(delta == this->getExpectedDelta(), GBL_RESULT_ERROR_INTERNAL);
            GBL_API_END();
        }();

#if 0
        this->getThread()->setCurrentCppExecutionContext(m_cppCtx);
#if ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_ERROR
        this->getThread()->error("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_WARNING
        this->getThread()->warn("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_ASSERT
    assert(false);
#endif
        this->getThread()->syncCppCallerContexts();
#endif
    }
    return delta;
}




//error strings?
class TestSet: public elysian::UnitTestSet {
Q_OBJECT
protected:

};

}

#endif // TEST_GIMBAL_HPP
