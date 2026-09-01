#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/*
 * A small UART-style framed protocol:
 *
 *   byte 0        : START (0x7E)
 *   byte 1        : LENGTH (payload byte count)
 *   byte 2..2+N-1 : PAYLOAD (N = LENGTH bytes)
 *   byte 2+N      : CHECKSUM (XOR of every payload byte)
 *
 * This is deliberately the same shape a firmware-to-firmware UART link
 * would use: a fixed start byte for resynchronization, an explicit length
 * so a receiver knows how many payload bytes to expect, and a checksum so
 * a corrupted frame is detected rather than silently accepted.
 */

#define PROTOCOL_START_BYTE   0x7Eu
#define PROTOCOL_MAX_PAYLOAD  16u
#define PROTOCOL_MAX_FRAME    (1u + 1u + PROTOCOL_MAX_PAYLOAD + 1u)

typedef enum {
    PROTOCOL_OK = 0,
    PROTOCOL_ERR_TOO_SHORT,
    PROTOCOL_ERR_BAD_START,
    PROTOCOL_ERR_LENGTH_MISMATCH,
    PROTOCOL_ERR_BAD_CHECKSUM,
} protocol_status_t;

/* Encodes payload[0..payload_len-1] into out as a framed message.
 * Returns the total frame length in bytes, or 0 if it would not fit in
 * out_cap or PROTOCOL_MAX_PAYLOAD. */
size_t protocol_encode(uint8_t *out, size_t out_cap, const uint8_t *payload, uint8_t payload_len);

/* Decodes a frame previously produced by protocol_encode (or corrupted
 * data pretending to be one). On PROTOCOL_OK, payload_out[0..*payload_len_out-1]
 * holds the recovered payload. payload_out must be at least
 * PROTOCOL_MAX_PAYLOAD bytes. */
protocol_status_t protocol_decode(const uint8_t *frame, size_t frame_len,
                                   uint8_t *payload_out, uint8_t *payload_len_out);

#endif /* PROTOCOL_H */
