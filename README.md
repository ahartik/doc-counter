Implementation of document counting using ILCP wavelet trees.

Building
===============
First make sure to get all submodules
$ git submodule init
$ git submodule update
To build:
$ cmake .
$ make -j


Running
====================
To run an experiment a document collection file is needed. 
This file is simply a null-delimited collection of the documents.

Pattern file is a collection of patterns to be searched, each on their own line.

$ doc_counter --structures brute,rle_skewed,rle,skewed,balanced,rle_rrr,rle_skewed_rrr \
    --pattern_file wiki_patterns --document_file ~/progradu/fiwiki/fiwiki.cat


Structures
=========================
Structures implemented are:
  brute - bruteforce counting for verifying results
  balanced - normal, balanced wavelet tree
  skewed - skewed wavelet tree
  ,rle_skewed,rle,skewed,balanced,rle_rrr,rle_skewed_rrr
