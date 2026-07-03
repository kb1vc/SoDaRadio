/*
 * vendor/hamlib/rig.h
 *
 * A minimal stand-in for <hamlib/rig.h>.
 *
 * SoDaRadio's HamlibServer / HamlibHandler pretend to be a rig for the
 * hamlib "rigctld" wire protocol, so they need the enum / macro codes that
 * appear on that wire (RIG_MODE_*, RIG_VFO_*, RIG_ANT_*, RIG_EINVAL, etc.)
 * but they never call a hamlib function.  Rather than require libhamlib-dev
 * on every build host, we ship the handful of constants we actually use.
 *
 * If SoDaRadio ever calls into libhamlib, delete this file and put back
 * the pkg-config check in the top-level CMakeLists.txt.
 *
 * Values below mirror hamlib 4.x (upstream file: include/hamlib/rig.h)
 * and are on the wire protocol -- they must not be renumbered.
 */

#ifndef SODA_VENDOR_HAMLIB_RIG_H
#define SODA_VENDOR_HAMLIB_RIG_H

/* rigctld protocol error return value */
#define RIG_EINVAL   1

/* VFO bitmask (32-bit) */
#define RIG_VFO_N(n) (1u << (n))
#define RIG_VFO_A    RIG_VFO_N(0)
#define RIG_VFO_B    RIG_VFO_N(1)

/* Antenna bitmask */
#define RIG_ANT_N(n) (1u << (n))
#define RIG_ANT_1    RIG_ANT_N(0)
#define RIG_ANT_2    RIG_ANT_N(1)

/* Mode bitmask (64-bit) */
#define RIG_MODE_FLAG(bit) (1ull << (bit))
#define RIG_MODE_AM    RIG_MODE_FLAG(0)
#define RIG_MODE_CW    RIG_MODE_FLAG(1)
#define RIG_MODE_USB   RIG_MODE_FLAG(2)
#define RIG_MODE_LSB   RIG_MODE_FLAG(3)
#define RIG_MODE_FM    RIG_MODE_FLAG(5)
#define RIG_MODE_WFM   RIG_MODE_FLAG(6)
#define RIG_MODE_CWR   RIG_MODE_FLAG(7)

/* No-op capability masks -- SoDaRadio doesn't implement functions, levels, or parms */
#define RIG_FUNC_NONE  0
#define RIG_LEVEL_NONE 0
#define RIG_PARM_NONE  0

#endif /* SODA_VENDOR_HAMLIB_RIG_H */
