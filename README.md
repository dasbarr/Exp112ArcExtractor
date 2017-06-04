# Experience112 Arc Extractor

A resource extractor for .arc files from "Experience112" ("The Experiment") game. Works with both 1-byte and 2-byte char encodings (automatically determines which one is used by the game archive) and doesn't use any of game libs.

# Usage

Place extractor executable to the directory that contains .arc file to extract. Open console window and type:
```
exp112ArcExtractor <archiveName>.arc
```
Example:
```
exp112ArcExtractor scripts.arc
```
    
Extractor will recreate all directory hierarchy from the archive in the current directory.