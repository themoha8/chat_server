#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"
#include "pool.h"

enum { default_alignment = 8, max_name_len = 16 };

typedef struct student_t {
	char name[max_name_len];
	int age;
	int class;
} student;

void _start(void)
{
	byte backing_buffer[1024];
	pool p;
	student *a, *b;

	pool_init(&p, backing_buffer, 1024, sizeof(student), default_alignment);

	a = pool_get(&p);
	b = pool_get(&p);

	c_strncpy(a->name, "Alex", max_name_len);
	a->age = 23;
	a->class = 'a';

	fmt_fprintf(stdout, "name: %s\nage: %d\nclass: %d\n", a->name, a->age, a->class);

	c_strncpy(b->name, "Anna", max_name_len);
	b->age = 21;
	b->class = 'a';

	fmt_fprintf(stdout, "name: %s\nage: %d\nclass: %d\n", b->name, b->age, b->class);

	pool_put(&p, a);
	pool_put(&p, b);

	sys_exit(0);
}
