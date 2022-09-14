/*! \file
 *  \brief GblIVariant interface for GblVariant compatibility
 *  \ingroup interfaces
 */


#ifndef GIMBAL_IVARIANT_H
#define GIMBAL_IVARIANT_H

#include "gimbal_interface.h"

#define GBL_IVARIANT_TYPE                       (GBL_BUILTIN_TYPE(IVARIANT))
#define GBL_IVARIANT_STRUCT                     GblIVariant
#define GBL_IVARIANT_CLASS_STRUCT               GblIVariantClass
#define GBL_IVARIANT(instance)                  (GBL_INSTANCE_CAST_PREFIX       (instance,  GBL_IVARIANT))
#define GBL_IVARIANT_CHECK(instance)            (GBL_INSTANCE_CHECK_PREFIX      (instance,  GBL_IVARIANT))
#define GBL_IVARIANT_IFACE(klass)               (GBL_CLASS_CAST_PREFIX          (klass,     GBL_IVARIANT))
#define GBL_IVARIANT_IFACE_CHECK(klass)         (GBL_CLASS_CHECK_PREFIX         (klass,     GBL_IVARIANT))
#define GBL_IVARIANT_GET_IFACE(instance)        (GBL_INSTANCE_GET_CLASS_PREFIX (instance,  GBL_IVARIANT))

#define GBL_IVARIANT_VALUE_VAR_ARG_MAX          4

GBL_DECLS_BEGIN

GBL_DECLARE_FLAGS(GBL_IVARIANT_OP_FLAGS) {
    GBL_IVARIANT_OP_FLAG_RELOCATABLE           = 0x00001,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_DEFAULT     = 0x00002,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_COPY        = 0x00004,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_MOVE        = 0x00008,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_VALUE_COPY  = 0x00010,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_VALUE_MOVE  = 0x00020,
    GBL_IVARIANT_OP_FLAG_CONSTRUCT_MASK        = 0x0003e,
    GBL_IVARIANT_OP_FLAG_SET_COPY              = 0x00040,
    GBL_IVARIANT_OP_FLAG_SET_MOVE              = 0x00080,
    GBL_IVARIANT_OP_FLAG_SET_VALUE_COPY        = 0x00100,
    GBL_IVARIANT_OP_FLAG_SET_VALUE_MOVE        = 0x00200,
    GBL_IVARIANT_OP_FLAG_SET_MASK              = 0x003c0,
    GBL_IVARIANT_OP_FLAG_SET_INDEX_COPY        = 0x00400,
    GBL_IVARIANT_OP_FLAG_SET_INDEX_MOVE        = 0x00800,
    GBL_IVARIANT_OP_FLAG_SET_INDEX_MASK        = 0x00c00,
    GBL_IVARIANT_OP_FLAG_GET_VALUE_COPY        = 0x01000,
    GBL_IVARIANT_OP_FLAG_GET_VALUE_PEEK        = 0x02000,
    GBL_IVARIANT_OP_FLAG_GET_VALUE_MOVE        = 0x04000,
    GBL_IVARIANT_OP_FLAG_GET_MASK              = 0x07000,
    GBL_IVARIANT_OP_FLAG_GET_INDEX_COPY        = 0x08000,
    GBL_IVARIANT_OP_FLAG_GET_INDEX_MOVE        = 0x10000,
    GBL_IVARIANT_OP_FLAG_GET_INDEX_PEEK        = 0x20000,
    GBL_IVARIANT_OP_FLAG_GET_INDEX_MASK        = 0x38000,
    GBL_IVARIANT_OP_FLAG_VALUELESS_TYPE        = 0x40000
};

#define VARIANT     GblVariant* pVariant
#define CVARIANT    const VARIANT

typedef struct GblIVariantClassVTable {
    GBL_IVARIANT_OP_FLAGS   supportedOps;
    char                    pSetValueFmt[GBL_IVARIANT_VALUE_VAR_ARG_MAX];
    char                    pGetValueFmt[GBL_IVARIANT_VALUE_VAR_ARG_MAX];

    GBL_RESULT (*pFnConstruct)(VARIANT, GblSize argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op);
    GBL_RESULT (*pFnSet)      (VARIANT, GblSize argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op);
    GBL_RESULT (*pFnGet)      (VARIANT, GblSize argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op);
    GBL_RESULT (*pFnDestruct) (VARIANT);
    GBL_RESULT (*pFnCompare)  (CVARIANT, const GblVariant* pOther, GblInt* pResult);
    GBL_RESULT (*pFnSave)     (CVARIANT, GblStringBuffer* pString);
    GBL_RESULT (*pFnLoad)     (VARIANT,  const GblStringBuffer* pString);
} GblIVariantClassVTable;

GBL_INTERFACE_DERIVE(GblIVariant)
    const GblIVariantClassVTable*   pVTable;
GBL_INTERFACE_END

#define GBL_SELF    GblIVariantClass* pSelf
#define GBL_CSELF   const GBL_SELF

GBL_API GblIVariantClass_validate            (GBL_CSELF)                                        GBL_NOEXCEPT;
GBL_API GblIVariantClass_constructDefault    (GBL_CSELF, VARIANT)                               GBL_NOEXCEPT;
GBL_API GblIVariantClass_constructCopy       (GBL_CSELF, VARIANT,  const GblVariant* pOther)    GBL_NOEXCEPT;
GBL_API GblIVariantClass_constructMove       (GBL_CSELF, VARIANT,  GblVariant* pOther)          GBL_NOEXCEPT;
GBL_API GblIVariantClass_constructValueCopy  (GBL_CSELF, VARIANT,  va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_constructValueMove  (GBL_CSELF, VARIANT,  va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_setCopy             (GBL_CSELF, VARIANT,  const GblVariant* pOther)    GBL_NOEXCEPT;
GBL_API GblIVariantClass_setMove             (GBL_CSELF, VARIANT,  GblVariant* pOther)          GBL_NOEXCEPT;
GBL_API GblIVariantClass_setValueCopy        (GBL_CSELF, VARIANT,  va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_setValueMove        (GBL_CSELF, VARIANT,  va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_getValueCopy        (GBL_CSELF, CVARIANT, va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_getValuePeek        (GBL_CSELF, CVARIANT, va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_getValueMove        (GBL_CSELF, CVARIANT, va_list* pVarArgs)           GBL_NOEXCEPT;
GBL_API GblIVariantClass_destruct            (GBL_CSELF, VARIANT)                               GBL_NOEXCEPT;

GBL_API GblIVariantClass_compare             (GBL_CSELF,
                                              CVARIANT,
                                              const GblVariant* pOther,
                                              GblInt* pCmpResult)                               GBL_NOEXCEPT;

GBL_API GblIVariantClass_save                (GBL_CSELF,
                                              CVARIANT,
                                              GblStringBuffer* pString)                         GBL_NOEXCEPT;

GBL_API GblIVariantClass_load                (GBL_CSELF,
                                              VARIANT,
                                              const GblStringBuffer* pString)                   GBL_NOEXCEPT;

#undef CVARIANT
#undef VARIANT

#undef GBL_CSELF
#undef GBL_SELF

GBL_DECLS_END

#endif // GIMBAL_IVARIANT_H
