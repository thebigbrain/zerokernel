#pragma once

#include "ITaskControlBlockFactory.hpp"
#include "ObjectFactory.hpp"

class SimpleTaskFactory : public ITaskControlBlockFactory
{
private:
    ObjectFactory *_obj_factory;
    void *_default_exit_router; // 构造时存入

public:
    SimpleTaskFactory(ObjectFactory *f, void *exit_router)
        : _obj_factory(f),
          _default_exit_router(exit_router) {}

    ITaskControlBlock *create_tcb(
        uint32_t id,
        ITaskContext *ctx,
        const TaskExecutionInfo &exec_info,
        const TaskResourceConfig &res_config) override;
};