# Epsilon Virtual File System

Lightweight library made for easy access to .zip files within C++.

Both Stored and compressed data is supported.

It uses Zlib Inflation/Deflation algorithms.

## Usage Example
```c++
#include <filesystem.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  Filesystem::Mount("container.zip");

  std::shared_ptr<File> myFile = Filesystem::Open<std::shared_ptr<File> >("file.txt");
  
  char fileContent[myFile->SizeUncomp];
  
  myFile->seekg(0);
  myFile->read((char*) fileContent, myFile->SizeUncomp);

  cout << string(fileContent, myFile->SizeUncomp) << endl;
  
  Filesystem::Close(myFile);
  Filesystem::Unmount("container.zip");
  
  return 0;
}
```

## License
### The MIT License
<pre>
Copyright 2017 Imanol Fotia

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom 
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</pre>
