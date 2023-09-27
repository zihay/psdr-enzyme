#pragma once
#include <core/fwd.h>
#include <core/object.h>
#include <unordered_map>
#include <variant>
#include <string>
#include <memory>

struct Properties
{
    using Entry = std::variant<bool, int, float, double, 
                                Vector, Spectrum, Matrix4x4, 
                                std::string, std::vector<Vector3i>,
                                std::vector<Vector2>, std::vector<Vector>,
                                Object *,
                                Properties>;
    using PropertiesPrivate = std::unordered_map<std::string, Entry>;

    Properties() : m_data(new PropertiesPrivate()){};
    Properties(const PropertiesPrivate &other)
        : m_data{new PropertiesPrivate(other)} {}

    void merge(const Properties &p)
    {
        for (const auto &e : *p.m_data)
            m_data->operator[](e.first) = e.second;
    }

    template <typename T>
    Properties &set(const std::string &key, T value)
    {
        m_data->operator[](key) = Entry(value);
        return *this;
    }

    bool has(const std::string &key) const
    {
        return m_data->find(key) != m_data->end();
    }

    // check if the key exists and the type is correct
    template <typename T>
    bool has(const std::string &key) const
    {
        auto it = m_data->find(key);
        if (it == m_data->end())
            return false;
        return std::holds_alternative<T>(it->second);
    }

    template <typename T>
    T get(const std::string &key) const
    {
        auto it = m_data->find(key);
        if (it == m_data->end())
        {
            throw std::runtime_error("Property not found: " + key);
        }
        try
        {
            return std::get<T>(it->second);
        }
        catch (std::bad_variant_access &)
        {
            throw std::runtime_error("Property type mismatch: " + key);
        }
    }

    template <typename T>
    T get(const std::string &key, T default_value) const
    {
        auto it = m_data->find(key);
        if (it == m_data->end())
        {
            return default_value;
        }
        try
        {
            return std::get<T>(it->second);
        }
        catch (std::bad_variant_access &)
        {
            throw std::runtime_error("Property type mismatch: " + key);
        }
    }

    PropertiesPrivate *data() { return m_data.get(); }

    std::shared_ptr<PropertiesPrivate> m_data;
};