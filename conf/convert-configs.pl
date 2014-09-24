#!/usr/bin/perl

# apt-dater - terminal-based remote package update manager
#
# Authors:
#   Andre Ellguth <ellguth@ibh.de>
#   Thomas Liske <liske@ibh.de>
#
# Copyright Holder:
#   2009-2014 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
#
# License:
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this package; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
#

use strict;
use warnings;

use Config::IniFiles;

my $xdg_config_home = (exists($ENV{XDG_CONFIG_HOME}) ? $ENV{XDG_CONFIG_HOME} : qq($ENV{HOME}/.config));
my $ofname = qq($xdg_config_home/apt-dater/apt-dater.conf);
my $nfname = qq($xdg_config_home/apt-dater/apt-dater.config);
my @sections = qw(SSH Paths Screen Notify Hooks Appearance AutoRef History TCLFilter);

die qq(New config file does already exist ($nfname)!\n)
    if(-e $nfname);

my $ocfg = Config::IniFiles->new(
    -file => $ofname,
    -nocase => 1
    );

print STDERR "Reading from $ofname...\n";

die "Failed to open $nfname for writing: $!\n"
    unless(open(STDOUT, '>', "$nfname.conv"));


print STDERR "Writing to $nfname.conv...\n";

foreach my $section (@sections) {
    next unless($ocfg->SectionExists($section));

    my @params = $ocfg->Parameters($section);

    if(scalar @params) {
	print STDERR "* [$section]\n";

	print "\n$section:\n{\n";
	foreach my $param (@params) {
	    print "\t$param=\"", $ocfg->val($section, $param), "\";\n";
	}
	print "}\n";
    }
}
 

die "Failed to rename $nfname.conv to $nfname: $!\n"
    unless(rename(qq($nfname.conv), $nfname));

print STDERR "Renamed $nfname.conv to $nfname.\n";


die "Failed to rename $ofname to $ofname.old: $!\n"
    unless(rename($ofname, qq($ofname.old)));

print STDERR "Renamed $ofname to $ofname.old.\n";
