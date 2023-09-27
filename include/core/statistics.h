#pragma once
#include <omp.h>

#include <condition_variable>
#include <string>
#include <thread>
#include <vector>

#include "fwd.h"
#include "object.h"

//#define MTS_NO_STATISTICS 1
inline bool atomicCompareAndExchange(volatile int32_t *v, int32_t newValue, int32_t oldValue) {
    return __sync_bool_compare_and_swap(v, oldValue, newValue);
}

/**
 * \brief Atomically attempt to exchange a 64-bit integer with another value
 *
 * \param v Pointer to the memory region in question
 * \param oldValue Last known value of the destination \a v
 * \param newValue Replacement value for the destination \a v
 * \return \c true if \c *v was equal to \c oldValue and the exchange
 *         was successful.
 */

inline bool atomicCompareAndExchange(volatile int64_t *v, int64_t newValue, int64_t oldValue) {
    return __sync_bool_compare_and_swap(v, oldValue, newValue);
}
#include <malloc.h>

#include <cstddef>
#include <cstdlib>
#define L1_CACHE_LINE_SIZE 64

void *__restrict allocAligned(size_t size);
void freeAligned(void *ptr);

// -----------------------------------------------------------------------
//  Statistics collection
// -----------------------------------------------------------------------

/// Size (in characters) of the console-based progress message
#define PROGRESS_MSG_SIZE 56

/**
 * Specifies the number of internal counters associated with each
 * \ref StatsCounter instance.
 *
 * This is needed for SMP/ccNUMA systems where different processors might
 * be contending for a cache line containing a counter. The solution used
 * here tries to ensure that every processor has  its own local counter.
 */
#define NUM_COUNTERS 128 // Must be a power of 2

/// Bitmask for \ref NUM_COUNTERS
#define NUM_COUNTERS_MASK (NUM_COUNTERS - 1)

/// Determines the multiples (e.g. 1000, 1024) and units of a \ref StatsCounter
enum EStatsType {
    ENumberValue = 0, ///< Simple unitless number, e.g. # of rays
    EByteCount,       ///< Number of read/written/transferred bytes
    EPercentage,      ///< Percentage with respect to a base counter
    EMinimumValue,    ///< Minimum observed value of some quantity
    EMaximumValue,    ///< Maximum observed value of some quantity
    EAverage          ///< Average value with respect to a base counter
};

/**
 * \brief Counter data structure, which is suitable for ccNUMA/SMP machines
 *
 * This counter takes up at least one cache line to reduce false sharing.
 */
struct CacheLineCounter {
    uint64_t value;
    char     unused[120];
};

/** \brief General-purpose statistics counter
 *
 * This class implements a simple counter, which can be used to track various
 * quantities within Mitsuba. At various points during the execution, it is
 * possible to then call \ref Statistics::printStats() to get a human-readable
 * report of their values.
 *
 * \ingroup libcore
 */
class StatsCounter {
public:
    /**
     * \brief Create a new statistics counter
     *
     * \param category Category of the counter when shown in the statistics summary
     * \param name     Name of the counter when shown in the statistics summary
     * \param type     Characterization of the quantity that will be measured
     * \param initial  Initial value of the counter
     * \param base     Initial value of the base counter (only for <tt>type == EPercentage</tt> and <tt>EAverage</tt>)
     */
    StatsCounter(const std::string &category, const std::string &name,
                 EStatsType type = ENumberValue, uint64_t initial = 0L, uint64_t base = 0L);

    /// Free all storage used by the counter
    ~StatsCounter();

    /// Increment the counter value by one
    inline uint64_t operator++() {
#if defined(MTS_NO_STATISTICS)
        // do nothing
        return 0;
#else
        return __sync_fetch_and_add(&m_value[omp_get_thread_num()].value, 1);
    }

    /// Increment the counter by the specified amount
    inline void operator+=(size_t amount) {
#ifdef MTS_NO_STATISTICS
        /// do nothing
#else
        __sync_fetch_and_add(&m_value[omp_get_thread_num()].value, amount);
#endif
    }

    /// Increment the base counter by the specified amount (only for use with EPercentage/EAverage)
    inline void incrementBase(size_t amount = 1) {
#ifdef MTS_NO_STATISTICS
        /// do nothing
#else
        __sync_fetch_and_add(&m_base[omp_get_thread_num()].value, amount);
#endif
    }

    /**
     * \brief When this is a minimum "counter", this function records
     * an observation of the quantity whose minimum is to be determined
     */
    inline void recordMinimum(size_t value) {
        int id = omp_get_thread_num();
#if MTS_32BIT_COUNTERS == 1
        volatile int32_t *ptr =
            (volatile int32_t *) &m_value[id].value;
        int32_t curMinimum;
        int32_t newMinimum = (int32_t) value;
#else
        volatile int64_t *ptr =
            (volatile int64_t *) &m_value[id].value;
        int64_t           curMinimum;
        int64_t           newMinimum = (int64_t) value;
#endif

        do {
            curMinimum = *ptr;
            if (newMinimum >= curMinimum)
                return;
#if (defined(__i386__) || defined(__amd64__))
            __asm__ __volatile__("pause\n");
#endif
        } while (!atomicCompareAndExchange(ptr, newMinimum, curMinimum));
    }

