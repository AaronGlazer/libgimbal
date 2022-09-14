#include <gimbal/meta/instances/gimbal_context.h>
#include "../types/gimbal_type_.h"

static GBL_RESULT GblContext_IAllocator_alloc_(GblIAllocator* pIAllocator, const GblStackFrame* pFrame, GblSize size, GblSize align, const char* pDbgStr, void** ppData) GBL_NOEXCEPT {
    GblContext* pParentCtx = GblContext_parentContext((GblContext*)pIAllocator);
    GBL_API_BEGIN(pParentCtx);

    // chain up
    if(GBL_API_CONTEXT() != (GblContext*)pIAllocator) {
        GBL_API_VERIFY_CALL(GblContext_memAlloc_(GBL_API_CONTEXT(), pFrame, size, align, pDbgStr, ppData));
    } else {
        GBL_API_VERIFY_ARG(size);
        GBL_API_VERIFY_ARG(align <= size && align >= 0);
        GBL_API_VERIFY_POINTER(ppData);
        //GBL_API_DEBUG("Malloc(Size: %" GBL_SIZE_FMT ", Align: %" GBL_SIZE_FMT ")", size, align);
        GBL_API_ERRNO_CLEAR();

        *ppData = GBL_ALIGNED_ALLOC(align, size);

        GBL_API_PUSH();
        GBL_API_PERROR("Malloc Failed");
        GBL_API_VERIFY(*ppData, GBL_RESULT_ERROR_MEM_ALLOC);
    #if 0
        GBL_API_DEBUG("%-20s: %20p", "Address", *ppData);
        GBL_API_DEBUG("%-20s: %20s", "Debug Marker", pDbgStr? pDbgStr : "NULL");
        GBL_API_DEBUG("%-20s: %20s", "Function", pFrame->sourceCurrent.pFunc);
        GBL_API_DEBUG("%-20s: %20"GBL_SIZE_FMT, "Line", pFrame->sourceCurrent.line);
        GBL_API_DEBUG("%-20s: %20s", "File", pFrame->sourceCurrent.pFile);
    #endif
        GBL_API_POP(1);
    }
    GBL_API_END();
}

static GBL_RESULT GblContext_IAllocator_realloc_(GblIAllocator* pIAllocator, const GblStackFrame* pFrame, void* pData, GblSize newSize, GblSize newAlign, void** ppNewData) GBL_NOEXCEPT {
    GBL_UNUSED(pFrame);

    GblContext* pParentCtx = GblContext_parentContext((GblContext*)pIAllocator);
    GBL_API_BEGIN(pParentCtx);
    // chain up
    if(GBL_API_CONTEXT() != (GblContext*)pIAllocator) {
        GBL_API_VERIFY_CALL(GblContext_memRealloc_(GBL_API_CONTEXT(), pFrame, pData, newSize, newAlign, ppNewData));
    } else {

        GBL_API_VERIFY_ARG(newSize);
        GBL_API_VERIFY_ARG(newAlign <= newSize && newAlign >= 0);
        GBL_API_VERIFY_POINTER(pData);
        GBL_API_VERIFY_POINTER(ppNewData);
        const uintptr_t ptrVal = (uintptr_t)pData;
        //GBL_API_DEBUG("Realloc(Size: %" GBL_SIZE_FMT ", Align: %" GBL_SIZE_FMT ") %p", newSize, newAlign, ptrVal);

        *ppNewData = GBL_ALIGNED_REALLOC(pData, newAlign, newSize);

        GBL_API_VERIFY(*ppNewData, GBL_RESULT_ERROR_MEM_REALLOC);
    #if 0
        GBL_API_PUSH();
        GBL_API_DEBUG("%-20s: %20p", "Address", *ppNewData);
        GBL_API_DEBUG("%-20s: %20s", "Function", pFrame->sourceCurrent.pFunc);
        GBL_API_DEBUG("%-20s: %20" GBL_SIZE_FMT, "Line", pFrame->sourceCurrent.line);
        GBL_API_DEBUG("%-20s: %20s", "File", pFrame->sourceCurrent.pFile);
        GBL_API_POP(1);
    #endif
    }
    GBL_API_END();
}

