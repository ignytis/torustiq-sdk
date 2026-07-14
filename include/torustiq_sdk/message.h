#ifndef _TORUSTIQ_SDK_MESSAGE_H_
#define _TORUSTIQ_SDK_MESSAGE_H_

#include <torustiq_sdk/typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a deep copy of a TorustiqMessage, including payload and headers.
 * Returns NULL if src is NULL or allocation fails.
 * The caller is responsible for freeing the returned message via
 * torustiq_message_free.
 */
TorustiqMessage* torustiq_message_clone(const TorustiqMessage* src);

/**
 * Frees a TorustiqMessage and all of its owned memory (payload, headers).
 * Safe to call with NULL.
 */
void torustiq_message_free(TorustiqMessage* msg);

#ifdef __cplusplus
}
#endif

#endif  // _TORUSTIQ_SDK_MESSAGE_H_
