#pragma once

#define _UCI true

#ifdef _TEST
#undef _UCI
#define _UCI false
#endif

#ifdef _BENCH
#undef _UCI
#define _UCI false
#endif