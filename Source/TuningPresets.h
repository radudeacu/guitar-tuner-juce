#pragma once

#include "Tuning.h"

#include <vector>

namespace TuningPresets
{
    /** The tuning the app starts in. */
    Tuning standard();

    /** Every built-in preset, in the order they should be offered in the UI. */
    std::vector<Tuning> all();
}
