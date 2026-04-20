
// Test library configuration for gtk.cfg
//
// Usage:
// $ cppcheck --check-library --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr --library=gtk test/cfg/gtk.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <stdio.h>
#include <glib.h>
#include <glib/gtypes.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>


#if GLIB_CHECK_VERSION(2, 3, 4)
#endif
#if GTK_CHECK_VERSION(4, 5, 6)
#endif
#if ATK_CHECK_VERSION(2, 3, 6)
#endif

void validCode(int argInt, GHashTableIter * hash_table_iter, GHashTable * hash_table)
{
    g_assert_cmpint(4 + 1, >=, 5);
    g_assert_cmpstr("test", ==, "test");

    // if G_UNLIKELY is not defined this results in a syntax error
    if G_UNLIKELY(argInt == 1) {} else if (G_UNLIKELY(argInt == 2)) {}

    if G_LIKELY(argInt == 0) {} else if (G_LIKELY(argInt == -1)) {}

    printf("%s", _("test"));
    printf("%s", Q_("a|test"));
    printf("%s", N_("test"));

    gpointer gpt = g_malloc(4);
    printf("%p", gpt);
    g_free(gpt);
    // cppcheck-suppress deallocuse
    g_assert(gpt);
    if (!gpt) {
        // cppcheck-suppress checkLibraryNoReturn
        g_assert_not_reached();
    }
    // cppcheck-suppress constVariablePointer
    gpointer p = GINT_TO_POINTER(1);
    int i = GPOINTER_TO_INT(p);
    // cppcheck-suppress knownConditionTrueFalse
    if (i == 1) {}

    g_print("test");
    g_print("%d", 1);
    g_printerr("err");

    GString * pGStr1 = g_string_new("test");
    g_string_append(pGStr1, "a");
    g_string_free(pGStr1, TRUE);

    gchar * pGchar1 = g_strconcat("a", "b", NULL);
    printf("%s", pGchar1);
    g_free(pGchar1);

    // cppcheck-suppress unusedAllocatedMemory
    GError * pGerror = g_error_new(1, -2, "a %d", 1);
    g_error_free(pGerror);

    static gsize init_val = 0;
    if (g_once_init_enter(&init_val)) {
        gsize result_val = 1;
        g_once_init_leave(&init_val, result_val);
    }

    g_hash_table_iter_replace(hash_table_iter, g_strdup("test"));
    g_hash_table_insert(hash_table, g_strdup("key"), g_strdup("value"));
    g_hash_table_replace(hash_table, g_strdup("key"), g_strdup("value"));

    // NULL is handled graciously
    char* str = g_strdup(NULL);
    if (g_strcmp0(str, NULL) || g_strcmp0(NULL, str))
        // cppcheck-suppress valueFlowBailout // TODO: caused by <pure/>?
        printf("%s", str);
    g_free(str);
}

