# JPEG Image File spec

## Segments
### JFIF
| Segment       |                                                               |
|:--------------|:--------------------------------------------------------------|
| `SOI`         | start of image                                                |
| `APP0`        |                                                               |
| [`APP0-JFXX`] | optional extension segment                                    |
| [`APP1`]      | an EXIF application segment may be present                    |
| ...           | optional additional segments, in any order: `SOF`, `DHT`, ... |
| `SOS`         |   start of scan                                                            |
| `EOI`         | end of image                                                  |

### EXIF
| Segment                      |                            |
|:-----------------------------|:---------------------------|
| `SOI`                        | start of image             |
| `APP1`                       |                            |
| [`APP2`]                     | optional extension segment |
| [`APPn`]                     | optional extension segments |
| `DQT`, `DHT`, [`DRI`], `SOF` | may be in any order        |
| `SOS`                        | start of scan              |
| `EOI`                        | end of image               |



## Links
### JFIF
- https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format#File_format_structure

### EXIF
- https://www.cipa.jp/std/documents/e/DC-008-2012_E.pdf
- https://stackoverflow.com/questions/1821515/how-is-exif-info-encoded

### TIFF
- https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
