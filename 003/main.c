#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char** argv, char** env)
{
	void *dl_handle;
	void (*func)();
	char *error;

	dl_handle = dlopen( "./libHello.so", RTLD_LAZY );
	if (!dl_handle ) {
		perror("dlopen");
		return -1;
	} 

	func = dlsym(dl_handle,"my_hello_world");
	if (!func) {
		perror("dlsym");
		return -1;
	}

	(*func)();

	dlclose(dl_handle);

	return 0;
}
