#pragma once
#include <string>
#include <variant>
#include <vector>
#include "date.h"
#include "datetime.h"

using value = std::variant<int, double, std::string, date, datetime>;
using row = std::vector<value>;