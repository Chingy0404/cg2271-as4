/*
 * CG2271 Assignment 4 — MCXC444 FreeRTOS skeleton
 *
 * This is a REFERENCE layout, not a buildable project on its own.
 * Create the real MCUXpresso FreeRTOS project in mcxc444/ (see mcxc444/README.md),
 * then copy the parts you need from here into its source/main.c.
 *
 * Rubric mapping:
 *   - >= 1 ISR ............ PORTx_IRQHandler (interrupt sensor), UARTx_IRQHandler (RX bytes)
 *   - >= 3 tasks .......... sensorPollTask, eventTask, commsRxTask, commsTxTask, actuatorTask
 *   - >= 1 queue .......... txQueue, rxByteQueue, actuatorQueue
 *   - >= 1 sem/mutex ...... eventSem (ISR -> task), stateMutex (shared sensor state)
 *   - polled sensor ....... sensorPollTask (vTaskDelayUntil)
 *   - interrupt sensor .... PORTx_IRQHandler -> eventSem
 *   - preemption + time slicing: FreeRTOSConfig.h
 *         #define configUSE_PREEMPTION    1
 *         #define configUSE_TIME_SLICING  1
 *
 * ISR rules (graded): no blocking calls, no waiting for input, only *FromISR APIs,
 * do the minimum and defer the rest to a task, call portYIELD_FROM_ISR.
 */
#include <stdint.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "protocol.h"

/* ---- Priorities (higher number = higher priority) -------------------------
 * Justify these in the report/demo: event handling and UART RX must not be
 * starved; periodic polling and TX can wait a little. */
#define PRIO_EVENT     (tskIDLE_PRIORITY + 4)
#define PRIO_COMMS_RX  (tskIDLE_PRIORITY + 3)
#define PRIO_ACTUATOR  (tskIDLE_PRIORITY + 3)
#define PRIO_POLL      (tskIDLE_PRIORITY + 2)
#define PRIO_COMMS_TX  (tskIDLE_PRIORITY + 2)

#define POLL_PERIOD_MS 500

/* ---- RTOS objects ---------------------------------------------------------- */
static QueueHandle_t     txQueue;        /* Packet  : tasks -> commsTxTask        */
static QueueHandle_t     rxByteQueue;    /* uint8_t : UART ISR -> commsRxTask     */
static QueueHandle_t     actuatorQueue;  /* Packet  : commsRxTask -> actuatorTask */
static SemaphoreHandle_t eventSem;       /* binary  : GPIO ISR -> eventTask       */
static SemaphoreHandle_t stateMutex;     /* protects sharedState                  */

typedef struct {
    uint16_t sensorA;      /* TODO(team): rename to real sensors */
    uint16_t sensorB;
    uint8_t  mode;         /* set remotely via MSG_CONFIG */
} SharedState;
static SharedState sharedState;

/* ---- Hardware hooks: TODO(owner of each sensor/actuator) ------------------- */
static void     hw_init_uart(void)        { /* TODO: UART clock, pins, baud AS4_UART_BAUD, enable RX IRQ, NVIC prio */ }
static void     hw_init_event_pin(void)   { /* TODO: PORT IRQ on falling/rising edge, NVIC prio */ }
static void     hw_init_sensors(void)     { /* TODO: ADC / GPIO for polled sensors */ }
static void     hw_init_actuators(void)   { /* TODO: TPM PWM / GPIO for actuators */ }
static uint16_t hw_read_sensorA(void)     { return 0; /* TODO */ }
static uint16_t hw_read_sensorB(void)     { return 0; /* TODO */ }
static void     hw_uart_write(const uint8_t *buf, size_t n) { (void)buf; (void)n; /* TODO: polled TX of n bytes */ }
static void     hw_drive_actuator(uint8_t id, uint8_t value) { (void)id; (void)value; /* TODO */ }

/* ---- ISRs ------------------------------------------------------------------ */
/* Rename to the real handler for your pin's port, e.g. PORTA_IRQHandler / PORTC_PORTD_IRQHandler */
void EVENT_PORT_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;
    /* TODO: clear the pin's interrupt flag (ISFR) FIRST */
    xSemaphoreGiveFromISR(eventSem, &woken);
    portYIELD_FROM_ISR(woken);
}

/* Rename to the real handler, e.g. UART2_FLEXIO_IRQHandler */
void COMMS_UART_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;
    /* TODO: while RX data register full: read byte, push to queue (never wait) */
    uint8_t b = 0; /* = UARTx->D; */
    xQueueSendFromISR(rxByteQueue, &b, &woken);
    portYIELD_FROM_ISR(woken);
}

