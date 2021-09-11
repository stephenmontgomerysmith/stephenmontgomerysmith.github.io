#!/usr/bin/perl -w

my $programname =

'naturalmath.cgi';


# Last modified July 21, 2001
#
# Copyright 1999 Stephen J Montgomery-Smith.  All rights reserved.
#
# This package is free software; you can redistribute it and/or modify
# it under the terms of either:
#
#     a) the GNU General Public License as published by the Free
#     Software Foundation; either version 2, or (at your option) any
#     later version, or
#
#     b) "Stephen's Artistic License" which comes with this Kit.
#
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See either
# the GNU General Public License or Stephen's Artistic License for more
# details.
#
# You should have received a copy of Stephen's Artistic License with this
# package, in the file named "Stephens-Artistic.txt".  If not, I'll be glad
# to provide one.
#
# You should also have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#######################################################################
#
# To use this script, you will also need the following installed:
#
# latex, dvips (part of the teTeX package)
# ghostscript (version 6.5 or greater works)
# ppmtogif (part of the netpbm package http://netpbm.sourceforge.net/ )
#
# Directory structure: this script presumes that there is a directory,
# also accessible to your web server, which is ../tex-stuff.  This
# directory has to be given write access to the user of the web server
# (which for example in my case I do by making ../tex-stuff owned by
# the user "nobody").
# This script also assumes that the program naturalmath is installed
# in the path space given below.
#
# Obviously you can customize these to suite your environment.
#
##################################################################
#
# If you find a way to crack into my system via this script, please be
# kind and tell me about it.  Thanks, Stephen

use CGI qw(:standard);
$CGI::POST_MAX=1000;

$ENV{PATH} = '/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin:/usr/local/sbin:/usr/X11R6/bin';
$TEX_STUFF = "../tex-stuff";

sub center
{
  return "<center>@_</center>";
}

my $nmtext = param("nmtext");
my $scale = param("scale");

my $ident = `date '+%Y-%m-%d-%H-%M-%S'`;
chomp($ident);
$ident .= "-$$";
$ident =~ /^(\d+-\d+-\d+)/;
my $date = $1;

print header;

print start_html(-title=>"Try Natural Math",
                 -author=>"Stephen Montgomery-Smith",
                 -BGCOLOR=>'pink'),
      center(h1("Try Natural Math")),
#     "Sorry - it is not working right now.",p,
      "Here you can try out ",
      "<a href=\"http://www.math.missouri.edu/\~stephen/naturalmath\">Natural Math</a>\n",
      "(<a href=\"http://www.math.missouri.edu/\~stephen/naturalmath/tutor\">click here for the tutorial</a>)\n",
#     "This web page is a bit slow.  (This is mostly\n",
#     "time taken to make the \`gif\' file.)\n",
      p,
      "Enter lines of Natural Math in the box below, and click\n",
      "on the Submit button.  Then the result will be displayed ",
      "below the Submit button.",p,
      center(
      start_form,
      textarea("nmtext",
               "sum from n=1 to infinity of 1 over n^2 = pi^2 over 6" .
               "\n\n" .
               "integral from -infinity to infinity of x^(2n) e^(-x^2/2) dx\n= sqrt(2 pi) (2n)! over (2^n n!)" .
               "\n\n" .
               "(1 + sqrt 5) over 2\n" . 
               "= 1 + 1 over (1 + 1 over (1 + 1 over (1 + 1 over (1 + dots))))" .
               "\n\n" .
               "\"sech\"(x) = 2 over (e^x+e^-x)" ,
               "\n" .
               10,80),p,
      "Scale: ", popup_menu('scale',[1,1.5,2,3,4],2),p,
      submit("Submit"),
      end_form);

if (param())
{
  if (length($nmtext) > 500)
  {
    print "You are limited to 500 characters.  If you want to try longer\n",
          "examples, I suggest you download the program and install it.";
    exit;
  }
  open(TEXFILE,"> $TEX_STUFF/afile$ident.nat");
  print TEXFILE "# remote host: " . remote_host() . "\n" if (defined(remote_host()));
  print TEXFILE "# user name:   " . user_name() . "\n" if (defined(user_name()));
  print TEXFILE "# referer:     " . referer() . "\n\n" if (defined(referer()));
  print TEXFILE $nmtext;
  close(TEXFILE);

  chdir "$TEX_STUFF";
  system "naturalmath -sn afile$ident > /dev/null 2>&1;" .
         "chmod a-r afile$ident.nat;" .
         "latex afile$ident > /dev/null 2>&1;" .
         "dvips -E afile$ident.dvi -o afile$ident.ps > /dev/null 2>&1;";
  $bounding = `grep BoundingBox afile$ident.ps`;
  $bounding =~ /\%\%BoundingBox:\s+(\-?\d+)\s+(\-?\d+)\s+(\-?\d+)\s+(\-?\d+)/;
  $bbx=-$1;
  $bby=-$2;
  $scale = 1 if ($scale<=1);
  $scale = 4 if ($scale>=4);
  $scale*=72;
  $bbw=int($scale/72*($3-$1)+.999999);
  $bbh=int($scale/72*($4-$2)+.999999);
  open(GS,"| gs -q -dSAFER -dNOPAUSE -sDEVICE=ppmraw -r$scale -g${bbw}x${bbh} -sOutputFile=afile$ident.ppm > /dev/null");
  print GS "$bbx $bby translate\n";
  print GS "(afile$ident.ps) run\n";
  print GS "quit\n";
  close(GS);
# system "pnmcrop afile$ident.ppm > afile$ident.cpm;" .
  system "ppmtogif -transparent white afile$ident.ppm > afile$ident.gif;" .
         "rm -f *.aux *.cpm *.ppm *.dvi *.log *.ppm *.ps *.tex;";

# $img = `cd $TEX_STUFF; giftopnm afile$ident.gif | pnmfile`;
# $img =~ m/(\d+) by (\d+)/;
# $bbw = $1;
# $bbh = $2;
  print "<center>Here is what it looks like.</center>",p,
        "\n<center><img src=\"$TEX_STUFF/afile$ident.gif\" ",
        "width=$bbw height=$bbh align=abscenter ",
        "alt=\"This is a visual representation of the equation you typed.\"></center>\n",
        p,"You can use this image for yourself: download the <a href=\"$TEX_STUFF/afile$ident.gif\">image</a>",
        " and use HTML something like:",br,
        "<tt>&lt;img src=\"image.gif\" width=$bbw height=$bbh align=abscenter alt=\"The picture.\"></tt>";

  print end_html;
}
