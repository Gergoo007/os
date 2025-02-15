#pragma once

void userspace_init();
void userexec(void (*entry)(), void* stack);
void userinit();
