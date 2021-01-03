#pragma once

#include <cstddef>

// page directory number
//  used in SetupIdentiryPageMap
//  1-page-directory -> 512 x 2MiB-page
const size_t kPageDirectoryCount = 64;

void SetupIdentityPageTable();