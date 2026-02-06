#pragma once

#include "test_framework.hpp"
#include "kernel/Kernel.hpp"
#include "inspect/KernelInspector.hpp"
#include "common/TaskTypes.hpp"

#include "mock/mock.hpp"

void unit_test_task_creation_integrity()
{
    Mock mock(32 * 1024);
    Kernel *kernel = mock.kernel();

    KernelInspector ki(kernel);

    K_T_ASSERT(ki.hooks() != nullptr, "Platform hooks are null.");
    K_T_ASSERT(ki.context_factory() != nullptr, "Task Context Factory is null.");

    kernel->setup_infrastructure();

    auto lifecycle = ki.lifecycle();
    K_T_ASSERT(lifecycle != nullptr, "Task lifecycle is null.");

    auto builder = ki.builder();
    auto strategy = ki.strategy();

    TaskExecutionInfo exec{};
    TaskResourceConfig res{};
    res.stack = builder->construct<KStackBuffer>(ki.heap(), 1024);
    ITaskControlBlock *tcb = lifecycle->spawn_task(exec, res);
    K_T_ASSERT(tcb != nullptr, "Task is null.");

    strategy->make_task_ready(tcb);
    K_T_ASSERT(tcb->get_state() == TaskState::READY, "Task state is not READY.");

    std::cout << "[PASS] create_kernel_task logic is sound." << std::endl;
}