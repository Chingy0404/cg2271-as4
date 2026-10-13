/*
 * protocol.h — shared UART packet format between MCXC444 and ESP32
 *
 * THIS FILE IS THE SINGLE SOURCE OF TRUTH.
 * Edit it here (common/protocol.h), then run scripts/sync_protocol.sh to copy
 * it into the ESP32 sketch folder and the MCXC444 source folder.
 * Never edit the copies directly — they will be overwritten.
 *
 * Works in both C (MCUXpresso / FreeRTOS) and C++ (Arduino / ESP32).
 *
 * Frame layout (all fields 1 byte unless stated):
 *
 *   +-------+------+-----+----------------------+----------+
 *   | START | TYPE | LEN | PAYLOAD (LEN bytes)  | CHECKSUM |
 *   +-------+------+-----+----------------------+----------+
 *     0xAA                 0..PKT_MAX_PAYLOAD      XOR of TYPE, LEN, PAYLOAD
 *
 * See docs/protocol.md for the message table.
 */
#ifndef AS4_PROTOCOL_H
#define AS4_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PKT_START        0xAA
#define PKT_MAX_PAYLOAD  16
#define PKT_OVERHEAD     4                      /* START + TYPE + LEN + CHECKSUM */
#define PKT_MAX_SIZE     (PKT_MAX_PAYLOAD + PKT_OVERHEAD)

#define AS4_UART_BAUD    9600                   /* must match on both boards */

/* ---- Message types -------------------------------------------------------
 * 0x01-0x0F : MCXC444 -> ESP32
 * 0x10-0x1F : ESP32  -> MCXC444
 * 0x20-0x2F : either direction
 * TODO(team): rename / add types once the project idea is fixed.
 */
typedef enum {
    MSG_SENSOR_DATA  = 0x01,   /* periodic sensor readings              */
    MSG_EVENT        = 0x02,   /* interrupt-driven event (e.g. trigger) */

    MSG_ACTUATOR_CMD = 0x10,   /* cloud result -> drive actuators       */
    MSG_CONFIG       = 0x11,   /* change thresholds / modes remotely    */

    MSG_ACK          = 0x20,
    MSG_NACK         = 0x21,
    MSG_PING         = 0x22
} MsgType;

typedef struct {
    uint8_t type;
    uint8_t len;
    uint8_t payload[PKT_MAX_PAYLOAD];
} Packet;

static inline uint8_t pkt_checksum(uint8_t type, uint8_t len, const uint8_t *payload)
{
    uint8_t c = type ^ len;
    for (uint8_t i = 0; i < len; i++) c ^= payload[i];
    return c;
}

/* Encode a packet into `out` (must hold PKT_MAX_SIZE bytes).
 * Returns number of bytes written, or 0 if len is too large. */
static inline size_t pkt_encode(uint8_t *out, uint8_t type, const uint8_t *payload, uint8_t len)
{
    if (len > PKT_MAX_PAYLOAD) return 0;
    out[0] = PKT_START;
    out[1] = type;
    out[2] = len;
    for (uint8_t i = 0; i < len; i++) out[3 + i] = payload[i];
    out[3 + len] = pkt_checksum(type, len, payload);
    return (size_t)(len + PKT_OVERHEAD);
}

/* ---- Byte-by-byte parser (non-blocking; safe to call from a task) -------- */
typedef enum { PS_WAIT_START, PS_TYPE, PS_LEN, PS_PAYLOAD, PS_CHECKSUM } ParserState;

typedef struct {
    ParserState state;
    uint8_t     idx;
    Packet      pkt;
} PktParser;

static inline void pkt_parser_init(PktParser *p) { p->state = PS_WAIT_START; p->idx = 0; }

/* Feed one received byte.
 * Returns  1 -> a complete, valid packet is in p->pkt
 *          0 -> need more bytes
 *         -1 -> checksum / length error (packet dropped, parser reset) */
static inline int pkt_parse_byte(PktParser *p, uint8_t b)
{
    switch (p->state) {
    case PS_WAIT_START:
        if (b == PKT_START) p->state = PS_TYPE;
        return 0;
    case PS_TYPE:
        p->pkt.type = b;
        p->state = PS_LEN;
        return 0;
    case PS_LEN:
        if (b > PKT_MAX_PAYLOAD) { pkt_parser_init(p); return -1; }
        p->pkt.len = b;
        p->idx = 0;
        p->state = (b == 0) ? PS_CHECKSUM : PS_PAYLOAD;
        return 0;
    case PS_PAYLOAD:
        p->pkt.payload[p->idx++] = b;
        if (p->idx >= p->pkt.len) p->state = PS_CHECKSUM;
        return 0;
    case PS_CHECKSUM: {
        uint8_t ok = (b == pkt_checksum(p->pkt.type, p->pkt.len, p->pkt.payload));
        pkt_parser_init(p);
        return ok ? 1 : -1;
    }
    }
    pkt_parser_init(p);
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif /* AS4_PROTOCOL_H */
