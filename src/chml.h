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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kc_chml kc_chml_t;

#define KC_CHML_OK      0
#define KC_CHML_ERROR  -1

#define KC_CHML_ROLE_SYSTEM     0
#define KC_CHML_ROLE_ASSISTANT  1
#define KC_CHML_ROLE_USER       2

/**
 * Create a new ChatML context.
 * @return Context pointer or NULL on failure.
 */
kc_chml_t *kc_chml_open(void);

/**
 * Set the message role.
 * @param ctx Context pointer.
 * @param role Role constant (KC_CHML_ROLE_*).
 * @return KC_CHML_OK or KC_CHML_ERROR.
 */
int kc_chml_set_role(kc_chml_t *ctx, int role);

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

#ifdef __cplusplus
}
#endif

#endif
