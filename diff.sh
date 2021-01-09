#!/bin/bash

shakyo_file="kernel/$1"
mikanos_file="../mikanos/kernel/$1"

diff $shakyo_file $mikanos_file