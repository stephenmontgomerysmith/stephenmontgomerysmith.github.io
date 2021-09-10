/*
Copyright (c) 2000, 2002, 2020 Stephen Montgomery-Smith
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Stephen Montgomery-Smith nor the names of his 
   contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE STEPHEN MONTGOMERY-SMITH AND CONTRIBUTORS 
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STEPHEN MONTGOMERY-SMITH OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

*/

#include "xkbset.h"

void print_controls(XkbControlsPtr ctrls) {
  int i;

  printf("Audible Bell = %s\n", (ctrls->enabled_ctrls & XkbAudibleBellMask) ? "On" : "Off");
  printf("Repeat Keys = %s\n", (ctrls->enabled_ctrls & XkbRepeatKeysMask) ? "On" : "Off");
  printf("Repeat Delay = %d\n", ctrls->repeat_delay);
  printf("Repeat Interval = %d\n", ctrls->repeat_interval);
  printf("Per Key Repeat =");
  for (i=0; i<32; i++) {
    if (i%8 == 0) printf("\n        ");
    printf("%02x", ctrls->per_key_repeat[i]);
  }
  printf("\n");
  printf("Mouse-Keys = %s\n", (ctrls->enabled_ctrls & XkbMouseKeysMask) ? "On" : "Off");
  printf("Mouse-Keys Default Button = %d\n", ctrls->mk_dflt_btn);
  printf("Mouse-Keys Acceleration = %s\n", (ctrls->enabled_ctrls & XkbMouseKeysAccelMask) ? "On" : "Off");
  printf("Mouse-Keys Acceleration Delay = %d\n", ctrls->mk_delay);
  printf("Mouse-Keys Acceleration Interval = %d\n", ctrls->mk_interval);
  printf("Mouse-Keys Acceleration Time to Max = %d\n", ctrls->mk_time_to_max);
  printf("Mouse-Keys Acceleration Max Speed = %d\n", ctrls->mk_max_speed);
  printf("Mouse-Keys Acceleration Curve = %d\n", ctrls->mk_curve);
  printf("Accessibility Features (AccessX) = %s\n", (ctrls->enabled_ctrls & XkbAccessXKeysMask) ? "On" : "Off");
  printf("Sticky-Keys = %s\n", (ctrls->enabled_ctrls & XkbStickyKeysMask) ? "On" : "Off");
  printf("Two Keys Mask = %s\n", (ctrls->ax_options & XkbAX_TwoKeysMask) ? "On" : "Off");
  printf("Latch to Lock Mask = %s\n", (ctrls->ax_options & XkbAX_LatchToLockMask) ? "On" : "Off");
  printf("Slow-Keys = %s\n", (ctrls->enabled_ctrls & XkbSlowKeysMask) ? "On" : "Off");
  printf("Slow Keys Delay = %d\n", ctrls->slow_keys_delay);
  printf("Bounce-Keys = %s\n", (ctrls->enabled_ctrls & XkbBounceKeysMask) ? "On" : "Off");
  printf("Debounce Delay = %d\n", ctrls->debounce_delay);
  printf("AccessX Feedback = %s\n", (ctrls->enabled_ctrls & XkbAccessXFeedbackMask) ? "On" : "Off");
  printf("Use Fixed Pitch Bell = %s\n", (ctrls->ax_options & XkbAX_DumbBellFBMask) ? "On" : "Off");
  printf("Beep when LED changes = %s\n", (ctrls->ax_options & XkbAX_IndicatorFBMask) ? "On" : "Off");
  printf("Beep on Controls on/off = %s\n", (ctrls->ax_options & XkbAX_FeatureFBMask) ? "On" : "Off");
  printf("Beep if Slow/Bounce-Keys about to be turned off = %s\n", (ctrls->ax_options & XkbAX_SlowWarnFBMask) ? "On" : "Off");
  printf("Beep on Slow-Key Press = %s\n", (ctrls->ax_options & XkbAX_SKPressFBMask) ? "On" : "Off");
  printf("Beep on Slow-Key Accept = %s\n", (ctrls->ax_options & XkbAX_SKAcceptFBMask) ? "On" : "Off");
  printf("Beep on Slow-Key Reject = %s\n", (ctrls->ax_options & XkbAX_SKRejectFBMask) ? "On" : "Off");
  printf("Beep on Slow-Key Release = %s\n", (ctrls->ax_options & XkbAX_SKReleaseFBMask) ? "On" : "Off");
  printf("Beep on Bounce-Key Reject = %s\n", (ctrls->ax_options & XkbAX_BKRejectFBMask) ? "On" : "Off");
  printf("Beep on Sticky-Key Actions = %s\n", (ctrls->ax_options & XkbAX_StickyKeysFBMask) ? "On" : "Off");
  printf("Keyboard Overlay 1 = %s\n", (ctrls->enabled_ctrls & XkbOverlay1Mask) ? "On" : "Off");
  printf("Keyboard Overlay 2 = %s\n", (ctrls->enabled_ctrls & XkbOverlay2Mask) ? "On" : "Off");
  printf("Groups Wrap Type = ");
  if ((ctrls->groups_wrap & 0xF0) == XkbRedirectIntoRange) printf("redirect\n");
  else if ((ctrls->groups_wrap & 0xF0) == XkbClampIntoRange) printf("clamp\n");
  else if ((ctrls->groups_wrap & 0xF0) == XkbWrapIntoRange) printf("wrap\n");
  else printf("undefined\n");
  printf("Groups Wrap Value = %d\n", ctrls->groups_wrap & 0x0F);
  printf("Ignore Group Lock = %s\n", (ctrls->enabled_ctrls & XkbIgnoreGroupLockMask) ? "On" : "Off");
  printf("Nullify Shift = %s\n", (ctrls->internal.real_mods & ShiftMask) ? "On" : "Off");
  printf("Nullify Caps-Lock = %s\n", (ctrls->internal.real_mods & LockMask) ? "On" : "Off");
  printf("Nullify Control = %s\n", (ctrls->internal.real_mods & ControlMask) ? "On" : "Off");
  printf("Nullify Mod1 = %s\n", (ctrls->internal.real_mods & Mod1Mask) ? "On" : "Off");
  printf("Nullify Mod2 = %s\n", (ctrls->internal.real_mods & Mod2Mask) ? "On" : "Off");
  printf("Nullify Mod3 = %s\n", (ctrls->internal.real_mods & Mod3Mask) ? "On" : "Off");
  printf("Nullify Mod4 = %s\n", (ctrls->internal.real_mods & Mod4Mask) ? "On" : "Off");
  printf("Nullify Mod5 = %s\n", (ctrls->internal.real_mods & Mod5Mask) ? "On" : "Off");
  printf("Ignore Lock of Shift = %s\n", (ctrls->ignore_lock.real_mods & ShiftMask) ? "On" : "Off");
  printf("Ignore Lock of Caps-Lock = %s\n", (ctrls->ignore_lock.real_mods & LockMask) ? "On" : "Off");
  printf("Ignore Lock of Control = %s\n", (ctrls->ignore_lock.real_mods & ControlMask) ? "On" : "Off");
  printf("Ignore Lock of Mod1 = %s\n", (ctrls->ignore_lock.real_mods & Mod1Mask) ? "On" : "Off");
  printf("Ignore Lock of Mod2 = %s\n", (ctrls->ignore_lock.real_mods & Mod2Mask) ? "On" : "Off");
  printf("Ignore Lock of Mod3 = %s\n", (ctrls->ignore_lock.real_mods & Mod3Mask) ? "On" : "Off");
  printf("Ignore Lock of Mod4 = %s\n", (ctrls->ignore_lock.real_mods & Mod4Mask) ? "On" : "Off");
  printf("Ignore Lock of Mod5 = %s\n", (ctrls->ignore_lock.real_mods & Mod5Mask) ? "On" : "Off");

}

