# SJSON CPP

`sjson-cpp` is a C++ library to read and write [SJSON](http://help.autodesk.com/view/Stingray/ENU/?guid=__stingray_help_managing_content_sjson_html) files.
It aims to be minimal, fast, and get out of the way of the programmer.

By design, the reader/parser does no memory allocations. This is in contrast to the [nflibs C parser](https://github.com/niklasfrykholm/nflibs).

Everything is 100% C++ header based for easy and trivial integration.

## Unsupported Uses

This parser is intended to accept only pure SJSON, and it will fail if given a JSON file, unlike the [Autodesk JS Stingray parser](https://github.com/Autodesk/sjson). 

The following are not yet supported:
- null literals
- unescaping characters within strings, since only string views (windowed pointers into the input) are returned.
- utf8 or unicode SJSON

## Authors

*  [Nicholas Frechette](https://github.com/nfrechette)
*  [Cody Jones](https://github.com/CodyDWJones)

## MIT License

Copyright (c) 2017 Nicholas Frechette, Cody Jones, and sjson-cpp contributors
