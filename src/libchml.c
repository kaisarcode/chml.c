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
#endif

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

struct kc_chml {
    int role;
};

/**
 * Create a new ChatML context.
 * @return Context pointer or NULL on failure.
 */
kc_chml_t *kc_chml_open(void) {
    struct kc_chml *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;
    ctx->role = KC_CHML_ROLE_USER;
    return ctx;
}

/**
 * Set the message role.
 * @param ctx Context pointer.
 * @param role Role constant (KC_CHML_ROLE_*).
 * @return KC_CHML_OK or KC_CHML_ERROR.
 */
int kc_chml_set_role(kc_chml_t *ctx, int role) {
    if (!ctx) return KC_CHML_ERROR;
    if (role < 0 || role >= KC_CHML_ROLE_COUNT) return KC_CHML_ERROR;
    ctx->role = role;
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

    size_t role_len = strlen(role_name);
    size_t content_len = strlen(content);

    size_t prefix_len = 12;
    size_t suffix_len = 12;

    size_t total = prefix_len + role_len + 1 + content_len + suffix_len + 1;
    char *out = malloc(total);
    if (!out) return NULL;

    int n = snprintf(out, total,
        "<|im_start|>%s\n%s\n<|im_end|>\n", role_name, content);
    if (n < 0 || (size_t)n >= total) {
        free(out);
        return NULL;
    }

    return out;
}

/**
 * Release a ChatML context.
 * @param ctx Context pointer (NULL safe).
 * @return None.
 */
void kc_chml_close(kc_chml_t *ctx) {
    free(ctx);
}
