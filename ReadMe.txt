
编译过程如下

1.使用VS2015打开OpenFace.sln
2.编译类型选择Release,X64
3.按顺序编译dlib,LandmarkDetector,faceDetDll,dllTester
4.将OpenFace\lib\3rdParty\OpenCV3.1\x64\v140\bin中的opencv_world310.dll拷贝到将OpenFace\x64\Release文件夹下
5.在x64/Release中打开dllTester.exe，测试是否可以正常运行，如果可以，将faceDetDll.dll拷贝到unity中Assets/Plugins目录下替换原来的


32位编译
打开vs，编译类型选择Release,X32
3.按顺序编译dlib,LandmarkDetector,faceDetDll,dllTester
4.将OpenFace\lib\3rdParty\OpenCV3.1\x86\v140\bin中的opencv_world310.dll拷贝到将OpenFace\Release文件夹下
5.在Release中打开dllTester.exe，测试是否可以正常运行
6.unity中，将程序发布为x86类型，同时将Release文件夹中的faceDetDll.dll,opencv_world310.dll,tbb.dll替换unity文件夹中原来的dll，即可直接运行


--update 2017-3-22
编译前先把原来工程中的lib/thirdParty的文件拷贝当前工程的lib文件夹下