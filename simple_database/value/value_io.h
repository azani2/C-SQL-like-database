#pragma once
#ifndef VALUE_IO_HEADER
#define VALUE_IO_HEADER
#include "value.h"

std::ostream& operator<<(std::ostream&, const value&);

#endif // !VALUE_IO_HEADER
