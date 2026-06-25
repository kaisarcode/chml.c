/**
 * chml.c - ChatML message wrapping CLI
 * Summary: Reads stdin and wraps content in a ChatML message.
 *
 * Author:  KaisarCode
 * Website: https://kaisarcode.com
 * License: https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "chml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads text from standard input into a dynamically allocated buffer.
 * @param out_text Destination pointer for the allocated text.
 * @return 0 on success, or -1 on failure.
 */
static int kc_chml_read_stdin(char **out_text) {
    char *data = NULL;
    size_t length = 0;
    size_t capacity = 0;
    char chunk[4096];
    size_t n;

    if (!out_text) return -1;

    while ((n = fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
        if (length + n + 1 > capacity) {
            size_t next_cap = capacity ? capacity * 2 : 4096;
            while (next_cap < length + n + 1)
                next_cap *= 2;
            char *next_data = realloc(data, next_cap);
            if (!next_data) {
                free(data);
                return -1;
            }
            data = next_data;
            capacity = next_cap;
        }
        memcpy(data + length, chunk, n);
        length += n;
    }

    if (ferror(stdin)) {
        free(data);
        return -1;
    }

    if (length == 0) {
        *out_text = NULL;
        return 0;
    }

    data[length] = '\0';
    *out_text = data;
    return 0;
}

/**
 * Print command usage information.
 * @param name Program executable name.
 * @return None.
 */
static void kc_print_help(const char *name) {
    printf("Usage: %s [options]\n", name);
    printf("\n");
    printf("Options:\n");
    printf("  -r, --role ROLE    Message role: system, assistant, or user\n");
    printf("  -f, --format FMT   Output format: chatml, qwen, gemma, llama, mistral, alpaca, phi, or zephyr\n");
    printf("  -h, --help         Show this help\n");
    printf("  -v, --version      Show version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  echo 'Hola' | %s\n", name);
    printf("  echo 'Hola' | %s -f gemma\n", name);
    printf("  echo 'You are a bot.' | %s --role system\n", name);
}

/**
 * Print version information.
 * @return None.
 */
static void kc_print_version(void) {
    printf("chml build %llu\n", (unsigned long long)kc_chml_version());
}

/**
 * Signal handler callback that requests stop.
 * @param ctx ChatML context.
 * @return None.
 */
static void kc_chml_stop_handler(kc_chml_t *ctx) {
    if (ctx) kc_chml_stop(ctx);
}

/**
 * Entry point.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process status code.
 */
int main(int argc, char **argv) {
    kc_chml_options_t opts = kc_chml_options_default();
    char *stdin_text = NULL;
    int i = 1;

    kc_chml_options_load_env(&opts);

    while (i < argc) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            kc_print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--version") == 0) {
            kc_print_version();
            return 0;
        } else if (strcmp(argv[i], "-r") == 0 ||
            strcmp(argv[i], "--role") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "chml: missing value for %s\n", argv[i - 1]);
                kc_chml_options_free(&opts);
                return 1;
            }
            free(opts.role);
            opts.role = strdup(argv[i]);
        } else if (strcmp(argv[i], "-f") == 0 ||
            strcmp(argv[i], "--format") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "chml: missing value for %s\n", argv[i - 1]);
                kc_chml_options_free(&opts);
                return 1;
            }
            opts.format = kc_chml_format_from_name(argv[i]);
            if (opts.format == KC_CHML_ERROR) {
                fprintf(stderr, "chml: invalid format '%s'\n", argv[i]);
                kc_chml_options_free(&opts);
                return 1;
            }
        } else {
            fprintf(stderr, "chml: unknown option '%s'\n", argv[i]);
            kc_chml_options_free(&opts);
            return 1;
        }
        i++;
    }

    kc_chml_t *ctx = NULL;
    if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) {
        fprintf(stderr, "chml: open failed\n");
        kc_chml_options_free(&opts);
        return 1;
    }

    if (kc_chml_read_stdin(&stdin_text) != 0) {
        fprintf(stderr, "chml: failed to read stdin\n");
        kc_chml_close(ctx);
        kc_chml_options_free(&opts);
        return 1;
    }

    if (!stdin_text || !*stdin_text) {
        free(stdin_text);
        kc_chml_close(ctx);
        kc_chml_options_free(&opts);
        return 0;
    }

    {
        size_t sl = strlen(stdin_text);
        while (sl > 0 && (stdin_text[sl - 1] == '\n' ||
            stdin_text[sl - 1] == '\r'))
            stdin_text[--sl] = '\0';
    }

    kc_chml_on_signal(ctx, 2, kc_chml_stop_handler);
    kc_chml_on_signal(ctx, 15, kc_chml_stop_handler);
    kc_chml_listen_signals(ctx);
#ifndef _WIN32
    kc_chml_listen_signal(ctx, 2);
    kc_chml_listen_signal(ctx, 15);
#endif

    char *result = kc_chml_render(ctx, stdin_text);
    if (!result) {
        fprintf(stderr, "chml: render failed\n");
        kc_chml_close(ctx);
        free(stdin_text);
        kc_chml_options_free(&opts);
        return 1;
    }

    printf("%s", result);

    free(result);
    kc_chml_close(ctx);
    free(stdin_text);
    kc_chml_options_free(&opts);
    return 0;
}
