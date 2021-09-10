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

Bool get_arguments(int argc, char *argv[], XkbControlsPtr ctrls, unsigned int *mask)
{
  char char_array_32[32];
  int inc, i = 1;
  int val1, val2, val3, val4, val5;

  *mask = 0;
  while (i<argc) {
    if ((i+1 <= argc && strcmp(argv[i], "bell") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "b") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bell") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-b") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbAudibleBellMask;
      else                   ctrls->enabled_ctrls |= XkbAudibleBellMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
    }
    else if ((i+2 <= argc && strcmp(argv[i], "repeatkeys") == 0 && get_number(argv[i+1], 0, 255, &val1) && (inc = 2)) ||
        (i+2 <= argc && strcmp(argv[i], "r") == 0 && get_number(argv[i+1], 0, 255, &val1) && (inc = 2)) ||
        (i+2 <= argc && strcmp(argv[i], "-repeatkeys") == 0 && get_number(argv[i+1], 0, 255, &val1) && (inc = 2)) ||
        (i+2 <= argc && strcmp(argv[i], "-r") == 0 && get_number(argv[i+1], 0, 255, &val1) && (inc = 2))) {
      if (argv[i][0] == '-') ctrls->per_key_repeat[val1>>3] &= ~(1<<(val1&7));
      else                   ctrls->per_key_repeat[val1>>3] |= (1<<(val1&7));
      *mask |= XkbPerKeyRepeatMask;
      i += inc;
    }
    else if ((i+2 <= argc && strcmp(argv[i], "perkeyrepeat") == 0 && get_64_hex(argv[i+1], char_array_32) && (inc = 2))) {
      memcpy(ctrls->per_key_repeat, char_array_32, 32);
      *mask |= XkbPerKeyRepeatMask;
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "repeatkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "r") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-repeatkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-r") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbRepeatKeysMask;
      else                   ctrls->enabled_ctrls |= XkbRepeatKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      if ((i+2 <= argc && strcmp(argv[i], "rate") == 0 && get_number(argv[i+1], 0, 65535, &val1) && (inc = 2))) {
        ctrls->repeat_delay = val1;
        *mask |= XkbRepeatKeysMask;
        i += inc;
        if ((i+1 <= argc && get_number(argv[i], 0, 65535, &val1) && (inc = 1))) {
          ctrls->repeat_interval = val1;
          i += inc;
        }
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "mousekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "m") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-mousekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-m") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbMouseKeysMask;
      else                   ctrls->enabled_ctrls |= XkbMouseKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      if ((i+1 <= argc && get_number(argv[i], 0, 255, &val1) && (inc = 1))) {
        ctrls->mk_dflt_btn = val1;
        *mask |= XkbMouseKeysMask;
        i += inc;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "mousekeysaccel") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ma") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-mousekeysaccel") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ma") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbMouseKeysAccelMask;
      else                   ctrls->enabled_ctrls |= XkbMouseKeysAccelMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      if ((i+5 <= argc && get_number(argv[i], 0, 65535, &val1) && get_number(argv[i+1], 0, 65535, &val2) && get_number(argv[i+2], 0, 65535, &val3) && get_number(argv[i+3], 0, 65535, &val4) && get_number(argv[i+4], -32767, 32767, &val5) && (inc = 5))) {
        ctrls->mk_delay = val1;
        ctrls->mk_interval = val2;
        ctrls->mk_time_to_max = val3;
        ctrls->mk_max_speed = val4;
        ctrls->mk_curve = val5;
        *mask |= XkbMouseKeysAccelMask;
        i += inc;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "accessx") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "a") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-accessx") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-a") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;
      else                   ctrls->enabled_ctrls |= XkbAccessXKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "sticky") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "st") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-sticky") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-st") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbStickyKeysMask;
      else                   ctrls->enabled_ctrls |= XkbStickyKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "twokey") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-twokey") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_TwoKeysMask;
          else                   ctrls->ax_options |= XkbAX_TwoKeysMask;
          *mask |= XkbStickyKeysMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "latchlock") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-latchlock") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_LatchToLockMask;
          else                   ctrls->ax_options |= XkbAX_LatchToLockMask;
          *mask |= XkbStickyKeysMask;
          i += inc;
        }
        else break;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "slowkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "sl") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-slowkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-sl") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
      else                   ctrls->enabled_ctrls |= XkbSlowKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      if ((i+1 <= argc && get_number(argv[i], 0, 65535, &val1) && (inc = 1))) {
        ctrls->slow_keys_delay = val1;
        *mask |= XkbSlowKeysMask;
        i += inc;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "bouncekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "bo") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bouncekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bo") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
      else                   ctrls->enabled_ctrls |= XkbBounceKeysMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      if ((i+1 <= argc && get_number(argv[i], 0, 65535, &val1) && (inc = 1))) {
        ctrls->debounce_delay = val1;
        *mask |= XkbBounceKeysMask;
        i += inc;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "feedback") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "f") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-feedback") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-f") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbAccessXFeedbackMask;
      else                   ctrls->enabled_ctrls |= XkbAccessXFeedbackMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "dumbbell") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-dumbbell") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_DumbBellFBMask;
          else                   ctrls->ax_options |= XkbAX_DumbBellFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "led") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-led") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_IndicatorFBMask;
          else                   ctrls->ax_options |= XkbAX_IndicatorFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "feature") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-feature") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_FeatureFBMask;
          else                   ctrls->ax_options |= XkbAX_FeatureFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowwarn") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowwarn") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_SlowWarnFBMask;
          else                   ctrls->ax_options |= XkbAX_SlowWarnFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowpress") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowpress") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_SKPressFBMask;
          else                   ctrls->ax_options |= XkbAX_SKPressFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowaccept") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowaccept") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_SKAcceptFBMask;
          else                   ctrls->ax_options |= XkbAX_SKAcceptFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowreject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowreject") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_SKRejectFBMask;
          else                   ctrls->ax_options |= XkbAX_SKRejectFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowrelease") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowrelease") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_SKReleaseFBMask;
          else                   ctrls->ax_options |= XkbAX_SKReleaseFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "bouncereject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-bouncereject") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_BKRejectFBMask;
          else                   ctrls->ax_options |= XkbAX_BKRejectFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "stickybeep") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-stickybeep") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ax_options &= ~XkbAX_StickyKeysFBMask;
          else                   ctrls->ax_options |= XkbAX_StickyKeysFBMask;
          *mask |= XkbAccessXFeedbackMask;
          i += inc;
        }
        else break;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "overlay1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ov1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-overlay1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ov1") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbOverlay1Mask;
      else                   ctrls->enabled_ctrls |= XkbOverlay1Mask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "overlay2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ov2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-overlay2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ov2") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbOverlay2Mask;
      else                   ctrls->enabled_ctrls |= XkbOverlay2Mask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
    }
    else if ((i+2 <= argc && strcmp(argv[i], "groupswrap") == 0 && 1 && (inc = 2))) {
      if (strcmp(argv[i+1],  "redirect") == 0) {
        ctrls->groups_wrap &= 0x0F;
        ctrls->groups_wrap |= XkbRedirectIntoRange;
      }
      else if (strcmp(argv[i+1],  " clamp") == 0) {
        ctrls->groups_wrap &= 0x0F;
        ctrls->groups_wrap |= XkbClampIntoRange;
      }
      else if (strcmp(argv[i+1],  " wrap") == 0) {
        ctrls->groups_wrap &= 0x0F;
        ctrls->groups_wrap |= XkbWrapIntoRange;
      }
      else return 0;
      *mask |= XkbGroupsWrapMask;
      i += inc;
      if ((i+1 <= argc && get_number(argv[i], 0, 255, &val1) && (inc = 1))) {
        ctrls->groups_wrap &= 0xF0;
        ctrls->groups_wrap |= 0x0F & val1;
        i += inc;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "ignoregrouplock") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ignoregrouplock") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') ctrls->enabled_ctrls &= ~XkbIgnoreGroupLockMask;
      else                   ctrls->enabled_ctrls |= XkbIgnoreGroupLockMask;
      *mask |= XkbControlsEnabledMask;
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "nullify") == 0 && (inc = 1))) {
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "shift") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-shift") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~ShiftMask;
          else                   ctrls->internal.real_mods |= ShiftMask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "lock") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-lock") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~LockMask;
          else                   ctrls->internal.real_mods |= LockMask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "control") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-control") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~ControlMask;
          else                   ctrls->internal.real_mods |= ControlMask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod1") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod1") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~Mod1Mask;
          else                   ctrls->internal.real_mods |= Mod1Mask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod2") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod2") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~Mod2Mask;
          else                   ctrls->internal.real_mods |= Mod2Mask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod3") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod3") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~Mod3Mask;
          else                   ctrls->internal.real_mods |= Mod3Mask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod4") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod4") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~Mod4Mask;
          else                   ctrls->internal.real_mods |= Mod4Mask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod5") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod5") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->internal.real_mods &= ~Mod5Mask;
          else                   ctrls->internal.real_mods |= Mod5Mask;
          *mask |= XkbInternalModsMask;
          i += inc;
        }
        else break;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "ignorelock") == 0 && (inc = 1))) {
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "shift") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-shift") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~ShiftMask;
          else                   ctrls->ignore_lock.real_mods |= ShiftMask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "lock") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-lock") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~LockMask;
          else                   ctrls->ignore_lock.real_mods |= LockMask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "control") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-control") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~ControlMask;
          else                   ctrls->ignore_lock.real_mods |= ControlMask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod1") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod1") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~Mod1Mask;
          else                   ctrls->ignore_lock.real_mods |= Mod1Mask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod2") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod2") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~Mod2Mask;
          else                   ctrls->ignore_lock.real_mods |= Mod2Mask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod3") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod3") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~Mod3Mask;
          else                   ctrls->ignore_lock.real_mods |= Mod3Mask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod4") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod4") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~Mod4Mask;
          else                   ctrls->ignore_lock.real_mods |= Mod4Mask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "mod5") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-mod5") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') ctrls->ignore_lock.real_mods &= ~Mod5Mask;
          else                   ctrls->ignore_lock.real_mods |= Mod5Mask;
          *mask |= XkbIgnoreLockModsMask;
          i += inc;
        }
        else break;
      }
    }
    else return 0;
  }
  return 1;
}

