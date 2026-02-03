#pragma once

#include <kernel/ITaskContextFactory.hpp>
#include <kernel/IObjectBuilder.hpp>

#include "MockTaskContext.hpp"

class MockTaskContextFactory : public ITaskContextFactory
{
public:
    ITaskContext *create_context() override
    {
        // 在不考虑 KObject 的情况下，直接 new 或用 builder->construct
        return new MockTaskContext();
    }

    void destroy_context(ITaskContext *ctx) override
    {
        delete ctx;
    }
};