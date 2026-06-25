
struct three_d_filter_t {
  char   name[16];
  double elem[2];
};

static three_d_filter_t base_filters[] = {
  {"Identity", { 1.0, 0.0 } },
  {"Echo", { 0.4, 0.0 } }
};