/* ---- Helpers ---------------------------------------------------------------- */
static void queuePacket(uint8_t type, const uint8_t *payload, uint8_t len)
{
    Packet p = { .type = type, .len = len };
    for (uint8_t i = 0; i < len; i++) p.payload[i] = payload[i];
    xQueueSend(txQueue, &p, pdMS_TO_TICKS(10));
}

/* ---- Tasks ------------------------------------------------------------------ */
static void sensorPollTask(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        uint16_t a = hw_read_sensorA();
        uint16_t b = hw_read_sensorB();

        xSemaphoreTake(stateMutex, portMAX_DELAY);
        sharedState.sensorA = a;
        sharedState.sensorB = b;
        xSemaphoreGive(stateMutex);

        uint8_t payload[4] = { a >> 8, a & 0xFF, b >> 8, b & 0xFF };
        queuePacket(MSG_SENSOR_DATA, payload, sizeof(payload));

        vTaskDelayUntil(&last, pdMS_TO_TICKS(POLL_PERIOD_MS));  /* yields */
    }
}

static void eventTask(void *arg)
{
    (void)arg;
    for (;;) {
        xSemaphoreTake(eventSem, portMAX_DELAY);  /* blocks until ISR fires */
        /* TODO: debounce if needed, react locally, then tell the ESP32 */
        uint8_t id = 1;
        queuePacket(MSG_EVENT, &id, 1);
    }
}

static void commsTxTask(void *arg)
{
    (void)arg;
    Packet p;
    uint8_t buf[PKT_MAX_SIZE];
    for (;;) {
        xQueueReceive(txQueue, &p, portMAX_DELAY);
        size_t n = pkt_encode(buf, p.type, p.payload, p.len);
        hw_uart_write(buf, n);  /* only this task touches UART TX -> no mutex needed */
    }
}

static void commsRxTask(void *arg)
{
    (void)arg;
    PktParser parser;
    pkt_parser_init(&parser);
    uint8_t b;
    for (;;) {
        xQueueReceive(rxByteQueue, &b, portMAX_DELAY);
        int r = pkt_parse_byte(&parser, b);
        if (r == 1) {
            Packet *pkt = &parser.pkt;
            switch (pkt->type) {
            case MSG_ACTUATOR_CMD:
                xQueueSend(actuatorQueue, pkt, 0);
                { uint8_t t = pkt->type; queuePacket(MSG_ACK, &t, 1); }
                break;
            case MSG_CONFIG:
                xSemaphoreTake(stateMutex, portMAX_DELAY);
                if (pkt->len) sharedState.mode = pkt->payload[0];
                xSemaphoreGive(stateMutex);
                { uint8_t t = pkt->type; queuePacket(MSG_ACK, &t, 1); }
                break;
            case MSG_PING:
                queuePacket(MSG_PING, NULL, 0);
                break;
            default:
                break;  /* ACK/NACK from ESP32: TODO retry logic if wanted */
            }
        }
    }
}

static void actuatorTask(void *arg)
{
    (void)arg;
    Packet cmd;
    for (;;) {
        xQueueReceive(actuatorQueue, &cmd, portMAX_DELAY);
        if (cmd.len >= 2) hw_drive_actuator(cmd.payload[0], cmd.payload[1]);
    }
}

/* ---- main ------------------------------------------------------------------- */
int main(void)
{
    /* Keep the BOARD_Init* calls MCUXpresso generated for your project here */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    txQueue       = xQueueCreate(8,  sizeof(Packet));
    rxByteQueue   = xQueueCreate(64, sizeof(uint8_t));
    actuatorQueue = xQueueCreate(4,  sizeof(Packet));
    eventSem      = xSemaphoreCreateBinary();
    stateMutex    = xSemaphoreCreateMutex();

    hw_init_sensors();
    hw_init_actuators();
    hw_init_uart();
    hw_init_event_pin();   /* enable IRQs only after RTOS objects exist */

    xTaskCreate(sensorPollTask, "poll",  configMINIMAL_STACK_SIZE + 100, NULL, PRIO_POLL,     NULL);
    xTaskCreate(eventTask,      "event", configMINIMAL_STACK_SIZE + 100, NULL, PRIO_EVENT,    NULL);
    xTaskCreate(commsTxTask,    "tx",    configMINIMAL_STACK_SIZE + 100, NULL, PRIO_COMMS_TX, NULL);
    xTaskCreate(commsRxTask,    "rx",    configMINIMAL_STACK_SIZE + 100, NULL, PRIO_COMMS_RX, NULL);
    xTaskCreate(actuatorTask,   "act",   configMINIMAL_STACK_SIZE + 100, NULL, PRIO_ACTUATOR, NULL);

    vTaskStartScheduler();
    for (;;) {}
}
