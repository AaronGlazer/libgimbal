#include <gimbal/meta/instances/gimbal_box.h>
#include "../types/gimbal_type_.h"

#define GBL_BOX_FIELD_KEY_DESTRUCTOR   "_dtor"
#define GBL_BOX_FIELD_KEY_USERDATA     "_ud"

GblQuark fieldKeyDtor_ = GBL_QUARK_INVALID;
GblQuark fieldKeyUd_   = GBL_QUARK_INVALID;

// ============== GblBoxClass ====================
GBL_EXPORT GblBoxClass* (GblBoxClass_createFloating)(GblType           derivedType,
                                                     size_t            totalSize,
                                                     void*             pUserdata,
                                                     GblArrayMapDtorFn pFnUdDtor)
{
    GblBoxClass* pClass = NULL;
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_TYPE(derivedType, GBL_BOX_TYPE);

    pClass = (GblBoxClass*)GblClass_createFloating(derivedType, totalSize);
    if(pUserdata) GBL_CTX_VERIFY_CALL(GblBoxClass_setUserdata(pClass, pUserdata));
    if(pFnUdDtor) GBL_CTX_VERIFY_CALL(GblBoxClass_setUserDestructor(pClass, pFnUdDtor));

    GBL_CTX_END_BLOCK();
    return pClass;
}

GBL_EXPORT GBL_RESULT (GblBoxClass_constructFloating)(GblBoxClass*      pSelf,
                                                      GblType           derivedType,
                                                      void*             pUserdata,
                                                      GblArrayMapDtorFn pFnUdDtor)
{
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_TYPE(derivedType, GBL_BOX_TYPE);

    GblClass_constructFloating(GBL_CLASS(pSelf), derivedType);
    if(pUserdata) GBL_CTX_VERIFY_CALL(GblBoxClass_setUserdata(pSelf, pUserdata));
    if(pFnUdDtor) GBL_CTX_VERIFY_CALL(GblBoxClass_setUserDestructor(pSelf, pFnUdDtor));

    GBL_CTX_END();
}

GBL_EXPORT void* GblBoxClass_userdata(const GblBoxClass* pSelf) {
    return (void*)GblArrayMap_getValue(&GBL_PRIV_REF(pSelf).pFields, fieldKeyUd_);
}

GBL_EXPORT GBL_RESULT GblBoxClass_setUserDestructor(GblBoxClass* pSelf, GblArrayMapDtorFn pFnDtor) {
    if(pFnDtor) {
        return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields,
                                       fieldKeyDtor_,
                                       (uintptr_t)pSelf,
                                       pFnDtor);
    } else {
        return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields,
                                 fieldKeyDtor_);
    }
}

GBL_EXPORT GBL_RESULT GblBoxClass_setUserdata(GblBoxClass* pSelf, void* pUserdata) {
    if(pUserdata) {
        return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields,
                                       fieldKeyUd_,
                                       (uintptr_t)pUserdata,
                                       NULL);
    } else {
        return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields,
                                 fieldKeyUd_);
    }
}

