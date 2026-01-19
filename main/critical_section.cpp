#include "critical_section.h"

// Инициализация статического мьютекса для CriticalSection
portMUX_TYPE CriticalSection::mux_ = portMUX_INITIALIZER_UNLOCKED;

// Инициализация статического мьютекса для CriticalSectionISR
portMUX_TYPE CriticalSectionISR::mux_ = portMUX_INITIALIZER_UNLOCKED;