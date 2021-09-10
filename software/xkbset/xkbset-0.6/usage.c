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

void print_usage()
{
  printf("Usage: To set or unset various options:\n");
  printf("\n");
  printf("  xkbset <options>\n");
  printf("\n");
  printf("where <options> may be all or any of (the '-' switches the feature off, \n");
  printf("otherwise it is switched on):\n");
  printf("    \n");
  printf("To switch the bell on or off:\n");
  printf("    \n");
  printf("    [-]{bell|b}\n");
  printf("    \n");
  printf("To switch one key to autorepeat or not:\n");
  printf("    \n");
  printf("    [-]{repeatkeys <per_key_repeat>|r <per_key_repeat>}\n");
  printf("    \n");
  printf("To send a hex mask for all keys to autorepeat or not\n");
  printf("    \n");
  printf("    perkeyrepeat <per_key_repeat>\n");
  printf("    \n");
  printf("To switch autorepeat on or off, and optionally set the delay before\n");
  printf("the first repeat and the interval between repeats (times in milliseconds):\n");
  printf("    \n");
  printf("    [-]{repeatkeys|r} [rate <repeat_delay> [<repeat_interval>]]\n");
  printf("    \n");
  printf("To switch mousekeys on or off, and optionally set the default\n");
  printf("button (whatever that is):\n");
  printf("    \n");
  printf("    [-]{mousekeys|m} [<mk_dflt_btn>]\n");
  printf("    \n");
  printf("To switch mousekeys acceleration on or off, and optionally set\n");
  printf("the acceleration characteristics:\n");
  printf("    \n");
  printf("    [-]{mousekeysaccel|ma} [<mk_delay> <mk_interval> <mk_time_to_max> \n");
  printf("      <mk_max_speed> <mk_curve>]\n");
  printf("    \n");
  printf("To switch AccessX on (so pressing shift five times starts sticky keys\n");
  printf("and pressing the shift key down 8 seconds starts slow keys):\n");
  printf("    \n");
  printf("    [-]{accessx|a}\n");
  printf("    \n");
  printf("To switch sticky keys on or off, and optionally set or reset:\n");
  printf("() two keys pressed at the same time stops sticky keys;\n");
  printf("() a modifier pressed twice will be locked:\n");
  printf("    \n");
  printf("    [-]{sticky|st} [[-]twokey|[-]latchlock]...\n");
  printf("    \n");
  printf("To switch on slowkeys, and optionally set the slow key delay (in\n");
  printf("milliseconds):\n");
  printf("    \n");
  printf("    [-]{slowkeys|sl} [<slow_keys_delay>]\n");
  printf("    \n");
  printf("To switch on bouncekeys, and optionally set the time (in milliseconds) for\n");
  printf("which if the key is pressed again in that time it will not work:\n");
  printf("    \n");
  printf("    [-]{bouncekeys|bo} [<debounce_delay>]\n");
  printf("    \n");
  printf("To switch on audible feedback, and optionally set which features\n");
  printf("cause the feedback (note [-]feature means that switching\n");
  printf("one of the AccessX features on of off causes feedback):\n");
  printf("    \n");
  printf("    [-]{feedback|f} [[-]dumbbell|[-]led|[-]feature|[-]slowwarn|\n");
  printf("      [-]slowpress|[-]slowaccept|[-]slowreject|[-]slowrelease|\n");
  printf("      [-]bouncereject|[-]stickybeep]...\n");
  printf("    \n");
  printf("To switch keyboard overlays 1 or 2 on or off:\n");
  printf("    \n");
  printf("    [-]{overlay1|ov1}\n");
  printf("    [-]{overlay2|ov2}\n");
  printf("    \n");
  printf("To select the group wrap type (now what is that)?\n");
  printf("    \n");
  printf("    groupswrap {redirect|clamp|wrap} [<groups_wrap>]\n");
  printf("    \n");
  printf("What is this?\n");
  printf("    \n");
  printf("    [-]ignoregrouplock\n");
  printf("    \n");
  printf("To cause some of the key modifiers (like shift, num-lock=mod2, etc)\n");
  printf("to not work:\n");
  printf("    \n");
  printf("    nullify [[-]shift|[-]lock|[-]control|[-]mod1|[-]mod2|[-]mod3|[-]mod4|\n");
  printf("      [-]mod5]...\n");
  printf("    \n");
  printf("What is this?\n");
  printf("    \n");
  printf("    ignorelock [[-]shift|[-]lock|[-]control|[-]mod1|[-]mod2|[-]mod3|\n");
  printf("      [-]mod4|[-]mod5]...\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("To set the AccessX expire controls:\n");
  printf("\n");
  printf("  xkbset exp <options>\n");
  printf("\n");
  printf("where <options> may be all or any of (<ax_timeout> is the timeout (in\n");
  printf("seconds) after which no user activity on X will cause the expiry; '-'\n");
  printf("indicates the feature will be switched off, '=' incicates the feature\n");
  printf("will be left unchanged, otherwise it will be switched on):\n");
  printf("\n");
  printf("    <ax_timeout>\n");
  printf("    [-|=]{bell|b}\n");
  printf("    [-|=]{repeatkeys|r}\n");
  printf("    [-|=]{mousekeys|m}\n");
  printf("    [-|=]{mousekeysaccel|ma}\n");
  printf("    [-|=]{accessx|a}\n");
  printf("    [-|=]{sticky|st} [[-|=]twokey|[-|=]latchlock]...\n");
  printf("    [-|=]{slowkeys|sl}\n");
  printf("    [-|=]{bouncekeys|bo}\n");
  printf("    [-|=]{feedback|f} [[-|=]dumbbell|[-|=]led|[-|=]feature|[-|=]slowwarn|\n");
  printf("      [-|=]slowpress|[-|=]slowaccept|[-|=]slowreject|[-|=]slowrelease|\n");
  printf("      [-|=]bouncereject|[-|=]stickybeep]...\n");
  printf("    [-|=]{overlay1|ov1}\n");
  printf("    [-|=]{overlay2|ov2}\n");
  printf("    [-|=]ignoregrouplock\n");
  printf("\n");
  printf("To see the current values of the controls:\n");
  printf("\n");
  printf("  xkbset q\n");
  printf("\n");
  printf("To see the current values of what controls will expire:\n");
  printf("\n");
  printf("  xkbset q exp\n");
  printf("\n");
  printf("To have these values written as a command line:\n");
  printf("\n");
  printf("  xkbset w [exp]\n");
  printf("\n");
  printf("To print this help message\n");
  printf("\n");
  printf("  xkbset [h]\n");
  printf("\n");

}
