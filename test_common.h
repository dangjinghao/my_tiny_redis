// When testing,every function in modules should be visible for outer modules. Because the gtest main is located in a single module
#ifdef TEST
#define COM_INNER_DECL
#else
#define COM_INNER_DECL static
#endif