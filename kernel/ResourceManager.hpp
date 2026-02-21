#pragma once
#include "KList.hpp"
#include <common/Resource.hpp>
#include "KMap.hpp"

class ResourceManager
{
private:
    KMap<const char *, HardwareResource, 32> _resources;

public:
    void register_hw(const char *name, uintptr_t base, size_t size)
    {
        HardwareResource res = {base, size};
        _resources.insert(name, res);
    }

    HardwareResource *query(const char *name)
    {
        return _resources.find(name);
    }
};