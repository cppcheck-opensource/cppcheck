
// Test library configuration for cairo.cfg
//
// Usage:
// $ cppcheck --check-library --library=cairo --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/cairo.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <cairo.h>

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 2, 3)
#endif

void validCode(cairo_surface_t *target)
{
    cairo_t * cairo1 = cairo_create(target);
    cairo_move_to(cairo1, 1.0, 2.0);
    cairo_line_to(cairo1, 5.0, 6.0);
    cairo_destroy(cairo1);
}

void ignoredReturnValue(cairo_surface_t *target)
{
    // cppcheck-suppress ignoredReturnValue
    cairo_create(target);
    // cppcheck-suppress ignoredReturnValue
    cairo_status_to_string(CAIRO_STATUS_READ_ERROR);
}
