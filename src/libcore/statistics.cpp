#include <core/statistics.h>
#include <cmath>
#include <sstream>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <iostream>

void *__restrict allocAligned(size_t size)
{
    return memalign(L1_CACHE_LINE_SIZE, size);
}

void freeAligned(void *ptr)
{
    free(ptr);
}

StatsCounter::StatsCounter(const std::string &cat, const std::string &name, EStatsType type, uint64_t initial, uint64_t base)
    : m_category(cat), m_name(name), m_type(type)
{
    m_value = (CacheLineCounter *)allocAligned(sizeof(CacheLineCounter) * NUM_COUNTERS);
    m_base = (CacheLineCounter *)allocAligned(sizeof(CacheLineCounter) * NUM_COUNTERS);
    memset(m_value, 0, sizeof(CacheLineCounter) * NUM_COUNTERS);
    memset(m_base, 0, sizeof(CacheLineCounter) * NUM_COUNTERS);
    m_value[0].value = initial;
    m_base[0].value = base;
    assert(Statistics::getInstance() != NULL);
    Statistics::getInstance()->registerCounter(this);
}

StatsCounter::~StatsCounter()
{
    freeAligned(m_value);
    freeAligned(m_base);
}

bool StatsCounter::operator<(const StatsCounter &v) const
{
    if (getCategory() == v.getCategory())
        return getName() < v.getName();
    return getCategory() < v.getCategory();
}

void Statistics::staticInitialization()
{
    assert(sizeof(CacheLineCounter) == 128);
}

void Statistics::staticShutdown()
{
}

Statistics::Statistics()
{
    printf("Statistics::Statistics()\n");
}

void Statistics::registerCounter(const StatsCounter *ctr)
{
    m_counters.push_back(ctr);
}

void Statistics::logPlugin(const std::string &name, const std::string &descr)
{
    m_plugins.push_back(std::pair<std::string, std::string>(name, descr));
}

void Statistics::printStats()
{
    // mitsuba::Logger *logger = Thread::getThread()->getLogger();
    // std::lock_guard<std::mutex> guard(logger->m_mutex);
    // ELogLevel curLevel = logger->getLogLevel();
    // logger->setLogLevel(EInfo);
    // logger->log(EInfo, NULL, __FILE__, __LINE__, "Statistics:\n%s", getStats().c_str());
    // logger->setLogLevel(curLevel);
    std::cout << getStats() << std::endl;
}

void Statistics::resetAll()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (size_t i = 0; i < m_counters.size(); ++i)
        const_cast<StatsCounter *>(m_counters[i])->reset();
}

std::string Statistics::getStats()
{
    std::ostringstream oss;
    std::lock_guard<std::mutex> lock(m_mutex);
    oss << "--------------------Statistics------------------------------" << std::endl;

    std::string suffixesNumber[] = {"", " K", " M", " G", " T"};
    std::string suffixesByte[] = {" B", " KiB", " MiB", " GiB", " TiB"};
    std::string category = "";
    const int lastSuffix = 4;

    std::sort(m_counters.begin(), m_counters.end(), compareCategory());
    int statsEntries = 0;

    for (size_t i = 0; i < m_counters.size(); ++i)
    {
        const StatsCounter *counter = m_counters[i];
        char temp[128];
        float baseValue = (float)counter->getBase();
        int suffixIndex = 0, suffixIndex2 = 0;
        EStatsType type = counter->getType();

        float value;
        if (type == EMinimumValue)
            value = (float)counter->getMinimum();
        else if (type == EMaximumValue)
            value = (float)counter->getMaximum();
        else
            value = (float)counter->getValue();

        if ((type != EPercentage && value == 0) ||
            (type == EPercentage && baseValue == 0))
            continue;

        if (category != counter->getCategory())
        {
            category = counter->getCategory();
            oss << std::endl
                << "  * " << category << " :" << std::endl;
        }

        switch (type)
        {
        case ENumberValue:
        case EMinimumValue:
        case EMaximumValue:
            while (value > 1000.0f && suffixIndex <= lastSuffix)
            {
                value /= 1000.0f;
                suffixIndex++;
            }

            if (value - std::floor(value) < 0.001f)
                snprintf(temp, sizeof(temp), "    -  %s : %.0f%s", counter->getName().c_str(), value, suffixesNumber[suffixIndex].c_str());
            else
                snprintf(temp, sizeof(temp), "    -  %s : %.3f%s", counter->getName().c_str(), value, suffixesNumber[suffixIndex].c_str());
            break;

        case EByteCount:
            while (value > 1024.0f && suffixIndex < lastSuffix)
            {
                value /= 1024.0f;
                suffixIndex++;
            }
            if (value - std::floor(value) < 0.001f)
                snprintf(temp, sizeof(temp), "    -  %s : %.0f%s", counter->getName().c_str(), value, suffixesByte[suffixIndex].c_str());
            else
                snprintf(temp, sizeof(temp), "    -  %s : %.3f%s", counter->getName().c_str(), value, suffixesByte[suffixIndex].c_str());
            break;

        case EPercentage:
        {
            Float value2 = value, value3 = baseValue;
            while (value2 > 1000.0f && suffixIndex < lastSuffix)
            {
                value2 /= 1000.0f;
                suffixIndex++;
            }
            while (value3 > 1000.0f && suffixIndex2 < lastSuffix)
            {
                value3 /= 1000.0f;
                suffixIndex2++;
            }
            snprintf(temp, sizeof(temp), "    -  %s : %.2f %% (%.2f%s of %.2f%s)",
                     counter->getName().c_str(), baseValue == 0 ? (Float)0 : value / baseValue * 100,
                     value2, suffixesNumber[suffixIndex].c_str(),
                     value3, suffixesNumber[suffixIndex2].c_str());
            break;
        }
        case EAverage:
        {
            Float avg = value / (Float)baseValue;
            Float value2 = value, value3 = baseValue;
            while (value2 > 1000.0f && suffixIndex < lastSuffix)
            {
                value2 /= 1000.0f;
                suffixIndex++;
            }
            while (value3 > 1000.0f && suffixIndex < lastSuffix)
            {
                value3 /= 1000.0f;
                suffixIndex2++;
            }
            snprintf(temp, sizeof(temp), "    -  %s : %.2f (%.2f%s / %.2f%s)",
                     counter->getName().c_str(), avg,
                     value2, suffixesNumber[suffixIndex].c_str(),
                     value3, suffixesNumber[suffixIndex2].c_str());
            break;
        }
        default:
            printf("Unknown stats type!");
        }
        oss << temp << std::endl;
        ++statsEntries;
    }

    if (statsEntries == 0)
    {
        oss << " * Statistics:" << std::endl
            << "     none." << std::endl;
    }

    oss << "------------------------------------------------------------";
    return oss.str();
}