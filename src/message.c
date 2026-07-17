#include "torustiq_sdk/message.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

TorustiqMessage* torustiq_message_clone(const TorustiqMessage* src) {
    if (src == NULL) {
        return NULL;
    }

    TorustiqMessage* dst = (TorustiqMessage*)malloc(sizeof(TorustiqMessage));
    if (dst == NULL) {
        return NULL;
    }

    dst->type = src->type;
    dst->payload_size = src->payload_size;
    dst->headers_count = src->headers_count;
    dst->payload = NULL;
    dst->headers = NULL;

    /* Deep-copy payload */
    if (src->payload != NULL && src->payload_size > 0) {
        dst->payload = (uint8_t*)malloc(src->payload_size);
        if (dst->payload == NULL) {
            free(dst);
            return NULL;
        }
        memcpy(dst->payload, src->payload, src->payload_size);
    }

    /* Deep-copy headers */
    if (src->headers != NULL && src->headers_count > 0) {
        dst->headers = (TorustiqMessageHeader*)malloc(
            src->headers_count * sizeof(TorustiqMessageHeader));
        if (dst->headers == NULL) {
            free(dst->payload);
            free(dst);
            return NULL;
        }
        for (size_t i = 0; i < src->headers_count; i++) {
            dst->headers[i].key =
                src->headers[i].key != NULL ? strdup(src->headers[i].key) : NULL;
            dst->headers[i].value =
                src->headers[i].value != NULL ? strdup(src->headers[i].value) : NULL;
        }
    }

    return dst;
}

void torustiq_message_free(TorustiqMessage* msg) {
    if (msg == NULL) {
        return;
    }

    if (msg->headers != NULL) {
        for (size_t i = 0; i < msg->headers_count; i++) {
            free((void*)msg->headers[i].key);
            free((void*)msg->headers[i].value);
        }
        free(msg->headers);
    }

    free(msg->payload);
    free(msg);
}

TorustiqMessage torustiq_message_create_eof() {
    TorustiqMessage msg;
    msg.type = TORUSTIQ_MESSAGE_TYPE_EOF;
    msg.payload_size = 0;
    msg.payload = NULL;
    msg.headers_count = 0;
    msg.headers = NULL;
    return msg;
}
