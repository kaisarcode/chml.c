/**
 * test.c - libchml public API tests.
 * Summary: Tests each public libchml function through one CTest case.
 *
 * Author:  KaisarCode
 * Website: https://kaisarcode.com
 * License: https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "chml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <process.h>
#define getpid _getpid
#else
#include <signal.h>
#include <unistd.h>
#endif

static int signal_count = 0;
static kc_chml_t *signal_ctx_seen = NULL;

/**
 * Records one signal callback invocation.
 * @param ctx Context supplied by the library.
 * @return None.
 */
static void count_signal(kc_chml_t *ctx) {
    if (ctx) { signal_count++; signal_ctx_seen = ctx; }
}

static int signal_count_b = 0;

/**
 * Records one replacement signal callback invocation.
 * @param ctx Context supplied by the library.
 * @return None.
 */
static void count_signal_b(kc_chml_t *ctx) {
    if (ctx) { signal_count_b++; }
}

/**
 * Verifies one integer result.
 * @param name Check description.
 * @param expected Expected value.
 * @param actual Actual value.
 * @return 0 on success, 1 on failure.
 */
static int expect_int(const char *name, int expected, int actual) {
    if (expected != actual) {
        printf("\033[31m[FAIL]\033[0m %s: expected %d, got %d\n", name, expected, actual);
        return 1;
    }
    printf("\033[32m[PASS]\033[0m %s\n", name);
    return 0;
}

/**
 * Verifies one boolean condition.
 * @param name Check description.
 * @param condition Non-zero when the check passed.
 * @return 0 on success, 1 on failure.
 */
static int expect_true(const char *name, int condition) {
    if (!condition) {
        printf("\033[31m[FAIL]\033[0m %s\n", name);
        return 1;
    }
    printf("\033[32m[PASS]\033[0m %s\n", name);
    return 0;
}

/**
 * Verifies one string result.
 * @param name Check description.
 * @param expected Expected string.
 * @param actual Actual string.
 * @return 0 on success, 1 on failure.
 */
static int expect_string(const char *name, const char *expected, const char *actual) {
    if (actual == NULL || strcmp(expected, actual) != 0) {
        printf("\033[31m[FAIL]\033[0m %s: expected '%s', got '%s'\n", name, expected,
            actual != NULL ? actual : "NULL");
        return 1;
    }
    printf("\033[32m[PASS]\033[0m %s\n", name);
    return 0;
}

/**
 * Tests kc_chml_version.
 * @return 0 on success, 1 on failure.
 */
static int case_version(void) {
    return expect_true("version returns non-zero", kc_chml_version() != 0U);
}

/**
 * Tests kc_chml_options_default.
 * @return 0 on success, 1 on failure.
 */
static int case_options_default(void) {
    kc_chml_options_t opts;
    opts = kc_chml_options_default();
    return expect_true("default options zeroed", opts.role == NULL && opts.format == 0);
}

/**
 * Tests kc_chml_options_load_env.
 * @return 0 on success, 1 on failure.
 */
static int case_options_load_env(void) {
    kc_chml_options_t opts = {0};
    kc_chml_options_load_env(&opts);
    kc_chml_options_load_env(NULL);
    return expect_true("load_env does not crash", 1);
}

/**
 * Tests kc_chml_options_free.
 * @return 0 on success, 1 on failure.
 */
static int case_options_free(void) {
    kc_chml_options_t opts = {0};
    kc_chml_options_free(&opts);
    kc_chml_options_free(NULL);
    return expect_true("options_free does not crash", 1);
}

/**
 * Tests kc_chml_open and kc_chml_close.
 * @return 0 on success, 1 on failure.
 */
