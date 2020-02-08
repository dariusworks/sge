# sge
===

## Steps To Build
1. Download boost and unzip into the api directory or use (Recommended) `vcpkg install boost-bind:x86-windows`
2. Copy 3rdparty/jpeg/jconfig.vc as jconfig.h (This "fork" already has it done for you)
3. Update TinyXML and UnitTest++ if you want. There is also .bat script for you. Make sure unzip command is useable (by having program mentioned in path (making command systemwide)