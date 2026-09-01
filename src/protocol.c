#include "protocol.h"

static uint8_t checksum_of(const uint8_t *payload, uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum ^= payload[i];
    }
    return sum;
}

size_t protocol_encode(uint8_t *out, size_t out_cap, const uint8_t *payload, uint8_t payload_len)
{
    if (payload_len > PROTOCOL_MAX_PAYLOAD) {
        return 0;
    }
    size_t frame_len = 2u + payload_len + 1u;
    if (frame_len > out_cap) {
        return 0;
    }

    out[0] = PROTOCOL_START_BYTE;
    out[1] = payload_len;
    for (uint8_t i = 0; i < payload_len; i++) {
        out[2 + i] = payload[i];
    }
    out[2 + payload_len] = checksum_of(payload, payload_len);

    return frame_len;
}

protocol_status_t protocol_decode(const uint8_t *frame, size_t frame_len,
                                   uint8_t *payload_out, uint8_t *payload_len_out)
{
    if (frame_len < 3u) {
        return PROTOCOL_ERR_TOO_SHORT;
    }
    if (frame[0] != PROTOCOL_START_BYTE) {
        return PROTOCOL_ERR_BAD_START;
    }

    uint8_t payload_len = frame[1];
    if (payload_len > PROTOCOL_MAX_PAYLOAD || frame_len != (size_t)(2u + payload_len + 1u)) {
        return PROTOCOL_ERR_LENGTH_MISMATCH;
    }

    const uint8_t *payload = &frame[2];
    uint8_t expected = frame[2 + payload_len];
    if (checksum_of(payload, payload_len) != expected) {
        return PROTOCOL_ERR_BAD_CHECKSUM;
    }

    for (uint8_t i = 0; i < payload_len; i++) {
        payload_out[i] = payload[i];
    }
    *payload_len_out = payload_len;
    return PROTOCOL_OK;
}
