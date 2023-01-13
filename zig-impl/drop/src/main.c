#include <stdio.h>

int main(void) {
	int ret = someipc(20,10);
	printf("hello %d\n", ret);

	void *somestructure = NULL;
	some_struct_bidouillage_init(&somestructure);
	int value = some_struct_bidouillage_update(&somestructure);
	printf("value %d\n", value);

	return 0;
}
