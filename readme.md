# My test tools used under UEFI Shell   

### **How to build it.**  
Currently, this project can be built with edk2-version: edk2-stable202108.  
1, get ready for UEFI development: https://github.com/tianocore/edk2   
2, copy KrishnaTestPkg to your workspace.  
3, run "edksetup.bat" to construct build environment.  
4, read KrishnaTestPkg\KrishnaTestPkg.dsc to fix dependencies for some tools.  
5, run "build -p KrishnaTestPkg\KrishnaTestPkg.dsc -a X64" to build it.  

### **Notes.**  
regex.efi has dependency: https://github.com/kokke/tiny-regex-c  
xdb.efi has dependency: https://github.com/akheron/jansson  
history.efi has got some idea from Z.t(http://www.lab-z.com/)  
Thanks to all the people behind them.  
