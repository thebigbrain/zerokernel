#pragma once

#include "test_framework.hpp"
#include "kernel/Kernel.hpp"
#include "common/TaskTypes.hpp"

#include "mock/mock.hpp"

void unit_test_task_creation_integrity()
{
    Kernel *kernel = create_kernel(32 * 1024);

    kernel->setup_infrastructure();

    auto lifecycle = kernel->get_task_lifecycle();
    K_ASSERT(lifecycle != nullptr, "Task lifecycle is null.");

    auto builder = kernel->get_object_builder();
    auto strategy = kernel->get_scheduling_strategy();

    TaskExecutionInfo exec{};
    TaskResourceConfig res{};
    res.stack = builder->construct<KStackBuffer>(kernel->get_runtime_heap(), 1024);
    ITaskControlBlock *tcb = lifecycle->spawn_task(exec, res);
    K_ASSERT(tcb != nullptr, "Task is null.");

    strategy->make_task_ready(tcb);
    K_ASSERT(tcb->get_state() == TaskState::READY, "Task state is not READY.");

    std::cout << "[PASS] create_kernel_task logic is sound." << std::endl;
}