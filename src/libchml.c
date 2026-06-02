/**
 * libchml.c - ChatML message wrapping library
 * Summary: Core implementation for ChatML message rendering.
 *
 * Author:  KaisarCode
 * Website: https://kaisarcode.com
 * License: https://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <signal.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stddef.h>

#include "chml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *kc_chml_role_names[] = {
    "system",
    "assistant",
    "user"
};

#define KC_CHML_ROLE_COUNT 3

typedef enum {
    KC_ENV_TYPE_INT,
    KC_ENV_TYPE_FLOAT,
    KC_ENV_TYPE_STR
} kc_env_type_t;

typedef struct {
    const char *env_var;
    size_t offset;
    kc_env_type_t type;
} kc_env_map_t;

static const kc_env_map_t env_config_table[] = {
    { "KC_CHML_ROLE", offsetof(kc_chml_options_t, role), KC_ENV_TYPE_STR },
};
static const int env_config_table_n =
    sizeof(env_config_table) / sizeof(env_config_table[0]);

typedef struct {
    const char *name;
    int value;
} kc_chml_format_entry_t;

static const kc_chml_format_entry_t kc_chml_format_table[] = {
    { "chatml", KC_CHML_FMT_CHATML },
    { "qwen", KC_CHML_FMT_CHATML },
    { "gemma", KC_CHML_FMT_GEMMA },
    { "llama", KC_CHML_FMT_LLAMA },
    { "mistral", KC_CHML_FMT_MISTRAL },
    { "alpaca", KC_CHML_FMT_ALPACA },
    { "phi", KC_CHML_FMT_PHI },
    { "zephyr", KC_CHML_FMT_ZEPHYR }
};

static const int kc_chml_format_table_n =
    sizeof(kc_chml_format_table) / sizeof(kc_chml_format_table[0]);

typedef struct {
    int sig;
    kc_chml_signal_callback_t cb;
} kc_chml_signal_entry_t;

static kc_chml_t *g_signal_ctx = NULL;

struct kc_chml {
    int role;
    int format;
    kc_chml_options_t opts;
    kc_chml_signal_entry_t *signal_handlers;
    int n_signal_handlers;
    int signal_handlers_capacity;
};

/**
 * Allocate formatted output for a two-string template.
 * @param format Printf-compatible template.
 * @param first First string parameter.
 * @param second Second string parameter.
 * @return Allocated string, or NULL on failure.
 */
static char *kc_chml_render_two(const char *format,
const char *first,
const char *second) {
    int n;
    char *out;
    size_t total;

    n = snprintf(NULL, 0, format, first, second);
    if (n < 0) {
        return NULL;
    }

    total = (size_t)n + 1;
    out = malloc(total);
    if (!out) {
        return NULL;
    }

    n = snprintf(out, total, format, first, second);
    if (n < 0 || (size_t)n >= total) {
        free(out);
        return NULL;
    }

    return out;
}

/**
 * Allocate formatted output for a one-string template.
 * @param format Printf-compatible template.
 * @param first First string parameter.
 * @return Allocated string, or NULL on failure.
 */
static char *kc_chml_render_one(const char *format, const char *first) {
    int n;
    char *out;
    size_t total;

    n = snprintf(NULL, 0, format, first);
    if (n < 0) {
        return NULL;
    }

    total = (size_t)n + 1;
    out = malloc(total);
    if (!out) {
        return NULL;
    }

    n = snprintf(out, total, format, first);
    if (n < 0 || (size_t)n >= total) {
        free(out);
        return NULL;
    }

    return out;
}

/**
 * Create a new ChatML context.
 * @param out Pointer to receive the context pointer.
 * @param opts Options.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_open(kc_chml_t **out, const kc_chml_options_t *opts) {
    kc_chml_t *ctx;

    if (!out || !opts) {
        return KC_CHML_ERROR;
    }

    ctx = (kc_chml_t *)calloc(1, sizeof(kc_chml_t));
    if (!ctx) {
        return KC_CHML_ERROR;
    }

    ctx->opts = *opts;
    ctx->opts.role = opts->role ? strdup(opts->role) : NULL;

    if (opts->format < KC_CHML_FMT_CHATML || opts->format > KC_CHML_FMT_ZEPHYR) {
        ctx->format = KC_CHML_FMT_CHATML;
    } else {
        ctx->format = opts->format;
    }

    if (ctx->opts.role) {
        if (strcmp(ctx->opts.role, "system") == 0) {
            ctx->role = KC_CHML_ROLE_SYSTEM;
        } else if (strcmp(ctx->opts.role, "assistant") == 0) {
            ctx->role = KC_CHML_ROLE_ASSISTANT;
        } else if (strcmp(ctx->opts.role, "user") == 0) {
            ctx->role = KC_CHML_ROLE_USER;
        } else {
            kc_chml_close(ctx);
            return KC_CHML_ERROR;
        }
    } else {
        ctx->role = KC_CHML_ROLE_USER;
    }

    *out = ctx;
    return KC_CHML_OK;
}

/**
 * Get the current role constant.
 * @param ctx Context pointer.
 * @return Role constant.
 */
