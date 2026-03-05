echo "setting up x64 toolchain"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
where cl.exe
echo "done"
