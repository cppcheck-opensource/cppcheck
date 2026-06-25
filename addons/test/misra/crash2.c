
// #11793

typedef struct pfmlib_pmu {
    int flags ;
    int (*get_event_encoding[10])(void* this, pfmlib_event_desc_t* e);
} pfmlib_pmu_t ;

pfmlib_pmu_t sparc_ultra3_support = { .flags = 0 };
