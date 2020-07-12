/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define MALE 0
#define FEMALE 1
#define MATCHMAKER 2

struct lock *whalelock;
struct cv *male_cv;
struct cv *female_cv;
struct cv *matchmaker_cv;
int state[3] = {0, 0, 0}; // State of male, female, and matchmaker
void set_state(void);
void broadcast_all(void);

/*
 * Called by the driver during initialization.
 */

void whalemating_init() {
  whalelock = lock_create("whale");
  male_cv = cv_create("male");
  female_cv = cv_create("female");
  matchmaker_cv = cv_create("matchmaker");
}

/*
 * Called by the driver during teardown.
 */

void whalemating_cleanup() {
  cv_destroy(matchmaker_cv);
  cv_destroy(female_cv);
  cv_destroy(male_cv);
  lock_destroy(whalelock);
}

void male(uint32_t index) {
  male_start(index);
  lock_acquire(whalelock);

  while (state[MALE] != 0) {
    cv_wait(male_cv, whalelock);
  }

  state[MALE] = 1;
  broadcast_all();

  while (state[FEMALE] == 0 || state[MATCHMAKER] == 0) {
    cv_wait(male_cv, whalelock);
  }
  state[MALE] = -1;
  set_state();
  broadcast_all();

  lock_release(whalelock);
  male_end(index);
}

void female(uint32_t index) {
  female_start(index);
  lock_acquire(whalelock);

  while (state[FEMALE] != 0) {
    cv_wait(female_cv, whalelock);
  }
  state[FEMALE] = 1;
  broadcast_all();

  while (state[MALE] == 0 || state[MATCHMAKER] == 0) {
    cv_wait(female_cv, whalelock);
  }
  state[FEMALE] = -1;
  set_state();
  broadcast_all();

  lock_release(whalelock);
  female_end(index);
}

void matchmaker(uint32_t index) {
  matchmaker_start(index);
  lock_acquire(whalelock);

  while (state[MATCHMAKER] != 0) {
    cv_wait(matchmaker_cv, whalelock);
  }
  state[MATCHMAKER] = 1;
  broadcast_all();

  while (state[MALE] == 0 || state[FEMALE] == 0) {
    cv_wait(matchmaker_cv, whalelock);
  }
  state[MATCHMAKER] = -1;
  set_state();
  broadcast_all();

  lock_release(whalelock);
  matchmaker_end(index);
}

void broadcast_all(void) {
  cv_broadcast(male_cv, whalelock);
  cv_broadcast(female_cv, whalelock);
  cv_broadcast(matchmaker_cv, whalelock);
}

void set_state(void) {
  if (state[MALE] == -1 && state[FEMALE] == -1 && state[MATCHMAKER] == -1) {
    state[MALE] = 0;
    state[FEMALE] = 0;
    state[MATCHMAKER] = 0;
  }
}