GBL_EXPORT uintptr_t GblBoxClass_field(const GblBoxClass* pSelf, GblQuark key) {
    return GblArrayMap_getValue(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT uintptr_t GblBoxClass_takeField(GblBoxClass* pSelf, GblQuark key) {
    uintptr_t value = 0;
    GblArrayMap_extractValue(&GBL_PRIV_REF(pSelf).pFields, key, &value);
    return value;
}

GBL_EXPORT GblBool GblBoxClass_clearField(GblBoxClass* pSelf, GblQuark key) {
    return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT GblBool GblBoxClass_hasField(const GblBoxClass* pSelf, GblQuark key) {
    return GblArrayMap_contains(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT GBL_RESULT GblBoxClass_setField(GblBoxClass*      pSelf,
                                           GblQuark          key,
                                           uintptr_t         ud,
                                           GblArrayMapDtorFn pFnDtor)
{
    return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields, key, ud, pFnDtor);
}

// ============== GblBox ==================
GBL_EXPORT GblBox* (GblBox_create)(GblType           derivedType,
                                   size_t            size,
                                   void*             pUserdata,
                                   GblArrayMapDtorFn pFnUdDtor,
                                   GblBoxClass*      pClass)
{
    GblBox* pBox = NULL;
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_TYPE(derivedType, GBL_BOX_TYPE);

    pBox = (GblBox*)GblInstance_create(derivedType,
                                          size,
                                          GBL_CLASS(pClass));

    if(pBox) {
        if(pUserdata)
            GBL_CTX_VERIFY_CALL(GblBox_setUserdata(pBox, pUserdata));
        if(pFnUdDtor)
            GBL_CTX_VERIFY_CALL(GblBox_setUserDestructor(pBox, pFnUdDtor));
    }

    GBL_CTX_END_BLOCK();
    return pBox;
}


GBL_EXPORT GBL_RESULT (GblBox_construct)(GblBox*           pSelf,
                                         GblType           derivedType,
                                         void*             pUserdata,
                                         GblArrayMapDtorFn pFnUdDtor,
                                         GblBoxClass*      pClass)
{
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_TYPE(derivedType, GBL_BOX_TYPE);

    GBL_CTX_VERIFY_CALL(GblInstance_construct((GblInstance*)pSelf,
                                              derivedType,
                                              GBL_CLASS(pClass)));
    GBL_PRIV_REF(pSelf).constructedInPlace = 1;

    if(pUserdata)
        GBL_CTX_VERIFY_CALL(GblBox_setUserdata(pSelf, pUserdata));
    if(pFnUdDtor)
        GBL_CTX_VERIFY_CALL(GblBox_setUserDestructor(pSelf, pFnUdDtor));

    GBL_CTX_END();
}


GBL_EXPORT GBL_RESULT GblBox_setUserDestructor(GblBox* pSelf, GblArrayMapDtorFn pFnDtor) {
    if(pFnDtor) {
        return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields,
                                       fieldKeyDtor_,
                                       (uintptr_t)pSelf,
                                       pFnDtor);
    } else {
        return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields,
                                 fieldKeyDtor_);
    }
}

GBL_EXPORT uintptr_t GblBox_field(const GblBox* pSelf, GblQuark key) {
    return GblArrayMap_getValue(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT uintptr_t GblBox_takeField(GblBox* pSelf, GblQuark key) {
    uintptr_t value = 0;
    GblArrayMap_extractValue(&GBL_PRIV_REF(pSelf).pFields, key, &value);
    return value;
}

GBL_EXPORT GblBool GblBox_clearField(GblBox* pSelf, GblQuark key) {
    return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT GblBool GblBox_hasField(const GblBox* pSelf, GblQuark key) {
    return GblArrayMap_contains(&GBL_PRIV_REF(pSelf).pFields, key);
}

GBL_EXPORT GBL_RESULT GblBox_setField(GblBox*           pSelf,
                                      GblQuark          key,
                                      uintptr_t         ud,
                                      GblArrayMapDtorFn pFnDtor)
{
    return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields, key, ud, pFnDtor);
}

GBL_EXPORT void* GblBox_userdata(const GblBox* pSelf) {
    return (void*)GblArrayMap_getValue(&GBL_PRIV_REF(pSelf).pFields, fieldKeyUd_);
}

GBL_EXPORT GBL_RESULT GblBox_setUserdata(GblBox* pSelf, void* pUserdata) {
    if(pUserdata) {
        return GblArrayMap_setUserdata(&GBL_PRIV_REF(pSelf).pFields,
                                       fieldKeyUd_,
                                       (uintptr_t)pUserdata,
                                       NULL);
    } else {
        return GblArrayMap_erase(&GBL_PRIV_REF(pSelf).pFields,
                                 fieldKeyUd_);
    }
}

GBL_EXPORT GblBox* GblBox_ref(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_POINTER(pSelf);

    if(pSelf) {
       GBL_ATOMIC_INT16_INC(GBL_PRIV_REF(pSelf).refCounter);
    }

    GBL_CTX_END_BLOCK();
    return pSelf;
}

GBL_EXPORT GblRefCount GblBox_unref(GblBox* pSelf) {
    GblRefCount count = 0;
    GBL_CTX_BEGIN(NULL);

    if(pSelf) {
        GBL_CTX_VERIFY_EXPRESSION(GBL_PRIV_REF(pSelf).refCounter);

       if((count = GBL_ATOMIC_INT16_DEC(GBL_PRIV_REF(pSelf).refCounter)-1) == 0) {
            GblBoxClass* pClass = GBL_BOX_GET_CLASS(pSelf);
            GBL_CTX_CALL(pClass->pFnDestructor(pSelf));

            if(!GBL_PRIV_REF(pSelf).constructedInPlace)
                GBL_CTX_CALL(GblInstance_destroy(&pSelf->base));
            else
                GBL_CTX_CALL(GblInstance_destruct(&pSelf->base));
        }
    }

    GBL_CTX_END_BLOCK();
    return count;
}

GBL_EXPORT GblRefCount GblBox_refCount(const GblBox* pSelf) {
    return GBL_ATOMIC_INT16_LOAD(GBL_PRIV_REF(pSelf).refCounter);
}

static GBL_RESULT GblBox_IVariant_construct_(GblVariant* pVariant, size_t argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op) {
    GBL_UNUSED(argc);
    GBL_CTX_BEGIN(NULL);

    if(op == GBL_IVARIANT_OP_FLAG_CONSTRUCT_DEFAULT) {
        pVariant->pBox = GblBox_create(pVariant->type);

    } else if(op == GBL_IVARIANT_OP_FLAG_CONSTRUCT_COPY ||
              op == GBL_IVARIANT_OP_FLAG_CONSTRUCT_VALUE_COPY)
    {
        pVariant->pBox = pArgs[0].pVoid?
                         GBL_REF(pArgs[0].pVoid) : NULL;
    } else {
        pVariant->pBox = pArgs[0].pVoid;
    }

    GBL_CTX_END();
}

static GBL_RESULT GblBox_IVariant_destruct_(GblVariant* pVariant) {
    GBL_CTX_BEGIN(NULL);

    GBL_UNREF(pVariant->pBox);

    GBL_CTX_END();
}

static GBL_RESULT GblBox_IVariant_compare_(const GblVariant* pVariant, const GblVariant* pOther, int* pResult) {
    GBL_CTX_BEGIN(NULL);

    *pResult = pVariant->pBox - pOther->pBox;

    GBL_CTX_END();
}

static GBL_RESULT GblBox_IVariant_get_(GblVariant* pSelf, size_t  argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op) {
    GBL_UNUSED(argc);

    GBL_CTX_BEGIN(NULL);

    *(GblBox**)pArgs[0].pVoid = pSelf->pBox;

    if(op == GBL_IVARIANT_OP_FLAG_GET_VALUE_COPY) {
        GBL_REF(pSelf->pBox);
    } else if(op == GBL_IVARIANT_OP_FLAG_GET_VALUE_MOVE) {
        GblVariant_invalidate(pSelf);
    }

    GBL_CTX_END();
}

static GBL_RESULT GblBox_IVariant_set_(GblVariant* pSelf, size_t  argc, GblVariant* pArgs, GBL_IVARIANT_OP_FLAGS op) {
    GBL_UNUSED(argc);
    GBL_CTX_BEGIN(NULL);

    pSelf->pBox = pArgs[0].pVoid;

    if(op == GBL_IVARIANT_OP_FLAG_SET_VALUE_COPY || op == GBL_IVARIANT_OP_FLAG_SET_COPY) {
        GBL_REF(pSelf->pBox);
    } else if(op == GBL_IVARIANT_OP_FLAG_SET_MOVE || op == GBL_IVARIANT_OP_FLAG_SET_VALUE_MOVE) {
        GblVariant_invalidate(&pArgs[0]);
    }

    GBL_CTX_END();
}

static GBL_RESULT GblBox_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_CALL(GblArrayMap_destroy(&GBL_PRIV_REF(pSelf).pFields));

    GBL_CTX_END();
}

static GBL_RESULT GblBox_init_(GblInstance* pInstance) {
    GBL_CTX_BEGIN(NULL);

    GBL_ATOMIC_INT16_INIT(GBL_PRIV_REF((GblBox*)pInstance).refCounter, 1);

    GBL_CTX_END();
}

static GBL_RESULT GblBoxClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

    if(!GblType_classRefCount(GBL_BOX_TYPE)) {
        fieldKeyDtor_ = GblQuark_fromStringStatic(GBL_BOX_FIELD_KEY_DESTRUCTOR);
        fieldKeyUd_   = GblQuark_fromStringStatic(GBL_BOX_FIELD_KEY_USERDATA);
    }

    static const GblIVariantVTable iVariantVTable = {
        .supportedOps = GBL_IVARIANT_OP_FLAG_CONSTRUCT_DEFAULT      |
                        GBL_IVARIANT_OP_FLAG_CONSTRUCT_COPY         |
                        GBL_IVARIANT_OP_FLAG_CONSTRUCT_MOVE         |
                        GBL_IVARIANT_OP_FLAG_CONSTRUCT_VALUE_COPY   |
                        GBL_IVARIANT_OP_FLAG_CONSTRUCT_VALUE_MOVE   |
                        GBL_IVARIANT_OP_FLAG_SET_VALUE_COPY         |
                        GBL_IVARIANT_OP_FLAG_SET_COPY               |
                        GBL_IVARIANT_OP_FLAG_SET_VALUE_MOVE         |
                        GBL_IVARIANT_OP_FLAG_SET_MOVE               |
                        GBL_IVARIANT_OP_FLAG_GET_VALUE_COPY         |
                        GBL_IVARIANT_OP_FLAG_GET_VALUE_MOVE         |
                        GBL_IVARIANT_OP_FLAG_GET_VALUE_PEEK,
        .pGetValueFmt = "p",
        .pSetValueFmt = "p",
        .pFnConstruct = GblBox_IVariant_construct_,
        .pFnDestruct  = GblBox_IVariant_destruct_,
        .pFnCompare   = GblBox_IVariant_compare_,
        .pFnGet       = GblBox_IVariant_get_,
        .pFnSet       = GblBox_IVariant_set_,
    };

    GBL_IVARIANT_CLASS(pClass)->pVTable       = &iVariantVTable;
    GBL_BOX_CLASS(pClass)     ->pFnDestructor = GblBox_destructor_;

    GBL_CTX_END();
}

static GBL_RESULT GblBoxClass_final_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

    GblBoxClass* pSelfClass = (GblBoxClass*)pClass;
    GBL_CTX_VERIFY_CALL(GblArrayMap_destroy(&GBL_PRIV_REF(pSelfClass).pFields));

    GBL_CTX_END();
}

GBL_EXPORT GblType GblBox_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaceMap[] = {
        {
            GBL_INVALID_TYPE,
            offsetof(GblBoxClass, GblIVariantImpl)
        }
    };

    static const GblTypeInfo typeInfo = {
        .classSize          = sizeof(GblBoxClass),
        .pFnClassInit       = GblBoxClass_init_,
        .pFnClassFinal      = GblBoxClass_final_,
        .instanceSize       = sizeof(GblBox),
        .pFnInstanceInit    = GblBox_init_,
        .interfaceCount     = 1,
        .pInterfaceImpls      = ifaceMap
    };

    if(type == GBL_INVALID_TYPE) GBL_UNLIKELY {
        ifaceMap[0].interfaceType = GBL_IVARIANT_TYPE;

        type = GblType_register(GblQuark_internStringStatic("GblBox"),
                                      GBL_INSTANCE_TYPE,
                                      &typeInfo,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }
    return type;
}
