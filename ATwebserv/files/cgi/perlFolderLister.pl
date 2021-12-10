#!/usr/bin/perl

use strict;
use CGI;

my $list = new CGI;

my $arg = $ARGV[0];
print $arg;
my $fileDir = "./";
my @files;

opendir DIR, "$fileDir" or die "Can't open $fileDir $!";
    @files = readdir DIR; # grep { /\.(?:txt|err|csv)$/i } readdir DIR;
closedir DIR;

print $list->header("text/html"),
      $list->start_html("Archives de $fileDir"),
      $list->p("Voici les fichiers de $fileDir");

foreach my $file (@files) {
    print $list->p(
            $list->a({-href=>$file},
            $file)
            );
    
}

print $list->end_html;

#ANCIENNE SOLUTION :

# DIR *dpdf;
# struct dirent *epdf;
# 
# dpdf = opendir("./");
# if (dpdf != NULL)
#    while ((epdf = readdir(dpdf)))
#      std::cout << epdf->d_name << std::endl;
# closedir(dpdf);