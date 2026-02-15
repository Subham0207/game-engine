//
// Created by subha on 14-02-2026.
//

#include "../Headers/UI/Shared/Utils.hpp"

namespace UI
{
    int Utils::toUiIndex(int dataTypeIndex)
    {
        // 0th is reserved for None, From 1st index your data starts.
        return dataTypeIndex + 1;
    }

    int Utils::toDataTypeIndex(int UiIndex)
    {
        // None will be 0th index so don't call this function.
        return UiIndex - 1;
    }
}