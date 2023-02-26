# Photo Directory Merger

Detects the naming scheme of the photos in subdirectories (ideally named by the
photographer's name), extracts the date and time from the filename, and copies
the photos in an output directory. The destination files are all named with the
same date and time format and the name of the source directory.

The result is an output directory with chronological sorted photos, including
the information who has taken the photo.

#### Usage:
```
phodime [options] INDIR [INDIR [INDIR [...]]] OUTDIR
```

#### Example:
```
$ cd ~/Pictures/our-great-trip-to-awesomeland/
$ phodime -v Emily Joe Mary merged
```
