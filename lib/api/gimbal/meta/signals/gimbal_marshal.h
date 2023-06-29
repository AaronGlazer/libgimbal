/*! \file
 *  \brief Builtin closure marshals and generator macros
 *  \ingroup signals
 *
 *  \author Falco Girgis
 */

#ifndef GIMBAL_MARSHAL_H
#define GIMBAL_MARSHAL_H

#include "../../core/gimbal_typedefs.h"
#include "../types/gimbal_type.h"
#include <stdarg.h>

#define GBL_DECL_CCLOSURE_MARSHAL_VOID__(postFix) \
    GBL_EXPORT GBL_RESULT GblMarshal_CClosure_VOID__##postFix(GblClosure* pClosure,     \
                                                              GblVariant* pRetValue,    \
                                                              size_t      argCount,     \
                                                              GblVariant* pArgs,        \
                                                              GblPtr      pMarshalData) GBL_NOEXCEPT

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(GblClosure);
GBL_FORWARD_DECLARE_STRUCT(GblVariant);

/*! Function signature for a generic marshal function
 *  \ingroup signals
 *
 *  Marshal functions form the basis of the closure system,
 *  and are used by all GblClosure instances. They are the primary
 *  entry point for generic function invocation, and are the glue
 *  between a generic GblClosure and an actual language-dependent
 *  real function call.
 *
 *  They take a generic list of arguments, marshal them into an
 *  actual function (which is typically stored with GblClosure),
 *  then demarshal out the return type.
 */
typedef GBL_RESULT (*GblMarshalFn)  (GblClosure*        pClosure,
                                     GblVariant*        pRetValue,
                                     size_t             argCount,
                                     GblVariant*        pArgs,
                                     GblPtr             pMarshalData);

typedef GBL_RESULT (*GblMarshalVaFn)(GblClosure*        pClosure,
                                     GblVariant*        pRetValue,
                                     void*              pInstance,
                                     va_list            args,
                                     void*              pMarshalData,
                                     size_t             argCount,
                                     const GblType*     pParamTypes);

GBL_EXPORT GBL_RESULT GblMarshal_CClosure_VOID__VOID (GblClosure*        pClosure,
                                                      GblVariant*        pRetValue,
                                                      size_t             argCount,
                                                      GblVariant*        pArgs,
                                                      GblPtr             pMarshalData)   GBL_NOEXCEPT;

GBL_EXPORT GBL_RESULT GblMarshal_ClassClosureMeta   (GblClosure*        pClosure,
                                                     GblVariant*        pRetValue,
                                                     size_t             argCount,
                                                     GblVariant*        pArgs,
                                                     GblPtr             pMarshalData)   GBL_NOEXCEPT;

GBL_EXPORT GBL_RESULT GblMarshal_SignalForwarder    (GblClosure*        pClosure,
                                                     GblVariant*        pRetValue,
                                                     size_t             argCount,
                                                     GblVariant*        pArgs,
                                                     GblPtr             pMarshalData)   GBL_NOEXCEPT;

GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_BOOL);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_CHAR);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT8);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT16);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_INT16);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT32);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_INT32);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT64);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_INT64);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_FLOAT);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_DOUBLE);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_STRING);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_ENUM);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_FLAGS);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_POINTER);
GBL_DECL_CCLOSURE_MARSHAL_VOID__(INSTANCE_BOXED);
// add event type


#define GBL_DEFINE_CCLOSURE_MARSHAL_VOID__(paramsPostfix, paramCount, paramList, argList)       \
    GBL_EXPORT GBL_RESULT GblMarshal_CClosure_VOID__##paramsPostfix(GblClosure* pClosure,       \
                                                                    GblVariant* pRetValue,      \
                                                                    size_t      argCount,       \
                                                                    GblVariant* pArgs,          \
                                                                    GblPtr      pMarshalData)   \
    GBL_NOEXCEPT {                                                                              \
        GBL_UNUSED(pRetValue, argCount);                                                        \
        typedef void (*CFunction)(GBL_EVAL paramList);                                          \
        GBL_ASSERT(argCount >= paramCount);                                                     \
        GBL_ASSERT(!pRetValue || GblVariant_typeOf(pRetValue) == GBL_INVALID_TYPE);             \
        GblCClosure* pCClosure = (GblCClosure*)pClosure;                                        \
        CFunction pFnPtr = (CFunction)((pMarshalData.pFunc)?                                    \
                            pMarshalData.pFunc : GBL_PRIV_REF(pCClosure).pFnCallback);          \
        pFnPtr(GBL_EVAL argList);                                                               \
        return GBL_RESULT_SUCCESS;                                                              \
    }

GBL_DECLS_END


#endif // GIMBAL_MARSHAL_H
