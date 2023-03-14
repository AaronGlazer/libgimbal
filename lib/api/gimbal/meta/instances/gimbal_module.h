/*! \file
 *  \brief GblModule loadable plugin instance and management API
 *  \ingroup metaBuiltinTypes
 *  \todo
 *      - implement properties
 *      - unit testing
 *      - plan out type registration/management
 *      - rig up option group parsery
 *      - stop inheriting GblContext
 */
#ifndef GIMBAL_MODULE_H
#define GIMBAL_MODULE_H

#include "gimbal_context.h"
#include "../ifaces/gimbal_iplugin.h"
#include "../../strings/gimbal_string_ref.h"
#include "../../utils/gimbal_version.h"

/// \ingroup metaBuiltinTypes
#define GBL_MODULE_TYPE                 (GBL_TYPEOF(GblModule))
#define GBL_MODULE(instance)            (GBL_INSTANCE_CAST(instance,  GblModule))
#define GBL_MODULE_CLASS(klass)         (GBL_CLASS_CAST(klass, GblModule))
#define GBL_MODULE_GET_CLASS(instance)  (GBL_INSTANCE_GET_CLASS(instance, GblModule))

#define GBL_REQUIRE(...)                GBL_REQUIRE_(__VA_ARGS__)

#define GBL_SELF_TYPE                   GblModule

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(GblOptionGroup);
GBL_FORWARD_DECLARE_STRUCT(GblModule);

typedef GblBool (*GblModuleIterFn)(GblModule* pIt, void* pClosure);

GBL_CLASS_DERIVE(GblModule, GblContext, GblIPlugin)
    GBL_RESULT  (*pFnLoad)  (GBL_SELF);
    GBL_RESULT  (*pFnUnload)(GBL_SELF);
GBL_CLASS_END

/*! \brief Dynamically loadable service and associated meta types
 *  \ingroup metaBuiltinTypes
 *  \extends GblContext
 *  \implements GblIPlugin
 *  \details
 *  A Module is a lazily-loaded object which is registered then later
 *  can be queried for by anywhere else in the application. This
 *  behavior lets you implement "services" as global singletons which
 *  can be accessed from anywhere. Modules also maintain their own inner
 *  registry of associated meta types, which is managed through the
 *  implemented GblIPlugin interface.
 *  \sa GbModuleClass
 */
GBL_INSTANCE_DERIVE(GblModule, GblContext)
    GblVersion      version;
    GblStringRef*   pPrefix;
    GblStringRef*   pAuthor;
    GblStringRef*   pDescription;
    GblOptionGroup* pOptionGroup;
GBL_INSTANCE_END

GBL_PROPERTIES(GblModule,
    (prefix,      GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_STRING_TYPE),
    (version,     GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_UINT32_TYPE),
    (author,      GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_STRING_TYPE),
    (description, GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_STRING_TYPE),
    (useCount,    GBL_GENERIC, (READ),                    GBL_INT16_TYPE),
    (typeCount,   GBL_GENERIC, (READ),                    GBL_UINT32_TYPE)
)

// ===== Static/Service API =====
GBL_EXPORT GblType     GblModule_type         (void)                     GBL_NOEXCEPT;
GBL_EXPORT GblModule*  GblModule_find         (const char* pName)        GBL_NOEXCEPT;
GBL_EXPORT GblModule*  GblModule_findQuark    (GblQuark name)            GBL_NOEXCEPT;
GBL_EXPORT GblModule*  GblModule_at           (GblSize index)            GBL_NOEXCEPT;
GBL_EXPORT GblSize     GblModule_count        (void)                     GBL_NOEXCEPT;

GBL_EXPORT GblBool     GblModule_foreach      (GblModuleIterFn pFnIter,
                                               void*           pClosure) GBL_NOEXCEPT;

GBL_EXPORT GblModule*  GblModule_require      (const char* pName,
                                               const char* pVersion,
                                               const char* pFile,
                                               const char* pFunc,
                                               GblSize     line)         GBL_NOEXCEPT;

GBL_EXPORT GblModule*  GblModule_requireQuark (GblQuark    quark,
                                               const char* pVersion,
                                               const char* pFile,
                                               const char* pFunc,
                                               GblSize     line)         GBL_NOEXCEPT;

// ===== Instance API =====

GBL_EXPORT GblModule*  GblModule_create       (GblType     derivedType,
                                               const char* pName,
                                               GblVersion  version,
                                               const char* pDescription,
                                               const char* pPrefix)      GBL_NOEXCEPT;

GBL_EXPORT GblRefCount GblModule_unref        (GBL_SELF)                 GBL_NOEXCEPT;

GBL_EXPORT GBL_RESULT  GblModule_register     (GBL_SELF)                 GBL_NOEXCEPT;
GBL_EXPORT GBL_RESULT  GblModule_unregister   (GBL_SELF)                 GBL_NOEXCEPT;

GBL_EXPORT GBL_RESULT  GblModule_use          (GBL_SELF)                 GBL_NOEXCEPT;
GBL_EXPORT GBL_RESULT  GblModule_unuse        (GBL_SELF)                 GBL_NOEXCEPT;
GBL_EXPORT GblRefCount GblModule_useCount     (GBL_CSELF)                GBL_NOEXCEPT;

GBL_EXPORT GblBool     GblModule_isLoaded     (GBL_CSELF)                GBL_NOEXCEPT;


// ===== SubType API ===== (TODO)

GblType     GblModule_registerType  (GBL_SELF,
                                     GblType            parent,
                                     const char*        pName,
                                     const GblTypeInfo* pInfo,
                                     GblFlags           flags)      GBL_NOEXCEPT;

GblType     GblModule_typeFromName  (GBL_CSELF, const char* pName)  GBL_NOEXCEPT;
GblType     GblModule_typeFromIndex (GBL_CSELF, GblSize     index)  GBL_NOEXCEPT;
GblSize     GblModule_typeCount     (GBL_CSELF)                     GBL_NOEXCEPT;


// ====== IMPLEMENTATION =====
///\cond

#define GBL_REQUIRE_(...) \
    GBL_VA_OVERLOAD_CALL(GBL_REQUIRE_, GBL_VA_OVERLOAD_SUFFIXER_ARGC, __VA_ARGS__)

#define GBL_REQUIRE__1(type) \
    GBL_INSTANCE_CAST(GblModule_require(GblType_name(GBL_TYPEOF(type)), \
                                                     GBL_NULL, __FILE__, __FUNC__, \
                                                     __LINE__), type)
#define GBL_REQUIRE__2(type, name) \
    GBL_INSTANCE_CAST(GblModule_require(name, GBL_NULL, __FILE__, __FUNC__, \
                                        __LINE__), type)

#define GBL_REQUIRE__3(type, name, version) \
    GBL_INSTANCE_CAST(GblModule_require(name, version, __FILE__, __FUNC__, \
                                        __LINE__), type)

///\endcond

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // GIMBAL_MODULE_H