Bool get_expire_arguments(int argc, char *argv[], XkbControlsPtr ctrls, unsigned int *mask)
{
  int inc, i = 2;
  int val1;

  *mask = XkbAccessXTimeoutMask;
  while (i<argc) {
    if ((i+1 <= argc && strcmp(argv[i], "bell") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "b") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bell") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-b") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=bell") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=b") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbAudibleBellMask;
        ctrls->axt_ctrls_values &= ~XkbAudibleBellMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbAudibleBellMask;
        ctrls->axt_ctrls_values &= ~XkbAudibleBellMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbAudibleBellMask;
        ctrls->axt_ctrls_values |= XkbAudibleBellMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "repeatkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "r") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-repeatkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-r") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=repeatkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=r") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbRepeatKeysMask;
        ctrls->axt_ctrls_values &= ~XkbRepeatKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbRepeatKeysMask;
        ctrls->axt_ctrls_values &= ~XkbRepeatKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbRepeatKeysMask;
        ctrls->axt_ctrls_values |= XkbRepeatKeysMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "mousekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "m") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-mousekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-m") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=mousekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=m") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbMouseKeysMask;
        ctrls->axt_ctrls_values &= ~XkbMouseKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbMouseKeysMask;
        ctrls->axt_ctrls_values &= ~XkbMouseKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbMouseKeysMask;
        ctrls->axt_ctrls_values |= XkbMouseKeysMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "mousekeysaccel") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ma") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-mousekeysaccel") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ma") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=mousekeysaccel") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=ma") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbMouseKeysAccelMask;
        ctrls->axt_ctrls_values &= ~XkbMouseKeysAccelMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbMouseKeysAccelMask;
        ctrls->axt_ctrls_values &= ~XkbMouseKeysAccelMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbMouseKeysAccelMask;
        ctrls->axt_ctrls_values |= XkbMouseKeysAccelMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "accessx") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "a") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-accessx") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-a") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=accessx") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=a") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbAccessXKeysMask;
        ctrls->axt_ctrls_values &= ~XkbAccessXKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbAccessXKeysMask;
        ctrls->axt_ctrls_values &= ~XkbAccessXKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbAccessXKeysMask;
        ctrls->axt_ctrls_values |= XkbAccessXKeysMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "sticky") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "st") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-sticky") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-st") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=sticky") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=st") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbStickyKeysMask;
        ctrls->axt_ctrls_values &= ~XkbStickyKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbStickyKeysMask;
        ctrls->axt_ctrls_values &= ~XkbStickyKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbStickyKeysMask;
        ctrls->axt_ctrls_values |= XkbStickyKeysMask;
      }
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "twokey") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-twokey") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=twokey") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_TwoKeysMask;
            ctrls->axt_opts_values &= ~XkbAX_TwoKeysMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_TwoKeysMask;
            ctrls->axt_opts_values &= ~XkbAX_TwoKeysMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_TwoKeysMask;
            ctrls->axt_opts_values |= XkbAX_TwoKeysMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "latchlock") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-latchlock") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=latchlock") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_LatchToLockMask;
            ctrls->axt_opts_values &= ~XkbAX_LatchToLockMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_LatchToLockMask;
            ctrls->axt_opts_values &= ~XkbAX_LatchToLockMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_LatchToLockMask;
            ctrls->axt_opts_values |= XkbAX_LatchToLockMask;
          }
          i += inc;
        }
        else break;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "slowkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "sl") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-slowkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-sl") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=slowkeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=sl") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbSlowKeysMask;
        ctrls->axt_ctrls_values &= ~XkbSlowKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbSlowKeysMask;
        ctrls->axt_ctrls_values &= ~XkbSlowKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbSlowKeysMask;
        ctrls->axt_ctrls_values |= XkbSlowKeysMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "bouncekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "bo") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bouncekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-bo") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=bouncekeys") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=bo") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbBounceKeysMask;
        ctrls->axt_ctrls_values &= ~XkbBounceKeysMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbBounceKeysMask;
        ctrls->axt_ctrls_values &= ~XkbBounceKeysMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbBounceKeysMask;
        ctrls->axt_ctrls_values |= XkbBounceKeysMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "feedback") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "f") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-feedback") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-f") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=feedback") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=f") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbAccessXFeedbackMask;
        ctrls->axt_ctrls_values &= ~XkbAccessXFeedbackMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbAccessXFeedbackMask;
        ctrls->axt_ctrls_values &= ~XkbAccessXFeedbackMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbAccessXFeedbackMask;
        ctrls->axt_ctrls_values |= XkbAccessXFeedbackMask;
      }
      i += inc;
      while (1) {
        if ((i+1 <= argc && strcmp(argv[i], "dumbbell") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-dumbbell") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=dumbbell") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_DumbBellFBMask;
            ctrls->axt_opts_values &= ~XkbAX_DumbBellFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_DumbBellFBMask;
            ctrls->axt_opts_values &= ~XkbAX_DumbBellFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_DumbBellFBMask;
            ctrls->axt_opts_values |= XkbAX_DumbBellFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "led") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-led") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=led") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_IndicatorFBMask;
            ctrls->axt_opts_values &= ~XkbAX_IndicatorFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_IndicatorFBMask;
            ctrls->axt_opts_values &= ~XkbAX_IndicatorFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_IndicatorFBMask;
            ctrls->axt_opts_values |= XkbAX_IndicatorFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "feature") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-feature") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=feature") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_FeatureFBMask;
            ctrls->axt_opts_values &= ~XkbAX_FeatureFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_FeatureFBMask;
            ctrls->axt_opts_values &= ~XkbAX_FeatureFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_FeatureFBMask;
            ctrls->axt_opts_values |= XkbAX_FeatureFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowwarn") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowwarn") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=slowwarn") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_SlowWarnFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SlowWarnFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_SlowWarnFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SlowWarnFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_SlowWarnFBMask;
            ctrls->axt_opts_values |= XkbAX_SlowWarnFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowpress") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowpress") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=slowpress") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_SKPressFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKPressFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_SKPressFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKPressFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_SKPressFBMask;
            ctrls->axt_opts_values |= XkbAX_SKPressFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowaccept") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowaccept") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=slowaccept") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_SKAcceptFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKAcceptFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_SKAcceptFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKAcceptFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_SKAcceptFBMask;
            ctrls->axt_opts_values |= XkbAX_SKAcceptFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowreject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowreject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=slowreject") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_SKRejectFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKRejectFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_SKRejectFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKRejectFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_SKRejectFBMask;
            ctrls->axt_opts_values |= XkbAX_SKRejectFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "slowrelease") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-slowrelease") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=slowrelease") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_SKReleaseFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKReleaseFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_SKReleaseFBMask;
            ctrls->axt_opts_values &= ~XkbAX_SKReleaseFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_SKReleaseFBMask;
            ctrls->axt_opts_values |= XkbAX_SKReleaseFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "bouncereject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-bouncereject") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=bouncereject") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_BKRejectFBMask;
            ctrls->axt_opts_values &= ~XkbAX_BKRejectFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_BKRejectFBMask;
            ctrls->axt_opts_values &= ~XkbAX_BKRejectFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_BKRejectFBMask;
            ctrls->axt_opts_values |= XkbAX_BKRejectFBMask;
          }
          i += inc;
        }
        else if ((i+1 <= argc && strcmp(argv[i], "stickybeep") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "-stickybeep") == 0 && (inc = 1)) ||
            (i+1 <= argc && strcmp(argv[i], "=stickybeep") == 0 && (inc = 1))) {
          if (argv[i][0] == '-') {
            ctrls->axt_opts_mask |= XkbAX_StickyKeysFBMask;
            ctrls->axt_opts_values &= ~XkbAX_StickyKeysFBMask;
          }
          else if (argv[i][0] == '=') {
            ctrls->axt_opts_mask &= ~XkbAX_StickyKeysFBMask;
            ctrls->axt_opts_values &= ~XkbAX_StickyKeysFBMask;
          }
          else {
            ctrls->axt_opts_mask |= XkbAX_StickyKeysFBMask;
            ctrls->axt_opts_values |= XkbAX_StickyKeysFBMask;
          }
          i += inc;
        }
        else break;
      }
    }
    else if ((i+1 <= argc && strcmp(argv[i], "overlay1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ov1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-overlay1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ov1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=overlay1") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=ov1") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbOverlay1Mask;
        ctrls->axt_ctrls_values &= ~XkbOverlay1Mask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbOverlay1Mask;
        ctrls->axt_ctrls_values &= ~XkbOverlay1Mask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbOverlay1Mask;
        ctrls->axt_ctrls_values |= XkbOverlay1Mask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "overlay2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "ov2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-overlay2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ov2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=overlay2") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=ov2") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbOverlay2Mask;
        ctrls->axt_ctrls_values &= ~XkbOverlay2Mask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbOverlay2Mask;
        ctrls->axt_ctrls_values &= ~XkbOverlay2Mask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbOverlay2Mask;
        ctrls->axt_ctrls_values |= XkbOverlay2Mask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && strcmp(argv[i], "ignoregrouplock") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "-ignoregrouplock") == 0 && (inc = 1)) ||
        (i+1 <= argc && strcmp(argv[i], "=ignoregrouplock") == 0 && (inc = 1))) {
      if (argv[i][0] == '-') {
        ctrls->axt_ctrls_mask |= XkbIgnoreGroupLockMask;
        ctrls->axt_ctrls_values &= ~XkbIgnoreGroupLockMask;
      }
      else if (argv[i][0] == '=') {
        ctrls->axt_ctrls_mask &= ~XkbIgnoreGroupLockMask;
        ctrls->axt_ctrls_values &= ~XkbIgnoreGroupLockMask;
      }
      else {
        ctrls->axt_ctrls_mask |= XkbIgnoreGroupLockMask;
        ctrls->axt_ctrls_values |= XkbIgnoreGroupLockMask;
      }
      i += inc;
    }
    else if ((i+1 <= argc && get_number(argv[i], 0, 65535, &val1) && (inc = 1))) {
      ctrls->ax_timeout = val1;
      i += inc;
    }
    else return 0;
  }
  return 1;
}