int kc_chml_get_role(const kc_chml_t *ctx) {
    if (!ctx) return KC_CHML_ERROR;
    return ctx->role;
}

/**
 * Get the role name string.
 * @param ctx Context pointer.
 * @return Role string ("system", "assistant", "user") or NULL on error.
 */
const char *kc_chml_get_role_name(const kc_chml_t *ctx) {
    if (!ctx) return NULL;
    if (ctx->role < 0 || ctx->role >= KC_CHML_ROLE_COUNT) return NULL;
    return kc_chml_role_names[ctx->role];
}

/**
 * Resolve a format name to its format constant.
 * @param name Format name.
 * @return Format constant, or KC_CHML_ERROR on invalid input.
 */
int kc_chml_format_from_name(const char *name) {
    int i;

    if (!name) {
        return KC_CHML_ERROR;
    }

    for (i = 0; i < kc_chml_format_table_n; i++) {
        if (strcmp(name, kc_chml_format_table[i].name) == 0) {
            return kc_chml_format_table[i].value;
        }
    }

    return KC_CHML_ERROR;
}

/**
 * Render content into a ChatML message.
 * Allocates and returns a new string. Caller must free.
 * @param ctx Context pointer.
 * @param content Message content text.
 * @return Allocated ChatML string or NULL on failure.
 */
char *kc_chml_render(const kc_chml_t *ctx, const char *content) {
    if (!ctx || !content) return NULL;

    const char *role_name = kc_chml_get_role_name(ctx);
    if (!role_name) return NULL;

    if (ctx->format == KC_CHML_FMT_GEMMA) {
        if (ctx->role == KC_CHML_ROLE_USER) {
            return kc_chml_render_two(
                "<|turn|>%s\n%s\n<|turn|>\n<|turn|>model\n",
                role_name, content);
        }
        return kc_chml_render_two("<|turn|>%s\n%s\n<|turn|>\n",
            role_name, content);
    }

    if (ctx->format == KC_CHML_FMT_LLAMA) {
        if (ctx->role == KC_CHML_ROLE_USER) {
            return kc_chml_render_two(
                "<|begin_of_text|><|start_header_id|>%s<|end_header_id|>\n\n%s<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n",
                role_name, content);
        }
        return kc_chml_render_two(
            "<|begin_of_text|><|start_header_id|>%s<|end_header_id|>\n\n%s<|eot_id|>",
            role_name, content);
    }

    if (ctx->format == KC_CHML_FMT_MISTRAL) {
        if (ctx->role == KC_CHML_ROLE_SYSTEM) {
            return kc_chml_render_one("[INST] <<SYS>>\n%s\n<</SYS>> [/INST]\n",
                content);
        }
        if (ctx->role == KC_CHML_ROLE_ASSISTANT) {
            return kc_chml_render_one("%s</s>\n", content);
        }
        return kc_chml_render_one("[INST] %s [/INST]\n", content);
    }

    if (ctx->format == KC_CHML_FMT_ALPACA) {
        if (ctx->role == KC_CHML_ROLE_SYSTEM) {
            return kc_chml_render_one("### System:\n%s\n", content);
        }
        if (ctx->role == KC_CHML_ROLE_ASSISTANT) {
            return kc_chml_render_one("### Response:\n%s\n", content);
        }
        return kc_chml_render_one("### Instruction:\n%s\n\n### Response:\n", content);
    }

    if (ctx->format == KC_CHML_FMT_PHI) {
        if (ctx->role == KC_CHML_ROLE_USER) {
            return kc_chml_render_one("<|user|>\n%s<|end|>\n<|assistant|>\n",
                content);
        }
        return kc_chml_render_two("<|%s|>\n%s<|end|>\n", role_name, content);
    }

    if (ctx->format == KC_CHML_FMT_ZEPHYR) {
        if (ctx->role == KC_CHML_ROLE_USER) {
            return kc_chml_render_two("<|%s|>\n%s</s>\n<|assistant|>\n",
                role_name, content);
        }
        return kc_chml_render_two("<|%s|>\n%s</s>\n", role_name, content);
    }

    if (ctx->role == KC_CHML_ROLE_USER) {
        return kc_chml_render_two(
            "<|im_start|>%s\n%s\n<|im_end|>\n<|im_start|>assistant\n",
            role_name, content);
    }
    return kc_chml_render_two("<|im_start|>%s\n%s\n<|im_end|>\n",
        role_name, content);
}

/**
 * Release a ChatML context.
 * @param ctx Context pointer (NULL safe).
 * @return None.
 */
void kc_chml_close(kc_chml_t *ctx) {
    if (!ctx) {
        return;
    }

    kc_chml_options_free(&ctx->opts);
    free(ctx->signal_handlers);
    free(ctx);
}

/**
 * Create an options struct initialized with default values.
 * @param none Unused.
 * @return Default-initialized options.
 */
kc_chml_options_t kc_chml_options_default(void) {
    kc_chml_options_t opts;
    memset(&opts, 0, sizeof(opts));
    return opts;
}

