// integration/kernel_bootstrap_test.cpp
#include "test_framework.hpp"

#include "unit/test_klist.hpp"
#include "unit/test_simulator_core.hpp" // 假设也有这个
#include "unit/test_zimg.hpp"
#include "unit/test_abi_frames.hpp"
#include "unit/test_message_system.hpp"
#include "unit/test_kernel_proxy.hpp"
#include "unit/test_task_factory.hpp"
#include "unit/test_task_creation_integrity.hpp"

// --- 基础引导与协议层 ---
K_TEST_CASE(unit_test_compact_pe_loading, "Compact PE Entry");
K_TEST_CASE(unit_test_zimg_header_integrity, "ZImg Protocol Integrity");

// --- 模拟器与硬件抽象层 (HAL) ---
K_TEST_CASE(unit_test_simulator_context_abi, "Simulator: Context ABI Integrity");
K_TEST_CASE(unit_test_simulator_memory_layout, "Simulator: Physical Memory Map");

// --- 架构与 ABI 契约 ---
K_TEST_CASE(unit_test_shadow_space_and_alignment_contract, "ABI: Shadow Space & Alignment");

// --- 核心领域模型 (Unit Contracts) ---
K_TEST_CASE(unit_test_klist_allocation, "[Step 1] Running Unit Contract: KList");
K_TEST_CASE(unit_test_task_factory_integrity, "[Step 2] Task Factory: Dependency Injection");
K_TEST_CASE(unit_test_message_system_integrity, "[Step 3] MessageBus: Pub-Sub Flow");

// --- 运行时与代理 (Service Layer) ---
K_TEST_CASE(unit_test_kernel_proxy_behavior, "KernelProxy: API Forwarding");

K_TEST_CASE(unit_test_task_creation_integrity, "Task Creation Integrity");

// --- 最终全链路启动测试 ---
// K_TEST_CASE(test_full_kernel_event_flow,               "Kernel: RootTask to Print Event Dispatch");