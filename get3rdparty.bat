@echo off

if not exist 3rdparty (
   mkdir 3rdparty
)

cd 3rdparty

REM https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.zip or (better) `vcpkg install boost-bind:x86-windows`

if not exist tinyxml (
   curl --location -O http://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip
   unzip tinyxml_2_6_2.zip
   del tinyxml_2_6_2.zip
)

if not exist UnitTest++ (
 curl --location -O https://github.com/unittest-cpp/unittest-cpp/archive/v1.4.zip
   REM curl --location -O  http://downloads.sourceforge.net/project/unittest-cpp/UnitTest++/1.4/unittest-cpp-1.4.zip
   REM outputs html code because it's not up on sourceforge
   unzip unittest-cpp-1.4.zip
   del unittest-cpp-1.4.zip
)

popd
