// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 */

#include "../sched.h"
#include "trace.h"
#include <linux/sched.h>
#include <linux/seq_buf.h>
#include <linux/trace_seq.h>

#ifndef MAX_CLUSTERS
#define MAX_CLUSTERS 3
#endif

extern unsigned long long sched_clock(void);
extern unsigned int cpu_cycles_to_freq(u64 cycles, u64 exec_time);
extern unsigned int sched_cpu_legacy_freq(int cpu);
extern unsigned int cpu_max_freq(int cpu);

#ifndef CONFIG_SCHED_WALT
static inline void __window_data(u32 *dst, u32 *src)
{
    if (src)
        memcpy(dst, src, nr_cpu_ids * sizeof(u32));
    else
        memset(dst, 0, nr_cpu_ids * sizeof(u32));
}
#endif

const char *__window_print(struct trace_seq *p, const u32 *buf, int buf_len)
{
    int i;
    const char *ret = p->buffer + seq_buf_used(&p->seq);

    for (i = 0; i < buf_len; i++)
        trace_seq_printf(p, "%u ", buf[i]);

    trace_seq_putc(p, 0);

    return ret;
}

static inline s64 __rq_update_sum(struct rq *rq, bool curr, bool new)
{
    if (curr) {
        if (new)
            return rq->rt.curr_runnable_sum;
        else
            return rq->rt.curr_runnable_sum;
    } else {
        if (new)
            return rq->rt.prev_runnable_sum;
        else
            return rq->rt.prev_runnable_sum;
    }
}

static inline s64 __grp_update_sum(struct rq *rq, bool curr, bool new)
{
    if (curr) {
        if (new)
            return rq->grp_time.nt.curr_runnable_sum;
        else
            return rq->grp_time.nt.curr_runnable_sum;
    } else {
        if (new)
            return rq->grp_time.nt.prev_runnable_sum;
        else
            return rq->grp_time.nt.prev_runnable_sum;
    }
}

static inline s64 _get_update_sum(struct rq *rq, enum migrate_types migrate_type,
                  bool src, bool new, bool curr)
{
    switch (migrate_type) {
    case RQ_TO_GROUP:
        if (src)
            return __rq_update_sum(rq, curr, new);
        else
            return __grp_update_sum(rq, curr, new);
    case GROUP_TO_RQ:
        if (src)
            return __grp_update_sum(rq, curr, new);
        else
            return __rq_update_sum(rq, curr, new);
    default:
        WARN_ON_ONCE(1);
        return -1;
    }
}

#define CREATE_TRACE_POINTS
#include "trace.h"