static GBL_RESULT GblContext_IAllocator_free_(GblIAllocator* pIAllocator, const GblStackFrame* pFrame, void* pData) GBL_NOEXCEPT {
    GBL_UNUSED(pFrame);

    GblContext* pParentCtx = GblContext_parentContext((GblContext*)pIAllocator);
    GBL_API_BEGIN(pParentCtx);
    // chain up
    if(GBL_API_CONTEXT() != (GblContext*)pIAllocator) {
        GBL_API_VERIFY_CALL(GblContext_memFree_(GBL_API_CONTEXT(), pFrame, pData));
    } else {

        const uintptr_t ptrVal = (uintptr_t)pData;

        GBL_ALIGNED_FREE(pData);

    #if 0
        GBL_API_DEBUG("Free(%p)", ptrVal);
        GBL_API_PUSH();
        GBL_API_DEBUG("%-20s: %20s", "Function", pFrame->sourceCurrent.pFunc);
        GBL_API_DEBUG("%-20s: %20" GBL_SIZE_FMT, "Line", pFrame->sourceCurrent.line);
        GBL_API_DEBUG("%-20s: %20s", "File", pFrame->sourceCurrent.pFile);
        GBL_API_POP(1);
    #endif
    }
    GBL_API_END();
}

static GBL_RESULT GblContext_ILogger_write_(GblILogger* pILogger, const GblStackFrame* pFrame, GBL_LOG_LEVEL level, const char* pFmt, va_list varArgs) GBL_NOEXCEPT {
    GblContext* pSelf = (GblContext*)(pILogger);
    GblContext* pCtx = (GblContext*)pILogger;

    if(!(pCtx->logFilter & level)) return GBL_RESULT_SUCCESS;

    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pFmt);
    GBL_API_VERIFY_ARG(level >= 0 /*&& level < GBL_LOG_LEVEL_COUNT*/); // or not to allow for user levels!

    char buffer[GBL_VA_SNPRINTF_BUFFER_SIZE] = { '\0' };
    char tabBuff[GBL_VA_SNPRINTF_BUFFER_SIZE];// = { '\t' };
    FILE* const pFile = (level >= GBL_LOG_LEVEL_ERROR)?
                stderr : stdout;
    const char* pPrefix = NULL;

    switch(level) {
    case GBL_LOG_LEVEL_WARNING: pPrefix = "! "; break;
    case GBL_LOG_LEVEL_ERROR:   pPrefix = "X "; break;
    case GBL_LOG_LEVEL_DEBUG:   pPrefix = "# "; break;
    case GBL_LOG_LEVEL_INFO:    pPrefix = "* "; break;
    default:                    pPrefix = "";   break;
    }

    //replace me later
    const int vsnprintfBytes = vsnprintf(buffer, sizeof(buffer), pFmt, varArgs);
    if(vsnprintfBytes > (int)sizeof(buffer)) {
        pPrefix = "T - "; //Truncated prefix!
        GBL_API_RECORD_SET(GBL_RESULT_TRUNCATED, "Log message truncated!");
    }

    //not per byte!
    if(pSelf) {
        for(unsigned t = 0; t < pSelf->logStackDepth*8; ++t) {
            tabBuff[t] = ' ';
        }
        tabBuff[pSelf->logStackDepth*8] = '\0';
    } else {
        tabBuff[0] = '\0';
    }

    switch(level) {
    case GBL_LOG_LEVEL_WARNING:
    case GBL_LOG_LEVEL_ERROR: {

        GBL_API_VERIFY((fprintf(pFile, "%s%s%s\n%s        @ %s(..): %s:%" GBL_SIZE_FMT"\n",
                            tabBuff, pPrefix, buffer, tabBuff,
                            pFrame->sourceCurrent.pFunc,
                            pFrame->sourceCurrent.pFile,
                            pFrame->sourceCurrent.line)
                    >= 0), GBL_RESULT_ERROR_FILE_WRITE);
        break;
    }
    default:
        GBL_API_VERIFY((fprintf(pFile, "%s%s%s\n",
                            tabBuff, pPrefix, buffer)
                    >= 0), GBL_RESULT_ERROR_FILE_WRITE);
        break;
    }

    GBL_API_VERIFY(fflush(pFile) == 0, GBL_RESULT_ERROR_FILE_WRITE);
    GBL_API_END();
}