void g_malloc_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc(8);

    gconstpointer gpt = g_malloc(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc0_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc0(8);

    gconstpointer gpt = g_malloc0(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc_n_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc_n(8, 1);

    gconstpointer gpt = g_malloc_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc0_n_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc0_n(8, 1);

    gconstpointer gpt = g_malloc0_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc(8);

    gconstpointer gpt = g_try_malloc(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc0_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc0(8);

    gconstpointer gpt = g_try_malloc0(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc_n_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc_n(8, 1);

    gconstpointer gpt = g_try_malloc_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc0_n_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc0_n(8, 1);

    gconstpointer gpt = g_try_malloc0_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_realloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_realloc(NULL, 1);

    gpointer gpt = g_malloc(1);
    gpt = g_realloc(gpt, 2); // No memleakOnRealloc since g_realloc aborts if it fails
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_realloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_realloc_n(NULL, 1, 2);

    gpointer gpt = g_malloc_n(1, 2);
    gpt = g_realloc_n(gpt, 2, 3); // No memleakOnRealloc since g_realloc_n aborts if it fails
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_realloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_realloc(NULL, 1);

    gpointer gpt = g_try_malloc(1);
    // cppcheck-suppress memleakOnRealloc
    gpt = g_try_realloc(gpt, 2);
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_realloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_realloc_n(NULL, 1, 2);

    gpointer gpt = g_try_malloc_n(1, 2);
    // cppcheck-suppress memleakOnRealloc
    gpt = g_try_realloc_n(gpt, 2, 3);
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_assert_test()
{
    int a;
    // cppcheck-suppress checkLibraryNoReturn
    // cppcheck-suppress assignmentInAssert
    g_assert(a = 5);
}

void g_assert_true_false_test()
{
    gboolean t = TRUE;
    gboolean f = FALSE;
    g_assert_true(t);
    // cppcheck-suppress checkLibraryNoReturn
    g_assert_false(f);
}

void g_assert_null_nonnull_test()
{
    char * gpt = g_malloc(1);
    g_assert_nonnull(gpt);
    gpt[0] = 0;
    g_free(gpt);
    // cppcheck-suppress checkLibraryNoReturn
    g_assert_null(NULL);
}

void g_print_test()
{
    // cppcheck-suppress invalidPrintfArgType_uint
    g_print("%u", -1);
    // cppcheck-suppress invalidPrintfArgType_uint
    g_printerr("%x", "a");
}

void g_alloca_test()
{
    // cppcheck-suppress allocaCalled
    char * pBuf1 = g_alloca(5);
    pBuf1[0] = '\0';
}

void g_new_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_new(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_new(struct a, 1);

    const struct a * pNew2 = g_new(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_new_if_test()
{
    struct a {
        int b;
    };

    const struct a * pNew3;
    if (pNew3 = g_new(struct a, 6)) {
        printf("%p", pNew3);
    }
    // cppcheck-suppress memleak
}

void g_new0_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_new0(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_new0(struct a, 1);

    const struct a * pNew2 = g_new0(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_try_new_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_try_new(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_try_new(struct a, 1);

    const struct a * pNew2 = g_try_new(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}
void g_try_new0_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_try_new0(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_try_new0(struct a, 1);

    const struct a * pNew2 = g_try_new0(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_renew_test()
{
    struct a {
        int b;
    };
    // cppcheck-suppress leakReturnValNotUsed
    g_renew(struct a, NULL, 1);

    struct a * pNew = g_new(struct a, 1);
    pNew = g_renew(struct a, pNew, 2); // No memleakOnRealloc since g_renew aborts if it fails
    printf("%p", pNew);

    // cppcheck-suppress memleak
}

void g_try_renew_test()
{
    struct a {
        int b;
    };
    // cppcheck-suppress leakReturnValNotUsed
    g_try_renew(struct a, NULL, 1);

    struct a * pNew = g_try_new(struct a, 1);
    // cppcheck-suppress memleakOnRealloc
    pNew = g_try_renew(struct a, pNew, 2);
    printf("%p", pNew);

    // cppcheck-suppress memleak
}

void g_error_new_test()
{
    // valid
    GError * pNew1 = g_error_new(1, -2, "a %d", 1);
    printf("%p", pNew1);
    g_error_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_error_new(1, -2, "a %d", 1);

    const GError * pNew2 = g_error_new(1, -2, "a %d", 1);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_once_init_enter_leave_test()
{
    static gsize init_val;
    if (g_once_init_enter(&init_val)) {
        gsize result_val = 0;
        // cppcheck-suppress invalidFunctionArg
        g_once_init_leave(&init_val, result_val);
    }

    gsize init_val2;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress ignoredReturnValue
    g_once_init_enter(&init_val2);

    gsize * init_val3 = NULL;
    // cppcheck-suppress nullPointer
    if (g_once_init_enter(init_val3)) {
        gsize* init_val31 = NULL;
        // cppcheck-suppress nullPointer
        g_once_init_leave(init_val31, 1);
    }

    gsize * init_val4;
    // cppcheck-suppress uninitvar
    if (g_once_init_enter(init_val4)) {
        gsize * init_val5;
        // cppcheck-suppress uninitvar
        g_once_init_leave(init_val5, 1);
    }
}

void g_strchug_g_strchomp_test(gchar * str1)
{
    g_strchug(str1);
    g_strchomp(str1);
    g_strchug(g_strchomp(str1));
    gchar * str2;
    // cppcheck-suppress uninitvar
    g_strchug(str2);
    gchar * str3;
    // cppcheck-suppress uninitvar
    g_strchomp(str3);
}

void g_abort_test()
{
    g_abort();
    //cppcheck-suppress unreachableCode
    printf("Never reached");
}

gchar* g_strchug_string_free_test(GString* t) // #12301
{
    gchar* p = g_strchug(g_string_free(t, FALSE));
    return p;
}

void g_variant_test() {
    // valid
    GVariant *pGvariant = g_variant_new("i", 1);
    printf("%p\n", pGvariant);
    g_variant_unref(pGvariant);

    // cppcheck-suppress leakReturnValNotUsed
    g_variant_new("i", 1);

    const GVariant *pGvariant1 = g_variant_new("i", 1);
    printf("%p\n", pGvariant1);

    GVariant *pGvariant2 = g_variant_parse(
        NULL, "{'Test': <{'Test1': <uint32 1>}>}", NULL, NULL, NULL);
    printf("%p\n", pGvariant2);
    g_variant_unref(pGvariant2);

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "String",
                          g_variant_new_string("String"));
    g_variant_builder_add(&builder, "{sv}", "Byte", g_variant_new_byte(8));
    g_variant_builder_add(&builder, "{sv}", "Int16", g_variant_new_int16(-16));
    g_variant_builder_add(&builder, "{sv}", "Int32", g_variant_new_int32(-32));
    g_variant_builder_add(&builder, "{sv}", "Int64", g_variant_new_int64(-64));
    g_variant_builder_add(&builder, "{sv}", "Double", g_variant_new_double(1.0));
    g_variant_builder_add(&builder, "{sv}", "UInt16", g_variant_new_uint16(16));
    g_variant_builder_add(&builder, "{sv}", "UInt32", g_variant_new_uint32(32));
    g_variant_builder_add(&builder, "{sv}", "UInt64", g_variant_new_uint64(64));
    g_variant_builder_add(&builder, "{sv}", "Boolean",
                            g_variant_new_boolean(TRUE));
    g_variant_builder_add(&builder, "{sv}", "TakenString",
                            g_variant_new_take_string(g_strdup("Owned string")));
    g_variant_builder_add(&builder, "{sv}", "PrintfString",
                            g_variant_new_printf("Formatted %d", 1));
    g_variant_builder_add_value(
        &builder, g_variant_new("{sv}", "String",
                                g_variant_new_string("Owned string 2")));
    g_variant_builder_add_parsed(&builder, "{'String', <'Owned string 3'>}");
    GVariant *variant_dict = g_variant_builder_end(&builder);
    printf("%p\n", variant_dict);
    g_variant_unref(variant_dict);

    GVariantBuilder builder_complex;
    g_variant_builder_init(&builder_complex, G_VARIANT_TYPE("(sa(sa(sis)))"));
    g_variant_builder_add(&builder_complex, "s", "OuterTest");
    g_variant_builder_open(&builder_complex, G_VARIANT_TYPE("a(sa(sis))"));
    g_variant_builder_open(&builder_complex, G_VARIANT_TYPE("(sa(sis))"));
    g_variant_builder_add(&builder_complex, "s", "MiddelTest");
    g_variant_builder_open(&builder_complex, G_VARIANT_TYPE("a(sis)"));
    g_variant_builder_add(&builder_complex, "(sis)", "InnerTest", 1, "Value");
    g_variant_builder_close(&builder_complex); // "a(sis)"
    g_variant_builder_close(&builder_complex); // "(sa(sis))"
    g_variant_builder_close(&builder_complex); // "a(sa(sis))"
    GVariant *variant_complex =
        g_variant_builder_end(&builder_complex); // "(sa(sa(sis)))"
    printf("%p\n", variant_complex);
    g_variant_unref(variant_complex);

    // leak from pGvariant1
    // cppcheck-suppress memleak
}

void g_queue_test() {
    // cppcheck-suppress leakReturnValNotUsed
    g_queue_new();

    GQueue *queue = g_queue_new();
    g_queue_push_head(queue, "test");
    g_queue_push_tail(queue, "test");
    // cppcheck-suppress ignoredReturnValue
    g_queue_get_length(queue);
    // cppcheck-suppress ignoredReturnValue
    g_queue_is_empty(queue);
    // cppcheck-suppress ignoredReturnValue
    g_queue_peek_head(queue);
    // cppcheck-suppress ignoredReturnValue
    g_queue_peek_tail(queue);
    // cppcheck-suppress ignoredReturnValue
    g_queue_pop_head(queue);
    // cppcheck-suppress ignoredReturnValue
    g_queue_pop_tail(queue);

    GQueue *copy_queue = g_queue_copy(queue);
    g_queue_free(queue);
    printf("%p\n", copy_queue);
    g_queue_free(copy_queue);

    GQueue *queue2 = g_queue_new();
    g_queue_push_head(queue2, g_strdup("test"));
    g_queue_push_tail(queue2, g_strdup("test"));
    printf("%p\n", queue2);
    g_queue_free_full(queue2, g_free);

    const GQueue *queue3 = g_queue_new();
    printf("%p\n", queue3);
    // cppcheck-suppress memleak
}

void g_tree_test() {
    // cppcheck-suppress leakReturnValNotUsed
    g_tree_new((GCompareFunc)g_strcmp0);

    GTree *tree = g_tree_new((GCompareFunc)g_strcmp0);
    g_tree_insert(tree, "banana", "yellow");
    g_tree_insert(tree, "apple", "red");
    g_tree_insert(tree, "grape", "purple");
    // cppcheck-suppress ignoredReturnValue
    g_tree_nnodes(tree);
    g_tree_remove(tree, "test");
    // cppcheck-suppress ignoredReturnValue
    g_tree_lookup(tree, "banana");
    g_tree_destroy(tree);

    const GTree *tree2 = g_tree_new((GCompareFunc)g_strcmp0);
    printf("%p\n", tree2);
    // cppcheck-suppress memleak
}
