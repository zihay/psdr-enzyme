
// The main idea of this macro is borrowed from https://github.com/swansontec/map-macro
// (C) William Swanson, Paul Fultz

#undef NDEBUG

#define PSDR_PY_EXPORT(Name) void python_export_##Name(py::module &m)
#define PSDR_PY_DECLARE(Name) extern void python_export_##Name(py::module &m)
#define PSDR_PY_IMPORT(Name) python_export_##Name(m)

#define INACTIVE_FN(name, fn) auto __enzyme_inactivefn_##name = fn;

#define PSDR_EVAL_0(...) __VA_ARGS__
#define PSDR_EVAL_1(...) PSDR_EVAL_0(PSDR_EVAL_0(PSDR_EVAL_0(__VA_ARGS__)))
#define PSDR_EVAL_2(...) PSDR_EVAL_1(PSDR_EVAL_1(PSDR_EVAL_1(__VA_ARGS__)))
#define PSDR_EVAL_3(...) PSDR_EVAL_2(PSDR_EVAL_2(PSDR_EVAL_2(__VA_ARGS__)))
#define PSDR_EVAL_4(...) PSDR_EVAL_3(PSDR_EVAL_3(PSDR_EVAL_3(__VA_ARGS__)))
#define PSDR_EVAL(...) PSDR_EVAL_4(PSDR_EVAL_4(PSDR_EVAL_4(__VA_ARGS__)))
#define PSDR_MAP_END(...)
#define PSDR_MAP_OUT
#define PSDR_MAP_COMMA ,
#define PSDR_MAP_GET_END() 0, PSDR_MAP_END
#define PSDR_MAP_NEXT_0(test, next, ...) next PSDR_MAP_OUT
#define PSDR_MAP_NEXT_1(test, next) PSDR_MAP_NEXT_0(test, next, 0)
#define PSDR_MAP_NEXT(test, next) PSDR_MAP_NEXT_1(PSDR_MAP_GET_END test, next)
#define PSDR_EXTRACT_0(next, ...) next

#define PSDR_MAP_EXPR_NEXT_1(test, next) \
    PSDR_MAP_NEXT_0(test, PSDR_MAP_COMMA next, 0)
#define PSDR_MAP_STMT_NEXT_1(test, next) \
    PSDR_MAP_NEXT_0(test, next, 0)

#define PSDR_MAP_EXPR_NEXT(test, next) \
    PSDR_MAP_EXPR_NEXT_1(PSDR_MAP_GET_END test, next)
#define PSDR_MAP_STMT_NEXT(test, next) \
    PSDR_MAP_STMT_NEXT_1(PSDR_MAP_GET_END test, next)

#define PSDR_MAP_STMT_F_0(call, Derived, peek, ...)      \
    if (strcmp(name, #Derived) == 0)                     \
        return static_cast<const Derived *>(this)->call; \
    PSDR_MAP_STMT_NEXT(peek, PSDR_MAP_STMT_F_1)          \
    (call, peek, __VA_ARGS__)
#define PSDR_MAP_STMT_F_1(call, Derived, peek, ...)      \
    if (strcmp(name, #Derived) == 0)                     \
        return static_cast<const Derived *>(this)->call; \
    PSDR_MAP_STMT_NEXT(peek, PSDR_MAP_STMT_F_0)          \
    (call, peek, __VA_ARGS__)

#define PSDR_MAP_STMT_F(call, ...) \
    char name[100];                \
    name[0] = 0;                   \
    className(name);               \
    PSDR_EVAL(PSDR_MAP_STMT_F_0(call, __VA_ARGS__, (), 0))

// Enzyme cannot currently handle virtual functions very well.
// when the differentiated code contains usage of virtual functions,
// we need to use this macro in the base class to populate the metadata
// of the class and do the virtual function dispatch manually.
#define PSDR_DECLARE_VIRTUAL_CLASS() \
    void className(char *) const;    \
    virtual void classNameImpl(char *) const = 0;

// Since we cannot create a string inside enzyme, we need to use this macro
// to inactivate the function.
#define PSDR_INACTIVE_CLASS(name)                                  \
    __attribute__((optnone)) void name::className(char *str) const \
    {                                                              \
        classNameImpl(str);                                        \
    }                                                              \
    INACTIVE_FN(name##_classname, &name::className)

#define PSDR_IMPLEMENT_VIRTUAL_CLASS(name)       \
    void classNameImpl(char *str) const override \
    {                                            \
        strcpy(str, #name);                      \
    }

#define PSDR_DECLARE_CLASS(name)           \
    std::string type_name() const override \
    {                                      \
        return #name;                      \
    }

#if !defined(NAMESPACE_BEGIN)
#define NAMESPACE_BEGIN(name) \
    namespace name            \
    {
#endif

#if !defined(NAMESPACE_END)
#define NAMESPACE_END(name) }
#endif