static GBL_RESULT GblContext_ILogger_pop_(GblILogger* pILogger, const GblStackFrame* pFrame, uint32_t count) GBL_NOEXCEPT {
    GBL_UNUSED(pFrame);
    GblContext* pSelf = (GblContext*)pILogger;
    if(pSelf->logStackDepth < count)
        return GBL_RESULT_ERROR_UNDERFLOW;
    pSelf->logStackDepth -= count;
    return GBL_RESULT_SUCCESS;
}

static GBL_RESULT GblContext_ILogger_push_(GblILogger* pILogger, const GblStackFrame* pFrame) GBL_NOEXCEPT {
    GBL_UNUSED(pFrame);
    GblContext* pSelf = (GblContext*)pILogger;
    if(pSelf->logStackDepth + 1 <= pSelf->logStackDepth) return GBL_RESULT_ERROR_OVERFLOW;
    ++pSelf->logStackDepth;
    return GBL_RESULT_SUCCESS;
}

static GBL_RESULT GblContext_constructor_(GblObject* pSelf) GBL_NOEXCEPT {
    GBL_PRIV(pSelf->base).contextType = 1;
    ((GblContext*)pSelf)->logFilter = 0xffffffff;
    GBL_API_BEGIN(pSelf);

    GblObjectClass* pObjClass = GBL_OBJECT_CLASS(GblClass_weakRefDefault(GBL_OBJECT_TYPE));

    GBL_API_VERIFY_CALL(pObjClass->pFnConstructor(pSelf));

    GBL_API_END();
}

static GBL_RESULT GblContextClass_init_(GblContextClass* pClass, void* pData, GblContext* pCtx) GBL_NOEXCEPT {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);

    if(!GblType_classRefCount(GBL_CONTEXT_TYPE)) {
        //GBL_PROPERTY_TABLE_REGISTER(GBL_CONTEXT, pClass);
        GBL_PROPERTIES_REGISTER(GblContext);
    }

    pClass->GblIAllocatorImpl.pFnAlloc    = GblContext_IAllocator_alloc_;
    pClass->GblIAllocatorImpl.pFnRealloc  = GblContext_IAllocator_realloc_;
    pClass->GblIAllocatorImpl.pFnFree     = GblContext_IAllocator_free_;
    pClass->GblILoggerImpl.pFnWrite       = GblContext_ILogger_write_;
    pClass->GblILoggerImpl.pFnPush        = GblContext_ILogger_push_;
    pClass->GblILoggerImpl.pFnPop         = GblContext_ILogger_pop_;
    pClass->base.pFnConstructor                = GblContext_constructor_;
    GBL_API_END();
}


static GblContextClass defaultClass = {
    .GblIAllocatorImpl.pFnAlloc    = GblContext_IAllocator_alloc_,
    .GblIAllocatorImpl.pFnRealloc  = GblContext_IAllocator_realloc_,
    .GblIAllocatorImpl.pFnFree     = GblContext_IAllocator_free_,
    .GblILoggerImpl.pFnWrite       = GblContext_ILogger_write_,
    .GblILoggerImpl.pFnPush        = GblContext_ILogger_push_,
    .GblILoggerImpl.pFnPop         = GblContext_ILogger_pop_
};

static GblContext defaultCtx_ = {
    .base.base.pClass = (GblBoxClass*)&defaultClass,
    .base.base.private_.contextType = 1,
    .logFilter = 0xffffffff
};

static GblContext* globalCtx_ = &defaultCtx_;


void GblContext_setGlobal(GblContext* pCtx) GBL_NOEXCEPT {
    globalCtx_ = pCtx;
}

GBL_EXPORT GblContext* GblContext_global(void) GBL_NOEXCEPT {
    return globalCtx_;
}

GBL_EXPORT GblContext* GblContext_parentContext(const GblContext* pSelf) GBL_NOEXCEPT {
    return GblObject_findContext(GblObject_parent((GblObject*)pSelf));
}

GBL_EXPORT const GblCallRecord* GblContext_lastIssue(const GblContext* pSelf) GBL_NOEXCEPT {
    return pSelf? &pSelf->lastIssue : NULL;
}

