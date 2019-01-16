/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * MAM is based on an original implementation & specification by apmi.bsu.by
 * [ITSec Lab]
 *
 * Refer to the LICENSE file for licensing information
 */

#include <unity/unity.h>

#include "mam/v2/sponge/sponge.h"

static bool_t sponge_test_hash(sponge_t *s) {
  bool_t r = 1;
  MAM2_TRITS_DEF0(X0, MAM2_SPONGE_RATE * 3);
  trits_t Xs[3], X;
  MAM2_TRITS_DEF0(Y1, 243);
  MAM2_TRITS_DEF0(Y2, 243);
  size_t n;
  X0 = MAM2_TRITS_INIT(X0, MAM2_SPONGE_RATE * 3);
  Y1 = MAM2_TRITS_INIT(Y1, 243);
  Y2 = MAM2_TRITS_INIT(Y2, 243);

  for (n = 0; n < MAM2_SPONGE_RATE * 3; n += MAM2_SPONGE_RATE / 2) {
    X = trits_take(X0, n);
    trits_set_zero(X);
    sponge_hash(s, X, Y1);

    if (0 && 0 < n) {
      trits_put1(X, trit_add(trits_get1(X), 1));
      sponge_hash(s, X, Y2);
      trits_put1(X, trit_sub(trits_get1(X), 1));
      r = r && !trits_cmp_eq(Y1, Y2);
    }

    Xs[0] = trits_take(X, n / 3);
    Xs[1] = trits_take(trits_drop(X, n / 3), n / 4);
    Xs[2] = trits_drop(X, n / 3 + n / 4);
    sponge_hashn(s, 3, Xs, Y2);
    r = r && trits_cmp_eq(Y1, Y2);
  }

  return r;
}

static bool_t sponge_test_ae(sponge_t *s) {
#define MAM2_SPONGE_TEST_MAX_K 1110
  size_t k, i;
  MAM2_TRITS_DEF0(K, MAM2_SPONGE_KEY_SIZE);
  MAM2_TRITS_DEF0(X, MAM2_SPONGE_TEST_MAX_K);
  MAM2_TRITS_DEF0(Y, MAM2_SPONGE_TEST_MAX_K);
  MAM2_TRITS_DEF0(Z, MAM2_SPONGE_TEST_MAX_K);
  size_t ks[] = {0,
                 1,
                 2,
                 3,
                 4,
                 5,
                 6,
                 242,
                 243,
                 244,
                 485,
                 486,
                 487,
                 MAM2_SPONGE_TEST_MAX_K,
                 MAM2_SPONGE_TEST_MAX_K + 1};
  K = MAM2_TRITS_INIT(K, MAM2_SPONGE_KEY_SIZE);
  X = MAM2_TRITS_INIT(X, MAM2_SPONGE_TEST_MAX_K);
  Y = MAM2_TRITS_INIT(Y, MAM2_SPONGE_TEST_MAX_K);
  Z = MAM2_TRITS_INIT(Z, MAM2_SPONGE_TEST_MAX_K);

  /* init X */
  for (i = 0; i < MAM2_SPONGE_TEST_MAX_K / 3; ++i)
    trits_put3(trits_drop(X, 3 * i), (tryte_t)((i % 27) - 13));
  /* init K */
  trits_set_zero(K);
  trits_from_str(K,
                 "NOPQRSTUVWXYZ9ABCDEFGHIJKLM"
                 "NOPQRSTUVWXYZ9ABCDEFGHIJKLM"
                 "NOPQRSTUVWXYZ9ABCDEFGHIJKLM");
  sponge_init(s);
  sponge_absorb(s, MAM2_SPONGE_CTL_KEY, K);
  sponge_squeeze(s, MAM2_SPONGE_CTL_PRN, K);

  for (i = 0; (k = ks[i++]) <= MAM2_SPONGE_TEST_MAX_K;) {
    X = MAM2_TRITS_INIT(X, k);
    Y = MAM2_TRITS_INIT(Y, k);
    Z = MAM2_TRITS_INIT(Z, k);

    sponge_init(s);
    sponge_absorb(s, MAM2_SPONGE_CTL_KEY, K);
    sponge_encr(s, X, Y); /* Y=E(X) */

    sponge_init(s);
    sponge_absorb(s, MAM2_SPONGE_CTL_KEY, K);
    sponge_decr(s, Y, Z); /* Z=D(E(X)) */
    if (!trits_cmp_eq(X, Z)) return 0;

    sponge_init(s);
    sponge_absorb(s, MAM2_SPONGE_CTL_KEY, K);
    sponge_encr(s, Z, Z); /* Z=E(Z=X) */
    if (!trits_cmp_eq(Y, Z)) return 0;

    sponge_init(s);
    sponge_absorb(s, MAM2_SPONGE_CTL_KEY, K);
    sponge_decr(s, Z, Z); /* Z=D(Z=E(X)) */
    if (!trits_cmp_eq(X, Z)) return 0;
  }

#undef MAM2_SPONGE_TEST_MAX_K
  return 1;
}

bool_t sponge_test(sponge_t *s) {
  return 1 && sponge_test_ae(s) && sponge_test_hash(s);
}

int main(void) {
  UNITY_BEGIN();

  return UNITY_END();
}
