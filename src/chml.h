/**
 * chml.h - ChatML message wrapping
 * Summary: Wraps text content in a single ChatML message with role.
 *
 * Author:  KaisarCode
 * Website: https://kaisarcode.com
 * License: https://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef KC_CHML_H
#define KC_CHML_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kc_chml kc_chml_t;

#define KC_CHML_OK      0
#define KC_CHML_ERROR  -1

#define KC_CHML_ROLE_SYSTEM     0
#define KC_CHML_ROLE_ASSISTANT  1
#define KC_CHML_ROLE_USER       2

#define KC_CHML_FMT_CHATML   0
#define KC_CHML_FMT_GEMMA    1
#define KC_CHML_FMT_LLAMA    2
#define KC_CHML_FMT_MISTRAL  3
#define KC_CHML_FMT_ALPACA   4
#define KC_CHML_FMT_PHI      5
#define KC_CHML_FMT_ZEPHYR   6

/**
 * ChatML options.
 * @param role Message role string (e.g., "system", "assistant", "user").
 * @param format Output format constant.
 */
typedef struct kc_chml_options {
    char *role;
    int format;
} kc_chml_options_t;

/**
 * Callback type for library-level signal handling.
 * @param ctx ChatML context.
 */
typedef void (*kc_chml_signal_callback_t)(kc_chml_t *ctx);

/**
 * Create a new ChatML context.
 * @param out Pointer to receive the context pointer.
 * @param opts Options.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_open(kc_chml_t **out, const kc_chml_options_t *opts);

/**
 * Get the current role constant.
 * @param ctx Context pointer.
 * @return Role constant.
 */
int kc_chml_get_role(const kc_chml_t *ctx);

/**
 * Get the role name string.
 * @param ctx Context pointer.
 * @return Role string ("system", "assistant", "user").
 */
const char *kc_chml_get_role_name(const kc_chml_t *ctx);

/**
 * Resolve a format name to its format constant.
 * @param name Format name.
 * @return Format constant, or KC_CHML_ERROR on invalid input.
 */
int kc_chml_format_from_name(const char *name);

/**
 * Render content into a ChatML message.
 * Allocates and returns a new string. Caller must free.
 * @param ctx Context pointer.
 * @param content Message content text.
 * @return Allocated ChatML string or NULL on failure.
 */
char *kc_chml_render(const kc_chml_t *ctx, const char *content);

/**
 * Release a ChatML context.
 * @param ctx Context pointer (NULL safe).
 * @return None.
 */
void kc_chml_close(kc_chml_t *ctx);

/**
 * Create an options struct initialized with default values.
 * @param none Unused.
 * @return Default-initialized options.
 */
kc_chml_options_t kc_chml_options_default(void);

/**
 * Load configuration from environment variables.
 * @param opts Options to update.
 * @return None.
 */
void kc_chml_options_load_env(kc_chml_options_t *opts);

/**
 * Free dynamically allocated resources within an options struct.
 * @param opts Options to clean up.
 * @return None.
 */
void kc_chml_options_free(kc_chml_options_t *opts);

/**
 * Register a handler for a library-level signal number.
 * @param ctx ChatML context.
 * @param sig Application-defined signal number.
 * @param cb Callback to invoke.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_on_signal(kc_chml_t *ctx, int sig, kc_chml_signal_callback_t cb);

/**
 * Raise a library-level signal.
 * @param ctx ChatML context.
 * @param sig Signal number to raise.
 * @return KC_CHML_OK if handled, or KC_CHML_ERROR if no handler.
 */
int kc_chml_raise_signal(kc_chml_t *ctx, int sig);

/**
 * Set the internal signal-listener context.
 * @param ctx ChatML context.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR if ctx is NULL.
 */
int kc_chml_listen_signals(kc_chml_t *ctx);

/**
 * Wire an OS signal to the library signal listener.
 * @param ctx ChatML context.
 * @param sig_id OS signal number.
 * @return KC_CHML_OK on success, or KC_CHML_ERROR on failure.
 */
int kc_chml_listen_signal(kc_chml_t *ctx, int sig_id);

/**
 * Generic signal-listener compatible with signal() / sigaction().
 * @param sig OS signal number.
 * @return None.
 */
void kc_chml_signal_listener(int sig);

/**
 * Returns the build version generated at compile time.
 * @return Unix timestamp for the current build.
 */
uint64_t kc_chml_version(void);

#ifdef __cplusplus
}
#endif

#endif
