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
    KernelInspector inspector(kernel);

    kernel->setup_infrastructure();

    auto lifecycle = inspector.lifecycle();
    K_ASSERT(lifecycle != nullptr, "Task lifecycle is null.");

    auto builder = inspector.builder();
    auto strategy = inspector.strategy();

    TaskExecutionInfo exec{};
    TaskResourceConfig res{};
    res.stack = builder->construct<KStackBuffer>(inspector.heap(), 1024);
    ITaskControlBlock *tcb = lifecycle->spawn_task(exec, res);
    K_ASSERT(tcb != nullptr, "Task is null.");

    strategy->make_task_ready(tcb);
    K_ASSERT(tcb->get_state() == TaskState::READY, "Task state is not READY.");

    std::cout << "[PASS] create_kernel_task logic is sound." << std::endl;
}