void print_controls_in_line(XkbControlsPtr ctrls) {
  int i;
  char *s;
  int print_nibble;

  printf("xkbset");
  printf(" %sb", (ctrls->enabled_ctrls & XkbAudibleBellMask) ? "" : "-");
  printf(" %sr", (ctrls->enabled_ctrls & XkbRepeatKeysMask) ? "" : "-");
  printf(" rate");
  printf(" %d", ctrls->repeat_delay);
  printf(" %d", ctrls->repeat_interval);
  printf(" perkeyrepeat");
  printf(" ");
  for (i=0; i<32; i++)
    printf("%02x", ctrls->per_key_repeat[i]);
  printf(" %sm", (ctrls->enabled_ctrls & XkbMouseKeysMask) ? "" : "-");
  printf(" %d", ctrls->mk_dflt_btn);
  printf(" %sma", (ctrls->enabled_ctrls & XkbMouseKeysAccelMask) ? "" : "-");
  printf(" %d", ctrls->mk_delay);
  printf(" %d", ctrls->mk_interval);
  printf(" %d", ctrls->mk_time_to_max);
  printf(" %d", ctrls->mk_max_speed);
  printf(" %d", ctrls->mk_curve);
  printf(" %sa", (ctrls->enabled_ctrls & XkbAccessXKeysMask) ? "" : "-");
  printf(" %sst", (ctrls->enabled_ctrls & XkbStickyKeysMask) ? "" : "-");
  printf(" %stwokey", (ctrls->ax_options & XkbAX_TwoKeysMask) ? "" : "-");
  printf(" %slatchlock", (ctrls->ax_options & XkbAX_LatchToLockMask) ? "" : "-");
  printf(" %ssl", (ctrls->enabled_ctrls & XkbSlowKeysMask) ? "" : "-");
  printf(" %d", ctrls->slow_keys_delay);
  printf(" %sbo", (ctrls->enabled_ctrls & XkbBounceKeysMask) ? "" : "-");
  printf(" %d", ctrls->debounce_delay);
  printf(" %sf", (ctrls->enabled_ctrls & XkbAccessXFeedbackMask) ? "" : "-");
  printf(" %sdumbbell", (ctrls->ax_options & XkbAX_DumbBellFBMask) ? "" : "-");
  printf(" %sled", (ctrls->ax_options & XkbAX_IndicatorFBMask) ? "" : "-");
  printf(" %sfeature", (ctrls->ax_options & XkbAX_FeatureFBMask) ? "" : "-");
  printf(" %sslowwarn", (ctrls->ax_options & XkbAX_SlowWarnFBMask) ? "" : "-");
  printf(" %sslowpress", (ctrls->ax_options & XkbAX_SKPressFBMask) ? "" : "-");
  printf(" %sslowaccept", (ctrls->ax_options & XkbAX_SKAcceptFBMask) ? "" : "-");
  printf(" %sslowreject", (ctrls->ax_options & XkbAX_SKRejectFBMask) ? "" : "-");
  printf(" %sslowrelease", (ctrls->ax_options & XkbAX_SKReleaseFBMask) ? "" : "-");
  printf(" %sbouncereject", (ctrls->ax_options & XkbAX_BKRejectFBMask) ? "" : "-");
  printf(" %sstickybeep", (ctrls->ax_options & XkbAX_StickyKeysFBMask) ? "" : "-");
  printf(" %sov1", (ctrls->enabled_ctrls & XkbOverlay1Mask) ? "" : "-");
  printf(" %sov2", (ctrls->enabled_ctrls & XkbOverlay2Mask) ? "" : "-");
  s = NULL;
  if ((ctrls->groups_wrap & 0xF0) == XkbRedirectIntoRange) s = "redirect";
  else if ((ctrls->groups_wrap & 0xF0) == XkbClampIntoRange) s = "clamp";
  else if ((ctrls->groups_wrap & 0xF0) == XkbWrapIntoRange) s = "wrap";
  if (s != NULL) printf(" groupswrap %s", s);
  print_nibble = 0;
  if ((ctrls->groups_wrap & 0xF0) == XkbRedirectIntoRange) print_nibble = 1;
  else if ((ctrls->groups_wrap & 0xF0) == XkbClampIntoRange) print_nibble = 1;
  else if ((ctrls->groups_wrap & 0xF0) == XkbWrapIntoRange) print_nibble = 1;
  if (print_nibble) printf(" %d", ctrls->groups_wrap & 0x0F);
  printf(" %signoregrouplock", (ctrls->enabled_ctrls & XkbIgnoreGroupLockMask) ? "" : "-");
  printf(" nullify");
  printf(" %sshift", (ctrls->internal.real_mods & ShiftMask) ? "" : "-");
  printf(" %slock", (ctrls->internal.real_mods & LockMask) ? "" : "-");
  printf(" %scontrol", (ctrls->internal.real_mods & ControlMask) ? "" : "-");
  printf(" %smod1", (ctrls->internal.real_mods & Mod1Mask) ? "" : "-");
  printf(" %smod2", (ctrls->internal.real_mods & Mod2Mask) ? "" : "-");
  printf(" %smod3", (ctrls->internal.real_mods & Mod3Mask) ? "" : "-");
  printf(" %smod4", (ctrls->internal.real_mods & Mod4Mask) ? "" : "-");
  printf(" %smod5", (ctrls->internal.real_mods & Mod5Mask) ? "" : "-");
  printf(" ignorelock");
  printf(" %sshift", (ctrls->ignore_lock.real_mods & ShiftMask) ? "" : "-");
  printf(" %slock", (ctrls->ignore_lock.real_mods & LockMask) ? "" : "-");
  printf(" %scontrol", (ctrls->ignore_lock.real_mods & ControlMask) ? "" : "-");
  printf(" %smod1", (ctrls->ignore_lock.real_mods & Mod1Mask) ? "" : "-");
  printf(" %smod2", (ctrls->ignore_lock.real_mods & Mod2Mask) ? "" : "-");
  printf(" %smod3", (ctrls->ignore_lock.real_mods & Mod3Mask) ? "" : "-");
  printf(" %smod4", (ctrls->ignore_lock.real_mods & Mod4Mask) ? "" : "-");
  printf(" %smod5", (ctrls->ignore_lock.real_mods & Mod5Mask) ? "" : "-");

  printf("\n");
}

