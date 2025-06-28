OutFile "MyAppInstaller.exe"
InstallDir "$PROGRAMFILES\MyApp"
RequestExecutionLevel admin

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "out\build\x64-Debug\*.*"
SectionEnd