/**
 * Load configuration from environment variables.
 * @param opts Options to update.
 * @return None.
 */
void kc_chml_options_load_env(kc_chml_options_t *opts) {
    int i;

    if (!opts) {
        return;
    }

    for (i = 0; i < env_config_table_n; i++) {
        const char *val = getenv(env_config_table[i].env_var);
        char *end;

        if (!val) {
            continue;
        }

        switch (env_config_table[i].type) {
            case KC_ENV_TYPE_INT: {
                long v = strtol(val, &end, 10);
                if (end != val && *end == '\0') {
                    *(int *)((char *)opts + env_config_table[i].offset) = (int)v;
                }
                break;
            }
            case KC_ENV_TYPE_FLOAT: {
                float v = strtof(val, &end);
                if (end != val && *end == '\0') {
                    *(float *)((char *)opts + env_config_table[i].offset) = v;
                }
                break;
            }
            case KC_ENV_TYPE_STR: {
                char **p = (char **)((char *)opts + env_config_table[i].offset);
                free(*p);
                *p = strdup(val);
                break;
            }
        }
    }

    {
        const char *fmt = getenv("KC_CHML_FMT");
        int value = kc_chml_format_from_name(fmt);
        if (value != KC_CHML_ERROR) {
            opts->format = value;
        }
    }
}

/**
 * Free dynamically allocated resources within an options struct.
 * @param opts Options to clean up.
 * @return None.
 */
void kc_chml_options_free(kc_chml_options_t *opts) {
    if (!opts) {
        return;
    }

    free(opts->role);
    opts->role = NULL;
}

/**
 * Register a handler for a library-level signal number.
 * @param ctx ChatML context.
 * @param sig Application-defined signal number.
 * @param cb Callback to invoke.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_on_signal(kc_chml_t *ctx, int sig, kc_chml_signal_callback_t cb) {
    int i;

    if (!ctx) {
        return KC_CHML_ERROR;
    }

    for (i = 0; i < ctx->n_signal_handlers; i++) {
        if (ctx->signal_handlers[i].sig == sig) {
            if (cb) {
                ctx->signal_handlers[i].cb = cb;
            } else {
                int tail = ctx->n_signal_handlers - i - 1;
                if (tail > 0) {
                    memmove(&ctx->signal_handlers[i],
                            &ctx->signal_handlers[i + 1],
                            (size_t)tail * sizeof(kc_chml_signal_entry_t));
                }
                ctx->n_signal_handlers--;
            }
            return KC_CHML_OK;
        }
    }

    if (!cb) {
        return KC_CHML_OK;
    }

    if (ctx->n_signal_handlers >= ctx->signal_handlers_capacity) {
        int new_cap = ctx->signal_handlers_capacity ? ctx->signal_handlers_capacity * 2 : 4;
        kc_chml_signal_entry_t *p = (kc_chml_signal_entry_t *)realloc(ctx->signal_handlers,
            (size_t)new_cap * sizeof(kc_chml_signal_entry_t));

        if (!p) {
            return KC_CHML_ERROR;
        }

        ctx->signal_handlers = p;
        ctx->signal_handlers_capacity = new_cap;
    }

    ctx->signal_handlers[ctx->n_signal_handlers].sig = sig;
    ctx->signal_handlers[ctx->n_signal_handlers].cb = cb;
    ctx->n_signal_handlers++;

    return KC_CHML_OK;
}

/**
 * Raise a library-level signal.
 * @param ctx ChatML context.
 * @param sig Signal number to raise.
 * @return KC_CHML_OK if handled, or KC_CHML_ERROR if no handler.
 */
int kc_chml_raise_signal(kc_chml_t *ctx, int sig) {
    int i;

    if (!ctx) {
        return KC_CHML_ERROR;
    }

    for (i = 0; i < ctx->n_signal_handlers; i++) {
        if (ctx->signal_handlers[i].sig == sig) {
            ctx->signal_handlers[i].cb(ctx);
            return KC_CHML_OK;
        }
    }

    return KC_CHML_ERROR;
}

/**
 * Set the internal signal-listener context.
 * @param ctx ChatML context.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR if ctx is NULL.
 */
int kc_chml_listen_signals(kc_chml_t *ctx) {
    if (!ctx) {
        return KC_CHML_ERROR;
    }

    g_signal_ctx = ctx;
    return KC_CHML_OK;
}

/**
 * Wire an OS signal to the library signal listener.
 * @param ctx ChatML context.
 * @param sig_id OS signal number.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_listen_signal(kc_chml_t *ctx, int sig_id) {
    if (!ctx) {
        return KC_CHML_ERROR;
    }

    g_signal_ctx = ctx;

#ifdef _WIN32
    (void)sig_id;
#else
    signal(sig_id, kc_chml_signal_listener);
#endif

    return KC_CHML_OK;
}

/**
 * Generic signal-listener compatible with signal() / sigaction().
 * @param sig OS signal number.
 * @return None.
 */
void kc_chml_signal_listener(int sig) {
    if (g_signal_ctx) {
        kc_chml_raise_signal(g_signal_ctx, sig);
    }
}