GBL_API GblContext_setLastIssue(GblContext* pSelf, const GblCallRecord* pRecord) GBL_NOEXCEPT {
    //GBL_API_BEGIN(pSelf);
    if(pRecord) memcpy(&pSelf->lastIssue, pRecord, sizeof(GblCallRecord));
    else memset(&pSelf->lastIssue, 0, sizeof(GblCallRecord));
    //GBL_API_END();
    return GBL_RESULT_SUCCESS;
}

GBL_API GblContext_clearLastIssue(GblContext* pSelf) GBL_NOEXCEPT {
    return GblContext_setLastIssue(pSelf, NULL);
}

GBL_EXPORT GblBool GblContext_hasIssue(const GblContext* pSelf) GBL_NOEXCEPT {
    return pSelf? GBL_RESULT_ISSUE(pSelf->lastIssue.result) : GBL_FALSE;
}

GBL_EXPORT GblBool GblContext_hasError(const GblContext* pSelf) GBL_NOEXCEPT {
    return pSelf? GBL_RESULT_ERROR(pSelf->lastIssue.result) : GBL_FALSE;
}

GBL_EXPORT GBL_RESULT GblContext_lastIssueResult  (const GblContext* pSelf) GBL_NOEXCEPT {
    return pSelf? pSelf->lastIssue.result : GBL_RESULT_ERROR_INVALID_POINTER;
}

GBL_EXPORT const char*  GblContext_lastIssueMessage (const GblContext* pSelf) GBL_NOEXCEPT {
    return pSelf? pSelf->lastIssue.message : NULL;
}


GBL_API GblContext_memAlloc_     (GblContext* pSelf,
                                  const GblStackFrame*  pFrame,
                                  GblSize               size,
                                  GblSize               align,
                                  const char*           pDbgStr,
                                  void**                ppData)     GBL_NOEXCEPT
{
    return pSelf->pClass->GblIAllocatorImpl.pFnAlloc((GblIAllocator*)pSelf, pFrame, size, align, pDbgStr, ppData);
}
GBL_API GblContext_memRealloc_   (GblContext* pSelf,
                                  const GblStackFrame*  pFrame,
                                  void*                 pData,
                                  GblSize               newSize,
                                  GblSize               newAlign,
                                  void**                ppNewData)  GBL_NOEXCEPT
{
    return pSelf->pClass->GblIAllocatorImpl.pFnRealloc((GblIAllocator*)pSelf, pFrame, pData, newSize, newAlign, ppNewData);
}
GBL_API GblContext_memFree_      (GblContext* pSelf,
                                  const GblStackFrame*  pFrame,
                                  void*                 pData)      GBL_NOEXCEPT
{
    return !pData? GBL_RESULT_SUCCESS : pSelf->pClass->GblIAllocatorImpl.pFnFree((GblIAllocator*)pSelf, pFrame, pData);
}

GBL_API GblContext_logWrite_     (GblContext* pSelf,
                                  const GblStackFrame*  pFrame,
                                  GBL_LOG_LEVEL         level,
                                  const char*           pFmt,
                                  va_list               varArgs)    GBL_NOEXCEPT
{
    //GBL_API_BEGIN(pSelf);
    return pSelf->pClass->GblILoggerImpl.pFnWrite((GblILogger*)pSelf, pFrame, level, pFmt, varArgs);
    //GBL_API_END();
}
GBL_API GblContext_logPush_      (GblContext* pSelf,
                                  const GblStackFrame*  pFrame)     GBL_NOEXCEPT
{
    return pSelf->pClass->GblILoggerImpl.pFnPush((GblILogger*)pSelf, pFrame);
}
GBL_API GblContext_logPop_       (GblContext* pSelf,
                                  const GblStackFrame*  pFrame,
                                  uint32_t              count)      GBL_NOEXCEPT
{
    return pSelf->pClass->GblILoggerImpl.pFnPop((GblILogger*)pSelf, pFrame, count);
}
GBL_API GblContext_callRecordSet_(GblContext* pSelf,
                                  const GblStackFrame* pFrame,
                                  const GblCallRecord* pRecord)     GBL_NOEXCEPT
{
    GBL_UNUSED(pFrame);
    return GblContext_setLastIssue(pSelf, pRecord);
}