static int case_open_close(void) {
    kc_chml_t *ctx = NULL;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    opts = kc_chml_options_default();
    rc += expect_int("open(NULL) returns ERROR", KC_CHML_ERROR, kc_chml_open(NULL, &opts));
    rc += expect_int("open(out, NULL) returns ERROR", KC_CHML_ERROR, kc_chml_open(&ctx, NULL));
    rc += expect_int("open valid returns OK", KC_CHML_OK, kc_chml_open(&ctx, &opts));
    rc += expect_true("open creates valid context", ctx != NULL);
    kc_chml_close(ctx);
    kc_chml_close(NULL);
    rc += expect_true("close does not crash", 1);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_stop.
 * @return 0 on success, 1 on failure.
 */
static int case_stop(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    rc += expect_int("stop(NULL) returns ERROR", KC_CHML_ERROR, kc_chml_stop(NULL));
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("stop returns OK", KC_CHML_OK, kc_chml_stop(ctx));
    rc += expect_int("stop is idempotent", KC_CHML_OK, kc_chml_stop(ctx));
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_get_role and kc_chml_get_role_name.
 * @return 0 on success, 1 on failure.
 */
static int case_get_role(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    opts = kc_chml_options_default();
    opts.role = "system";
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("get_role returns system", KC_CHML_ROLE_SYSTEM, kc_chml_get_role(ctx));
    rc += expect_string("get_role_name returns system", "system", kc_chml_get_role_name(ctx));
    kc_chml_close(ctx);
    opts.role = "assistant";
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("get_role returns assistant", KC_CHML_ROLE_ASSISTANT, kc_chml_get_role(ctx));
    rc += expect_string("get_role_name returns assistant", "assistant", kc_chml_get_role_name(ctx));
    kc_chml_close(ctx);
    opts.role = "user";
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("get_role returns user", KC_CHML_ROLE_USER, kc_chml_get_role(ctx));
    rc += expect_string("get_role_name returns user", "user", kc_chml_get_role_name(ctx));
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_format_from_name.
 * @return 0 on success, 1 on failure.
 */
static int case_format_from_name(void) {
    int rc;

    rc = 0;
    rc += expect_int("format_from_name(NULL) returns ERROR", KC_CHML_ERROR, kc_chml_format_from_name(NULL));
    rc += expect_int("format_from_name(chatml) returns 0", KC_CHML_FMT_CHATML, kc_chml_format_from_name("chatml"));
    rc += expect_int("format_from_name(gemma) returns 1", KC_CHML_FMT_GEMMA, kc_chml_format_from_name("gemma"));
    rc += expect_int("format_from_name(llama) returns 2", KC_CHML_FMT_LLAMA, kc_chml_format_from_name("llama"));
    rc += expect_int("format_from_name(mistral) returns 3", KC_CHML_FMT_MISTRAL, kc_chml_format_from_name("mistral"));
    rc += expect_int("format_from_name(alpaca) returns 4", KC_CHML_FMT_ALPACA, kc_chml_format_from_name("alpaca"));
    rc += expect_int("format_from_name(phi) returns 5", KC_CHML_FMT_PHI, kc_chml_format_from_name("phi"));
    rc += expect_int("format_from_name(zephyr) returns 6", KC_CHML_FMT_ZEPHYR, kc_chml_format_from_name("zephyr"));
    rc += expect_int("format_from_name(invalid) returns ERROR", KC_CHML_ERROR, kc_chml_format_from_name("invalid"));
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_render with different formats.
 * @return 0 on success, 1 on failure.
 */
static int case_render(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    char *result;
    int rc;

    rc = 0;
    rc += expect_true("render(NULL) returns NULL", kc_chml_render(NULL, "hi") == NULL);
    rc += expect_true("render(ctx, NULL) returns NULL", kc_chml_render(NULL, NULL) == NULL);
    opts = kc_chml_options_default();
    opts.role = "user";
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "hello");
    rc += expect_true("render produces output", result != NULL);
    rc += expect_true("render contains content", result != NULL && strstr(result, "hello") != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_GEMMA;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("gemma render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_LLAMA;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("llama render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_MISTRAL;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("mistral render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_ALPACA;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("alpaca render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_PHI;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("phi render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    opts.format = KC_CHML_FMT_ZEPHYR;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    result = kc_chml_render(ctx, "test");
    rc += expect_true("zephyr render produces output", result != NULL);
    free(result);
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_on_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_on_signal(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;
    int i;

    rc = 0;
    signal_count = 0;
    signal_count_b = 0;
    rc += expect_int("on_signal NULL returns ERROR", KC_CHML_ERROR, kc_chml_on_signal(NULL, 1, count_signal));
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("remove missing returns OK", KC_CHML_OK, kc_chml_on_signal(ctx, 1, NULL));
    rc += expect_int("register returns OK", KC_CHML_OK, kc_chml_on_signal(ctx, 1, count_signal));
    rc += expect_int("raise returns OK", KC_CHML_OK, kc_chml_raise_signal(ctx, 1));
    rc += expect_int("handler invoked", 1, signal_count);
    rc += expect_int("replace returns OK", KC_CHML_OK, kc_chml_on_signal(ctx, 1, count_signal_b));
    signal_count = 0; signal_count_b = 0;
    kc_chml_raise_signal(ctx, 1);
    rc += expect_int("old not invoked", 0, signal_count);
    rc += expect_int("replacement invoked", 1, signal_count_b);
    rc += expect_int("remove returns OK", KC_CHML_OK, kc_chml_on_signal(ctx, 1, NULL));
    rc += expect_int("raise removed returns ERROR", KC_CHML_ERROR, kc_chml_raise_signal(ctx, 1));
    for (i = 0; i < 8; i++) {
        rc += expect_int("register growth returns OK", KC_CHML_OK, kc_chml_on_signal(ctx, 200 + i, count_signal));
    }
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_raise_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_raise_signal(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    signal_count = 0;
    signal_ctx_seen = NULL;
    rc += expect_int("raise_signal NULL returns ERROR", KC_CHML_ERROR, kc_chml_raise_signal(NULL, 1));
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    rc += expect_int("raise unhandled returns ERROR", KC_CHML_ERROR, kc_chml_raise_signal(ctx, 1));
    kc_chml_on_signal(ctx, 1, count_signal);
    rc += expect_int("raise handled returns OK", KC_CHML_OK, kc_chml_raise_signal(ctx, 1));
    rc += expect_true("context matches", signal_ctx_seen == ctx);
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_listen_signals.
 * @return 0 on success, 1 on failure.
 */
static int case_listen_signals(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    signal_count = 0;
    signal_ctx_seen = NULL;
    rc += expect_int("listen_signals NULL returns ERROR", KC_CHML_ERROR, kc_chml_listen_signals(NULL));
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    kc_chml_on_signal(ctx, 44, count_signal);
    rc += expect_int("listen_signals returns OK", KC_CHML_OK, kc_chml_listen_signals(ctx));
    kc_chml_signal_listener(44);
    rc += expect_int("listener dispatched", 1, signal_count);
    rc += expect_true("correct context", signal_ctx_seen == ctx);
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_listen_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_listen_signal(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    rc += expect_int("listen_signal NULL returns ERROR", KC_CHML_ERROR, kc_chml_listen_signal(NULL, 1));
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
#ifdef _WIN32
    rc += expect_int("listen_signal returns OK", KC_CHML_OK, kc_chml_listen_signal(ctx, 2));
#else
    rc += expect_int("listen_signal returns OK", KC_CHML_OK, kc_chml_listen_signal(ctx, SIGUSR1));
#endif
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests kc_chml_signal_listener.
 * @return 0 on success, 1 on failure.
 */
static int case_signal_listener(void) {
    kc_chml_t *ctx;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    signal_count = 0;
    signal_ctx_seen = NULL;
    opts = kc_chml_options_default();
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) return 1;
    kc_chml_on_signal(ctx, 55, count_signal);
    kc_chml_listen_signals(ctx);
    kc_chml_signal_listener(55);
    rc += expect_int("signal_listener invokes", 1, signal_count);
    rc += expect_true("correct context", signal_ctx_seen == ctx);
    kc_chml_close(ctx);
    return rc == 0 ? 0 : 1;
}

/**
 * Tests two contexts coexist.
 * @return 0 on success, 1 on failure.
 */
static int case_multictx(void) {
    kc_chml_t *a;
    kc_chml_t *b;
    kc_chml_options_t opts;
    int rc;

    rc = 0;
    opts = kc_chml_options_default();
    if (kc_chml_open(&a, &opts) != KC_CHML_OK) return 1;
    if (kc_chml_open(&b, &opts) != KC_CHML_OK) {
        kc_chml_close(a);
        return 1;
    }
    rc += expect_int("stop a returns OK", KC_CHML_OK, kc_chml_stop(a));
    rc += expect_int("stop b returns OK", KC_CHML_OK, kc_chml_stop(b));
    rc += expect_int("close a returns OK", KC_CHML_OK, 0);
    kc_chml_close(a);
    rc += expect_int("close b returns OK", KC_CHML_OK, 0);
    kc_chml_close(b);
    return rc == 0 ? 0 : 1;
}

/**
 * Runs one libchml public API test case.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 or 2 on failure.
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "test case: expected one argument, got %d\n", argc - 1);
        return 2;
    }
    if (strcmp(argv[1], "version") == 0) return case_version();
    if (strcmp(argv[1], "options-default") == 0) return case_options_default();
    if (strcmp(argv[1], "options-load-env") == 0) return case_options_load_env();
    if (strcmp(argv[1], "options-free") == 0) return case_options_free();
    if (strcmp(argv[1], "open-close") == 0) return case_open_close();
    if (strcmp(argv[1], "stop") == 0) return case_stop();
    if (strcmp(argv[1], "get-role") == 0) return case_get_role();
    if (strcmp(argv[1], "format-from-name") == 0) return case_format_from_name();
    if (strcmp(argv[1], "render") == 0) return case_render();
    if (strcmp(argv[1], "on-signal") == 0) return case_on_signal();
    if (strcmp(argv[1], "raise-signal") == 0) return case_raise_signal();
    if (strcmp(argv[1], "listen-signals") == 0) return case_listen_signals();
    if (strcmp(argv[1], "listen-signal") == 0) return case_listen_signal();
    if (strcmp(argv[1], "signal-listener") == 0) return case_signal_listener();
    if (strcmp(argv[1], "multictx") == 0) return case_multictx();
    fprintf(stderr, "unknown test case: %s\n", argv[1]);
    return 2;
}
