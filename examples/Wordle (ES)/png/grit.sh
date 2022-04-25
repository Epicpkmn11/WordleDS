#!/bin/bash

for IMAGE in *.png; do
	grit $IMAGE -ftr -fh! -o../$IMAGE
done