GBL_EXPORT void GblContext_setLogFilter(GblContext* pSelf, GblFlags mask) {
    pSelf->logFilter = mask;
}

GBL_EXPORT void GblContext_logBuildInfo(const GblContext* pSelf) {
    GBL_API_BEGIN(pSelf);

    GBL_API_INFO("Build Info");
    GBL_API_PUSH();

    GBL_API_INFO("Project Info");
    GBL_API_INFO("%-20s: %-100.100s", "Name", GBL_PROJECT_NAME);
    GBL_API_INFO("%-20s: %-100.100s", "Version", GBL_PROJECT_VERSION);
    GBL_API_INFO("%-20s: %-100.100s", "URL", GBL_PROJECT_URL);
    GBL_API_INFO("%-20s: %-100.100s", "Description", GBL_PROJECT_DESCRIPTION);
    GBL_API_POP(1);

    GBL_API_INFO("Build Info");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Timestamp", GBL_BUILD_TIMESTAMP);
    GBL_API_INFO("%-20s: %-100.100s", "Type", GBL_BUILD_TYPE);
    GBL_API_INFO("%-20s: %-100.100s", "Config", GBL_BUILD_CONFIG);
    GBL_API_POP(1);

    GBL_API_INFO("CI Info");
    GBL_API_PUSH();
    #if GBL_BUILD_CI
    GBL_API_INFO("%-20s: %-100.100s", "Project", GBL_BUILD_CI_PROJECT_TITLE);

    GBL_API_INFO("Source Control Commit Info");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Title", GBL_BUILD_CI_COMMIT_TITLE);
    GBL_API_INFO("%-20s: %-100.100s", "Timestamp", GBL_BUILD_CI_COMMIT_TIMESTAMP);
    GBL_API_INFO("%-20s: %-100.100s", "SHA", GBL_BUILD_CI_COMMIT_SHA);
    GBL_API_INFO("%-20s: %-100.100s", "Tag", GBL_BUILD_CI_COMMIT_TAG);
    GBL_API_INFO("%-20s: %-100.100s", "Ref", GBL_BUILD_CI_COMMIT_BRANCH);
    GBL_API_POP(1);

    GBL_API_INFO("Job Info");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "ID", GBL_BUILD_CI_JOB_ID);
    GBL_API_INFO("%-20s: %-100.100s", "Name", GBL_BUILD_CI_JOB_NAME);
    GBL_API_INFO("%-20s: %-100.100s", "Stage", GBL_BUILD_CI_JOB_STAGE);
    GBL_API_POP(1);

    GBL_API_INFO("User Info");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Login", GBL_BUILD_CI_USER_LOGIN);
    GBL_API_INFO("%-20s: %-100.100s", "Name", GBL_BUILD_CI_USER_NAME);
    GBL_API_INFO("%-20s: %-100.100s", "Email Address", GBL_BUILD_CI_USER_EMAIL);
    GBL_API_POP(1);

    GBL_API_INFO("Build Node Info");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Description", GBL_BUILD_CI_RUNNER_DESCRIPTION);
    GBL_API_INFO("%-20s: %-100.100s", "Tags", GBL_BUILD_CI_RUNNER_TAGS);
    GBL_API_INFO("%-20s: %-100.100s", "Architecture", GBL_BUILD_CI_RUNNER_ARCH);
    GBL_API_POP(1);
    #else
    GBL_API_WARN("UNOFFICIAL LOCAL BUILD !");
    #endif
    GBL_API_POP(1);

    GBL_API_INFO("Compiler Info");
    GBL_API_PUSH();
    GBL_API_INFO("C");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "ID", GBL_BUILD_C_COMPILER_ID);
    GBL_API_INFO("%-20s: %-100.100s", "Version", GBL_BUILD_C_COMPILER_VERSION);
    GBL_API_INFO("%-20s: %-100.100s", "Target", GBL_BUILD_C_COMPILER_TARGET);
    GBL_API_INFO("%-20s: %-100.100s", "Toolchain", GBL_BUILD_C_COMPILER_TOOLCHAIN);
    GBL_API_INFO("%-20s: %-100.100s", "Language Standard", GBL_BUILD_C_STANDARD);
    GBL_API_INFO("%-20s: %-100.100s", "Language Extensions", GBL_BUILD_C_EXTENSIONS);
    GBL_API_POP(1);

    GBL_API_INFO("C++");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "ID", GBL_BUILD_CPP_COMPILER_ID);
    GBL_API_INFO("%-20s: %-100.100s", "Version", GBL_BUILD_CPP_COMPILER_VERSION);
    GBL_API_INFO("%-20s: %-100.100s", "Target", GBL_BUILD_CPP_COMPILER_TARGET);
    GBL_API_INFO("%-20s: %-100.100s", "Toolchain", GBL_BUILD_CPP_COMPILER_TOOLCHAIN);
    GBL_API_INFO("%-20s: %-100.100s", "Language Standard", GBL_BUILD_CPP_STANDARD);
    GBL_API_INFO("%-20s: %-100.100s", "Language Extensions", GBL_BUILD_CPP_EXTENSIONS);
    GBL_API_POP(1);
    GBL_API_POP(1);

    GBL_API_INFO("Environment Info");
    GBL_API_PUSH();
    GBL_API_INFO("Host");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Host Name", GBL_BUILD_HOST_NAME);
    //GBL_API_VERBOSE("%-20s: %-100.100s", "Domain Name", GBL_BUILD_HOST_DOMAIN_NAME);
    GBL_API_INFO("%-20s: %-100.100s", "Operating System", GBL_BUILD_HOST_OS);
    GBL_API_INFO("%-20s: %-100.100s", "Architecture", GBL_BUILD_HOST_ARCH);
    GBL_API_INFO("Processor");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Family", GBL_BUILD_HOST_PROCESSOR_NAME);
    GBL_API_INFO("%-20s: %-100.100s", "Description", GBL_BUILD_HOST_PROCESSOR_DESCRIPTION);
    GBL_API_INFO("%-20s: %-100.u", "Physical Cores", GBL_BUILD_HOST_PROCESSOR_CORES_PHYSICAL);
    GBL_API_INFO("%-20s: %-100.u", "Logical Cores", GBL_BUILD_HOST_PROCESSOR_CORES_LOGICAL);
    GBL_API_POP(1);
    GBL_API_INFO("Physical Memory");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100u", "Total (MB)", GBL_BUILD_HOST_MEMORY_PHYSICAL_TOTAL);
    GBL_API_INFO("%-20s: %-100u", "Available (MB)", GBL_BUILD_HOST_MEMORY_PHYSICAL_AVAILABLE);
    GBL_API_POP(1);
    GBL_API_POP(1);
    GBL_API_INFO("Target");
    GBL_API_PUSH();
    GBL_API_INFO("%-20s: %-100.100s", "Operating System", GBL_BUILD_TARGET_OS);
    GBL_API_INFO("%-20s: %-100.100s", "Architecture", GBL_BUILD_TARGET_ARCH);
    GBL_API_INFO("%-20s: %-100u", "Word Size", GBL_PTR_SIZE * 8);
    GBL_API_INFO("%-20s: %-100.100s", "Endianness", GBL_BIG_ENDIAN? "Big" : "Little");
    GBL_API_POP(2);

    GBL_API_END_BLOCK();
}
GBL_EXPORT GblType GblContext_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[2] = {
        {
            .classOffset   = offsetof(GblContextClass, GblIAllocatorImpl)
        }, {
            .classOffset   = offsetof(GblContextClass, GblILoggerImpl)
        }
    };

    static GblTypeInfo info = {
        .pFnClassInit     = (GblTypeClassInitializeFn)GblContextClass_init_,
        .classSize        = sizeof(GblContextClass),
        .instanceSize     = sizeof(GblContext),
        .interfaceCount   = 2,
        .pInterfaceMap    = ifaceEntries
    };


    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);

        ifaceEntries[0].interfaceType = GBL_IALLOCATOR_TYPE;
        ifaceEntries[1].interfaceType = GBL_ILOGGER_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("GblContext"),
                                      GBL_OBJECT_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
    }
    return type;
}

