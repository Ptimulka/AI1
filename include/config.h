/* Name of package */
/* #undef PACKAGE */

/* Version number of package */
#define VERSION "2.2.0"

#ifdef _WIN64
#define inttype long long
#define FANNINTIO "%lld"
#define FANNUINTIO "%llu"
#else
#define inttype int
#define FANNINTIO "%d"
#define FANNUINTIO "%u"
#endif

/* Define for the x86_64 CPU famyly */
/* #undef X86_64 */
