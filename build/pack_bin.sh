#!/bin/bash

# author        Oliver Blaser
# date          11.02.2023
# copyright     GNU GPLv3 - Copyright (c) 2023 Oliver Blaser

source dep_globals.sh

platform=$(uname -m)
packedDir="./packed"
outDirName="${prjDirName}_linux_$platform"
outDir="$packedDir/$outDirName"
archiveFileName="${prjDirName}_linux_${platform}_v$versionstr.tar.gz"
archive="$packedDir/$archiveFileName"

rm -rf $outDir

mkdir -p $outDir/$prjDirName
#mkdir $outDir/$prjDirName/include
#mkdir $outDir/$prjDirName/lib

#cp -r ../include/* $outDir/$prjDirName/include
#cp ../lib/lib${prjBinName}.a $outDir/$prjDirName/lib
#cp ../lib/lib${prjBinName}.so* $outDir/$prjDirName/lib
cp ./cmake/$prjBinName $outDir/$prjDirName

writeReadmeFile $outDir/$prjDirName/readme.txt
cp ../license.txt $outDir/$prjDirName

rm -f $archive
tar -czf $archive -C $outDir $prjDirName
