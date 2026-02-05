#pragma once

#include <kernel/ITaskContext.hpp>
#include "MockTaskContext.hpp"

/**
 * @brief 专门用于验证任务上下文状态的检查员
 */
class ContextInspector
{
private:
    MockTaskContext *_m_ctx;

public:
    explicit ContextInspector(ITaskContext *ctx)
        : _m_ctx(static_cast<MockTaskContext *>(ctx)) {}

    // 仅仅暴露数据，不包含 K_ASSERT
    void *entry() const { return reinterpret_cast<void *>(_m_ctx->_entry); }
    uintptr_t arg(size_t i) const { return _m_ctx->_args[i]; }
    bool was_executed() const { return _m_ctx->_has_executed; }
    uint32_t jump_count() const { return _m_ctx->_jump_count; }
};