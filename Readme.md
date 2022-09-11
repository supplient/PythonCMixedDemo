#! https://zhuanlan.zhihu.com/p/563444290
# Python-C混合编程（Python宿主）

本文的撰写目的是总结调研结果，如有错误欢迎指正。

示例项目：[github](https://github.com/supplient/PythonCMixedDemo)

# 问题
我有一个Python项目，我想为它添加一些C++代码来实现特定功能。

我的选择是为它写一个[C++ extension](https://docs.python.org/3/extending/extending.html)。
该方法的效果如该代码所示：

```python
# main.py
# cuda_hallo是一个C++ extension
import cuda_hallo
cuda_hallo.test()
```

* Q：为什么不用[ctypes](https://docs.python.org/3/library/ctypes.html)导入dll？
	* A：我希望能有良好的语法提示，而且用`.pyd`而不是`.dll`的话，导入进来的对象是python原生的`module`类型，而不是ctypes定义的特定类型，会更容易调试。
* Q：为什么不用[Cython](https://cython.org/)？
	* A：我想用干净的Python，我不想花时间去学习Cython的语法和它的编译器。

综上两个原因，我最后选择的是写C++ extension。从python层面来看，它就是一个简单的module而已，和其他所有module都一样。


# 总流程
1. 构建C++ library
2. 构建C++ extension
3. 生成stub file
4. 配置python IDE的intellisense
5. 修改python代码，以确保能找到C++ extension
6. 混合调试C++ library, C++ extension, python

下文逐节详述。

# 1. 构建C++ library
这一部分就是实际工作的C++代码。

关于.dll和.lib是什么，请参阅[visual studio的这篇文档](https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=msvc-170)。其他平台（例如Linux的.so, .a）也都是类似的。

按标准流程构建C++ library即可，没有任何特别的地方。

* Q：为什么示例项目里选择static library(.lib)而不是dynamic library(.dll)？
	* A: 主要是为了能让第二步生成的.pyd文件和这一步生成的.lib/.dll文件重名。如果选择dynamic library的话，那dll文件就得和pyd文件放在一起，此时如果它俩重名的话，它俩各自的调试文件(e.g. `test.pdb`)就也会重名，结果就会出错了。因此我选择static library，那lib文件就能放到和pyd文件不同的目录下了。


# 2. 构建C++ extension
这一部分用来构建C++部分开放给python的接口。

这部分基于的是[CPython](https://github.com/python/cpython)提供的[Python API](https://docs.python.org/3/extending/extending.html)，我并没有调研[PyPy](https://www.pypy.org/)等第三方Python实现有没有提供类似的接口，所以并不清楚那些Python实现该怎么编写C++ extension。

当然可以直接用Python API来编写C++ extension，那直接按官方文档走就行了，但那样的工作量太大了，而且其中大量的工作都只是在繁琐地填写类型而已。
因此我选择使用第三方工具来封装Python API，也就是：`简短的接口定义代码 => 第三方工具 => 冗长的Python API代码`。

我选择的是[pybind11](https://pybind11.readthedocs.io/en/stable/)。

* Q: 为什么不选择[cffi](https://cffi.readthedocs.io/en/latest/)？
	* A：原因有五：
		1. cffi是用我手写的python代码来生成用于构建C extension的胶水代码的，它多了一个`python => C`的生成过程。而pybind11是一个C++ header库，所以可以直接参与进构建C++ extension的编译过程。
		2. cffi不支持C++，需要我额外写个C wrapper封装一下。
		3. 为cffi构建的C extension编写[stub file](https://stackoverflow.com/questions/59051631/what-is-the-use-of-stub-files-pyi-in-python)是件繁琐的事情：作者并没有[支持自动生成stub file的想法](https://groups.google.com/g/python-cffi/c/6V8PBjA6ZJ4)，而用[stubgen](https://mypy.readthedocs.io/en/stable/stubgen.html)为cffi构建的C extension生成stub file时效果很糟。
		4. cffi构建的C extension总是会分成`ffi`, `lib`两块，我知道作者是为了方便从python端构建C对象、操作C内存，但这对python端而言并不是那么自然，它表现得不像一个module。
		5. cffi没有提供方便的编译工具，只能用官方提供的[distutils方法](https://docs.python.org/3/extending/building.html#building)或者[手动编写复杂的工程文件](https://docs.python.org/3/extending/windows.html#building-on-windows)。
		
按照pybind11的官方指南编写定义接口的C++文件，并构建生成pyd文件即可。值得一提的是pyd文件和dll文件是等价的，只是后缀名不同而已。

* Q: 为什么示例项目中选择的是[CMake](https://pybind11.readthedocs.io/en/stable/compiling.html#building-with-cmake)，而不是[setuptools](https://pybind11.readthedocs.io/en/stable/compiling.html#building-with-setuptools)？
	* A：一个原因是为了能和C++ library的构建更直观地整合到一个系统里，不过这个就算是用setup也是能做到的。更主要的原因是为了能和IDE协作。定义接口的C++文件也是我手写的，是很有可能出错的。如果采用setup方法的话，构建时的编译错误、链接错误都很难和IDE进行联动（当然可以自己写个parser工具来解析setup的输出，但那可太麻烦了），结果就加大了我的开发成本。而采用cmake方法的话，像Visual Studio之类的IDE会很好地与cmake进行联动，帮助开发、调试C++ extension。



# 3. 生成[stub file](https://mypy.readthedocs.io/en/stable/stubs.html)
目前所有python的intellisense工具都没法直接从.pyd文件里里提取类型信息，所以必须为.pyd文件编写.pyi文件才能让IDE为我们提供类型提示、自动补全、静态检查。

当然可以手动编写stub file，但那工作量未免大了点。
我选择的方案是使用[stubgen](https://mypy.readthedocs.io/en/stable/stubgen.html#stubgen)。
需要注意的有两点：

* 需要使用`-m`来为C++ extension生成stub file。[参考](https://github.com/python/mypy/issues/7692#issuecomment-541001171)
* 可以使用[rst file](https://en.wikipedia.org/wiki/ReStructuredText)来辅助stub file的生成。

* Q: stubgen的效果似乎不够好，别的stub file生成工具？
	* A：很遗憾，似乎没有。曾有过一个针对pybind11设计的stubgen，[pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen)，但它已经停止开发，转而拥抱stubgen了（参见[issue](https://github.com/sizmailov/pybind11-stubgen/issues/31)。



# 4. 配置python IDE的intellisense
毕竟我们大多数时候不会把编写的C++ extension安装到系统的python里面去的，所以需要把前两步生成的pyd, pyi文件的位置告知IDE，不然intellisense工具依然无法正确工作。

不同IDE的做法不同：
* VSCode：在.vscode下的settings.json添加`python.autoComplete.extraPaths, python.analysis.extraPaths`。
* PyCharm：按照[这篇文章](https://github.com/sizmailov/pybind11-stubgen)添加路径即可。
* Visual Studio：在Project设置里追加search path。


# 5. 修改python代码，以确保能找到C++ extension
上一步只是让IDE知道了pyd的路径而已而已，但python解释器其实还是不知道的（PyCharm的情况倒是都知道了）。

按照[这个回答](https://stackoverflow.com/questions/3108285/in-python-script-how-do-i-set-pythonpath)，给`sys.path`追加路径即可。



# 6. 混合调试C++ library, C++ extension, python
最理想的情况当然是python和C++在同一个IDE里调试，但现实是似乎没有一个IDE做得到。

尽管Visual Studio存在[Mixed-mode debugging](https://docs.microsoft.com/en-us/visualstudio/python/debugging-mixed-mode-c-cpp-python-in-visual-studio?view=vs-2022)，但这一模式其实存在严重bug，并不能正常使用。参见[so的这个问题](https://stackoverflow.com/questions/64636079/debugging-python-and-c-in-visual-studio)、[github上的这个issue](https://github.com/MicrosoftDocs/visualstudio-docs/issues/7750)。

所以现实中唯一的做法仍然还是启动两个不同的IDE进程，然后让一个IDE启动python进程，让另一个IDE attach上python进程，从而实现混合调试。



# 示例项目简介
除了main分支以外还有个cuda分支，那个分支下有个cuda_hallo模块。

构建需要的工具：
* CMake：>=3.8。最好是3.24版本，因为有个cuda相关的设置是3.24引入的。
* Python：>=3.7
* [stubgen](https://mypy.readthedocs.io/en/stable/stubgen.html#stubgen)
* cuda：如果你想试试cuda分支下的cuda_hallo模块的话
* git：pybind11是作为submodule放进来的，所以你需要git来clone它。

构建步骤：
1. 获取pybind11：`git submodule update --init`
2. 构建C++ library, C++ extension，生成stub file
	1. `cd cexts/build`
	2. `cmake ..`
	3. `cmake --build .`
3. 完成。`python main.py`查看是否成功导入吧。

目录结构：
* main.py：测试C++ extension能否正常导入的python文件
* .vscode：我的python IDE是VSCode，这是它的配置文件
* cexts：C++ extension的目录
	* CMakeLists.txt：用于总揽C++ library的构建, C++ extension的构建, stub file的生成。
	* hallo：C++ library的目录
	* pybind：C++ extension的目录