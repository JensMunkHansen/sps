// TODO: Strace for Windows

// SymFromAddr(), CaptureBackTrace

// use __declspec(noinline)  to know how many stack frames to skip


SymInitialize(process, NULL, TRUE);
SymSetOptions(SYMOPT_LOAD_LINES);

// Enable SEH exception style


On Windows, unhandled C++ exception automatically generates SEH exception. SEH __except block allows to attach a filter that accepts _EXCEPTION_POINTERS structure as a parameter, which contains the pointer to the processor's context record in the moment exception was thrown. Passing this pointer to StackWalk64 function gives the stack trace in the moment of exception. So, this problem can be solved by using SEH-style exception handling instead of C++ style.
