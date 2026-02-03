#pragma once

#include "ITaskControlBlockFactory.hpp"
#include "IObjectBuilder.hpp" // 替换为 Builder
#include "ITaskContextFactory.hpp"
#include "IIdGenerator.hpp"
#include "BitmapIdGenerator.hpp"

class SimpleTaskFactory : public ITaskControlBlockFactory
{
private:
    IObjectBuilder *_builder; // 核心构建器
    ITaskContextFactory *_context_factory;
    void *_default_exit_router;

    BitmapIdGenerator<64> *_id_gen; // 建议加上下划线保持风格一致

public:
    SimpleTaskFactory(IObjectBuilder *b, ITaskContextFactory *context_factory, void *exit_router)
        : _builder(b),
          _context_factory(context_factory),
          _default_exit_router(exit_router)
    {
    }

    ITaskControlBlock *create_tcb(
        const TaskExecutionInfo &exec_info,
        const TaskResourceConfig &res_config) override;
};