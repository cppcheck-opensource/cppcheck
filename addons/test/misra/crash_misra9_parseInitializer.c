union {
	struct {
		uint8_t a;
		uint8_t b;
	} a;
} bar;

struct foo {
	uint8_t a;
	union bar w;
	uint8_t b;
};

struct foo asdf = {
	0,
	{{0,0}},
	1
};
