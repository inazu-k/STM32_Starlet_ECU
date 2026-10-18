#include "engine_sync.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static const SyncConfig config = {100, 2000, 3000};

static SyncMarker marker_at(unsigned tooth)
{
    if (tooth % 24 == 0) return SYNC_G1;
    if (tooth % 24 == 12) return SYNC_G2;
    return SYNC_G_NONE;
}

static uint32_t acquire(EngineSync *s, unsigned start, uint32_t tick)
{
    CHECK(engine_sync_init(s, config));
    unsigned first_g = (12 - start % 12) % 12;
    for (unsigned n = 0; n <= first_g + 24; ++n) {
        engine_sync_tooth(s, tick, marker_at(start + n));
        CHECK(s->state == (n < first_g ? SYNC_SEARCH :
              n < first_g + 24 ? SYNC_CANDIDATE : SYNC_LOCKED));
        tick += 1000;
    }
    CHECK(s->tooth == (start + first_g + 24) % 24);
    return tick;
}

int main(void)
{
    EngineSync s;
    /* Every starting tooth, including G2 first and counter wrap. */
    for (unsigned start = 0; start < 24; ++start) {
        uint32_t tick = acquire(&s, start, UINT32_MAX - 15000);
        unsigned tooth = s.tooth;
        for (unsigned n = 1; n <= 240; ++n) {
            /* Variable speed within the configured envelope. */
            engine_sync_tooth(&s, tick, marker_at(tooth + n));
            CHECK(s.state == SYNC_LOCKED);
            CHECK(s.tooth == (tooth + n) % 24);
            tick += (n % 2) ? 600 : 1400;
        }
    }
    /* NE alone never establishes absolute phase. */
    CHECK(engine_sync_init(&s, config));
    for (unsigned n = 0; n < 100; ++n)
        engine_sync_tooth(&s, n * 1000, SYNC_G_NONE);
    CHECK(s.state == SYNC_SEARCH);

    /* Early / late / wrong / duplicate / ambiguous G. */
    const unsigned positions[] = {11, 13, 12, 0, 6};
    const SyncMarker markers[] = {SYNC_G2, SYNC_G2, SYNC_G1, SYNC_G1, SYNC_G_INVALID};
    for (unsigned c = 0; c < 5; ++c) {
        uint32_t tick = acquire(&s, 0, 0);
        for (unsigned n = 1; n <= positions[c]; ++n) {
            engine_sync_tooth(&s, tick, n == positions[c] ? markers[c] : SYNC_G_NONE);
            tick += 1000;
            if (s.state == SYNC_SEARCH) break;
        }
        if (positions[c] == 0) engine_sync_tooth(&s, tick, markers[c]);
        CHECK(s.state == SYNC_SEARCH);
    }
    /* Missing either G, and recovery requiring two confirmations again. */
    for (unsigned start = 0; start <= 12; start += 12) {
        uint32_t tick = acquire(&s, start, 0);
        for (unsigned n = 1; n <= 12; ++n) {
            engine_sync_tooth(&s, tick, SYNC_G_NONE); tick += 1000;
        }
        CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_MISSING_MARKER);
        for (unsigned n = 0; n <= 24; ++n) {
            engine_sync_tooth(&s, tick, marker_at(n)); tick += 1000;
            CHECK(s.state == (n < 24 ? SYNC_CANDIDATE : SYNC_LOCKED));
        }
    }
    /* NE noise, missing tooth, duplicate timestamp and reversed timestamp. */
    const uint32_t gaps[] = {99, 2001, 0, UINT32_MAX};
    for (unsigned n = 0; n < 4; ++n) {
        acquire(&s, 0, 0);
        engine_sync_tooth(&s, s.last_tick + gaps[n], SYNC_G_NONE);
        CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_BAD_PERIOD);
    }
    /* An unobserved missing NE within timing limits is caught at the next G. */
    uint32_t tick = acquire(&s, 0, 0);
    for (unsigned n = 1; n <= 12; ++n) {
        if (n != 5) engine_sync_tooth(&s, tick, marker_at(n));
        tick += 1000;
    }
    CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_BAD_MARKER);
    acquire(&s, 0, UINT32_MAX - 15000);
    engine_sync_poll(&s, s.last_tick + 2999);
    CHECK(s.state == SYNC_LOCKED);
    engine_sync_poll(&s, s.last_tick + 3000);
    CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_TIMEOUT);
    /* Rejected short edges do not postpone the fixed stop deadline. */
    acquire(&s, 0, 0);
    uint32_t last = s.last_tick;
    engine_sync_tooth(&s, last + 50, SYNC_G_NONE);
    CHECK(s.last_tick == last);
    engine_sync_poll(&s, last + config.stop_timeout_ticks);
    CHECK(s.fault == SYNC_TIMEOUT && !s.have_ne);
    for (unsigned n = 0; n <= 24; ++n)
        engine_sync_tooth(&s, last + 10000 + n * 1000, marker_at(n));
    CHECK(s.state == SYNC_LOCKED);
    acquire(&s, 0, 0);
    engine_sync_capture_loss(&s);
    CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_CAPTURE_LOSS);
    CHECK(!engine_sync_init(&s, (SyncConfig){0, 2000, 3000}));
    CHECK(!engine_sync_init(&s, (SyncConfig){100, 3000, 3000}));
    CHECK(!engine_sync_init(&s, (SyncConfig){100, 2000, UINT32_MAX}));
    engine_sync_tooth(&s, 0, SYNC_G1);
    CHECK(s.state == SYNC_SEARCH && s.fault == SYNC_BAD_CONFIG);
    puts("engine_sync: all checks passed");
    return 0;
}
