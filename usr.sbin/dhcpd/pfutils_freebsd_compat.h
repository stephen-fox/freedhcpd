// unveil is an OpenBSD thing, this is just a placeholder.
#ifdef unveil
#error "unveil already defined"
#else
static inline int unveil(const char *path, const char *permissions) {
	(void)path;
	(void)permissions;
	return 0;
}
#endif