    /**
     * \brief When this is a maximum "counter", this function records
     * an observation of the quantity whose maximum is to be determined
     */
    inline void recordMaximum(size_t value) {
        int id = omp_get_thread_num();
#if MTS_32BIT_COUNTERS == 1
        volatile int32_t *ptr =
            (volatile int32_t *) &m_value[id].value;
        int32_t curMaximum;
        int32_t newMaximum = (int32_t) value;
#else
        volatile int64_t *ptr =
            (volatile int64_t *) &m_value[id].value;
        int64_t curMaximum;
        int64_t newMaximum = (int64_t) value;
#endif

        do {
            curMaximum = *ptr;
            if (newMaximum <= curMaximum)
                return;
#if (defined(__i386__) || defined(__amd64__))
            __asm__ __volatile__("pause\n");
#endif
        } while (!atomicCompareAndExchange(ptr, newMaximum, curMaximum));
    }

    /// Return the name of this counter
    inline const std::string &getName() const { return m_name; }

    /// Return the category of this counter
    inline const std::string &getCategory() const { return m_category; }

    /// Return the type of this counter
    inline EStatsType getType() const { return m_type; }

    /// Return the value of this counter as 64-bit unsigned integer
#ifdef MTS_NO_STATISTICS
    inline uint64_t getValue() const {
        return 0L;
    }
    inline uint64_t getMaximum() const { return 0L; }
    inline uint64_t getMinimum() const { return 0L; }
#else
    inline uint64_t getValue() const {
        uint64_t result = 0;
        for (int i = 0; i < NUM_COUNTERS; ++i)
            result += m_value[i].value;
        return result;
    }

    inline uint64_t getMinimum() const {
        uint64_t result = 0;
        for (int i = 0; i < NUM_COUNTERS; ++i)
            result = std::min(static_cast<uint64_t>(m_value[i].value), result);
        return result;
    }

    inline uint64_t getMaximum() const {
        uint64_t result = 0;
        for (int i = 0; i < NUM_COUNTERS; ++i)
            result = std::max(static_cast<uint64_t>(m_value[i].value), result);
        return result;
    }
#endif

    /// Get the reference number (only used with the EPercentage/EAverage counter type)
#ifdef MTS_NO_STATISTICS
    inline uint64_t getBase() const {
        return 0L;
    }
#else
    inline uint64_t getBase() const {
        uint64_t result = 0;
        for (int i = 0; i < NUM_COUNTERS; ++i)
            result += m_base[i].value;
        return result;
    }
#endif

    /// Reset the stored counter values
    inline void reset() {
        for (int i = 0; i < NUM_COUNTERS; ++i) {
            m_value[i].value = m_base[i].value = 0;
        }
    }

    /// Sorting by name (for the statistics)
    bool operator<(const StatsCounter &v) const;

private:
    std::string m_category;
    std::string m_name;
    EStatsType m_type;
    CacheLineCounter *m_value;
    CacheLineCounter *m_base;
};

/** \brief Collects various rendering statistics and presents them
 * in a human-readable form.
 *
 * \remark Only the \ref getInstance(), \ref getStats(), and
 * \ref printStats() functions are implemented in the Python bindings.
 *
 * \ingroup libcore
 * \ingroup libpython
 */
class Statistics : public Object {
public:
    std::string type_name() const override { return "Statistics"; }
    /// Return the global stats collector instance
    static Statistics *getInstance() {
        static Statistics instance;
        return &instance;
    }

    /// Register a counter with the statistics collector
    void registerCounter(const StatsCounter *ctr);

    /// Record that a plugin has been loaded
    void logPlugin(const std::string &pname, const std::string &descr);

    /// Print a summary of gathered statistics
    void printStats();

    /// Return a string containing gathered statistics
    std::string getStats();

    /// Reset all statistics counters
    void resetAll();

    /// Initialize the global statistics collector
    static void staticInitialization();

    /// Free the memory taken by staticInitialization()
    static void staticShutdown();

protected:
    /// Create a statistics instance
    Statistics();
    /// Virtual destructor
    virtual ~Statistics() {}

private:
    struct compareCategory {
        bool operator()(const StatsCounter *c1, const StatsCounter *c2) {
            if (c1->getCategory() == c2->getCategory())
                return c1->getName() <= c2->getName();
            return c1->getCategory() < c2->getCategory();
        }
    };

    std::vector<const StatsCounter *> m_counters;
    std::vector<std::pair<std::string, std::string>> m_plugins;
    std::mutex m_mutex;
};

#endif /* __MITSUBA_CORE_STATISTICS_H_ */