void print_expire_controls(XkbControlsPtr ctrls) {
  printf("AccessX Timeout = %d\n", ctrls->ax_timeout);
  printf("Upon Expiry Audible Bell will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbAudibleBellMask) ? ((ctrls->axt_ctrls_values & XkbAudibleBellMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Repeat Keys will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbRepeatKeysMask) ? ((ctrls->axt_ctrls_values & XkbRepeatKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Mouse-Keys will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbMouseKeysMask) ? ((ctrls->axt_ctrls_values & XkbMouseKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Mouse-Keys Acceleration will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbMouseKeysAccelMask) ? ((ctrls->axt_ctrls_values & XkbMouseKeysAccelMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Accessibility Features (AccessX) will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbAccessXKeysMask) ? ((ctrls->axt_ctrls_values & XkbAccessXKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Sticky-Keys will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbStickyKeysMask) ? ((ctrls->axt_ctrls_values & XkbStickyKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Two Keys Mask will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_TwoKeysMask) ? ((ctrls->axt_opts_values & XkbAX_TwoKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Latch to Lock Mask will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_LatchToLockMask) ? ((ctrls->axt_opts_values & XkbAX_LatchToLockMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Slow-Keys will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbSlowKeysMask) ? ((ctrls->axt_ctrls_values & XkbSlowKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Bounce-Keys will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbBounceKeysMask) ? ((ctrls->axt_ctrls_values & XkbBounceKeysMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry AccessX Feedback will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbAccessXFeedbackMask) ? ((ctrls->axt_ctrls_values & XkbAccessXFeedbackMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Use Fixed Pitch Bell will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_DumbBellFBMask) ? ((ctrls->axt_opts_values & XkbAX_DumbBellFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep when LED changes will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_IndicatorFBMask) ? ((ctrls->axt_opts_values & XkbAX_IndicatorFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Controls on/off will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_FeatureFBMask) ? ((ctrls->axt_opts_values & XkbAX_FeatureFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep if Slow/Bounce-Keys about to be turned off will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_SlowWarnFBMask) ? ((ctrls->axt_opts_values & XkbAX_SlowWarnFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Slow-Key Press will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_SKPressFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKPressFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Slow-Key Accept will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_SKAcceptFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKAcceptFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Slow-Key Reject will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_SKRejectFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKRejectFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Slow-Key Release will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_SKReleaseFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKReleaseFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Bounce-Key Reject will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_BKRejectFBMask) ? ((ctrls->axt_opts_values & XkbAX_BKRejectFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Beep on Sticky-Key Actions will be: %s\n", 
    (ctrls->axt_opts_mask & XkbAX_StickyKeysFBMask) ? ((ctrls->axt_opts_values & XkbAX_StickyKeysFBMask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Keyboard Overlay 1 will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbOverlay1Mask) ? ((ctrls->axt_ctrls_values & XkbOverlay1Mask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Keyboard Overlay 2 will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbOverlay2Mask) ? ((ctrls->axt_ctrls_values & XkbOverlay2Mask) ? "On" : "Off") : "Unchanged");
  printf("Upon Expiry Ignore Group Lock will be: %s\n", 
    (ctrls->axt_ctrls_mask & XkbIgnoreGroupLockMask) ? ((ctrls->axt_ctrls_values & XkbIgnoreGroupLockMask) ? "On" : "Off") : "Unchanged");

}

void print_expire_controls_in_line(XkbControlsPtr ctrls) {
  printf("xkbset exp %d", ctrls->ax_timeout);
  printf(" %sb", 
    (ctrls->axt_ctrls_mask & XkbAudibleBellMask) ? ((ctrls->axt_ctrls_values & XkbAudibleBellMask) ? "" : "-") : "=");
  printf(" %sr", 
    (ctrls->axt_ctrls_mask & XkbRepeatKeysMask) ? ((ctrls->axt_ctrls_values & XkbRepeatKeysMask) ? "" : "-") : "=");
  printf(" %sm", 
    (ctrls->axt_ctrls_mask & XkbMouseKeysMask) ? ((ctrls->axt_ctrls_values & XkbMouseKeysMask) ? "" : "-") : "=");
  printf(" %sma", 
    (ctrls->axt_ctrls_mask & XkbMouseKeysAccelMask) ? ((ctrls->axt_ctrls_values & XkbMouseKeysAccelMask) ? "" : "-") : "=");
  printf(" %sa", 
    (ctrls->axt_ctrls_mask & XkbAccessXKeysMask) ? ((ctrls->axt_ctrls_values & XkbAccessXKeysMask) ? "" : "-") : "=");
  printf(" %sst", 
    (ctrls->axt_ctrls_mask & XkbStickyKeysMask) ? ((ctrls->axt_ctrls_values & XkbStickyKeysMask) ? "" : "-") : "=");
  printf(" %stwokey", 
    (ctrls->axt_opts_mask & XkbAX_TwoKeysMask) ? ((ctrls->axt_opts_values & XkbAX_TwoKeysMask) ? "" : "-") : "=");
  printf(" %slatchlock", 
    (ctrls->axt_opts_mask & XkbAX_LatchToLockMask) ? ((ctrls->axt_opts_values & XkbAX_LatchToLockMask) ? "" : "-") : "=");
  printf(" %ssl", 
    (ctrls->axt_ctrls_mask & XkbSlowKeysMask) ? ((ctrls->axt_ctrls_values & XkbSlowKeysMask) ? "" : "-") : "=");
  printf(" %sbo", 
    (ctrls->axt_ctrls_mask & XkbBounceKeysMask) ? ((ctrls->axt_ctrls_values & XkbBounceKeysMask) ? "" : "-") : "=");
  printf(" %sf", 
    (ctrls->axt_ctrls_mask & XkbAccessXFeedbackMask) ? ((ctrls->axt_ctrls_values & XkbAccessXFeedbackMask) ? "" : "-") : "=");
  printf(" %sdumbbell", 
    (ctrls->axt_opts_mask & XkbAX_DumbBellFBMask) ? ((ctrls->axt_opts_values & XkbAX_DumbBellFBMask) ? "" : "-") : "=");
  printf(" %sled", 
    (ctrls->axt_opts_mask & XkbAX_IndicatorFBMask) ? ((ctrls->axt_opts_values & XkbAX_IndicatorFBMask) ? "" : "-") : "=");
  printf(" %sfeature", 
    (ctrls->axt_opts_mask & XkbAX_FeatureFBMask) ? ((ctrls->axt_opts_values & XkbAX_FeatureFBMask) ? "" : "-") : "=");
  printf(" %sslowwarn", 
    (ctrls->axt_opts_mask & XkbAX_SlowWarnFBMask) ? ((ctrls->axt_opts_values & XkbAX_SlowWarnFBMask) ? "" : "-") : "=");
  printf(" %sslowpress", 
    (ctrls->axt_opts_mask & XkbAX_SKPressFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKPressFBMask) ? "" : "-") : "=");
  printf(" %sslowaccept", 
    (ctrls->axt_opts_mask & XkbAX_SKAcceptFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKAcceptFBMask) ? "" : "-") : "=");
  printf(" %sslowreject", 
    (ctrls->axt_opts_mask & XkbAX_SKRejectFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKRejectFBMask) ? "" : "-") : "=");
  printf(" %sslowrelease", 
    (ctrls->axt_opts_mask & XkbAX_SKReleaseFBMask) ? ((ctrls->axt_opts_values & XkbAX_SKReleaseFBMask) ? "" : "-") : "=");
  printf(" %sbouncereject", 
    (ctrls->axt_opts_mask & XkbAX_BKRejectFBMask) ? ((ctrls->axt_opts_values & XkbAX_BKRejectFBMask) ? "" : "-") : "=");
  printf(" %sstickybeep", 
    (ctrls->axt_opts_mask & XkbAX_StickyKeysFBMask) ? ((ctrls->axt_opts_values & XkbAX_StickyKeysFBMask) ? "" : "-") : "=");
  printf(" %sov1", 
    (ctrls->axt_ctrls_mask & XkbOverlay1Mask) ? ((ctrls->axt_ctrls_values & XkbOverlay1Mask) ? "" : "-") : "=");
  printf(" %sov2", 
    (ctrls->axt_ctrls_mask & XkbOverlay2Mask) ? ((ctrls->axt_ctrls_values & XkbOverlay2Mask) ? "" : "-") : "=");
  printf(" %signoregrouplock", 
    (ctrls->axt_ctrls_mask & XkbIgnoreGroupLockMask) ? ((ctrls->axt_ctrls_values & XkbIgnoreGroupLockMask) ? "" : "-") : "=");

  printf("\n");
}
