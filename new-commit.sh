#!/bin/bash
mkdir commit-$1
cp commit-"$(($1-1))"/* commit-$1
cd commit-$1