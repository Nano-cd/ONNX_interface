# ONNX_interface
onnx metadata change/onnx metadata get in python and c++
---

# ONNX 模型元数据读写工具

## 概述

本项目提供了一套工具，用于在 ONNX 模型文件中嵌入和读取自定义元数据。这在模型部署和版本管理中非常有用，可以确保模型与其使用者（例如，一个动态链接库或可执行文件）之间的版本兼容性。

- **`writer.py` (Python)**: 一个 Python 脚本，用于向现有的 `.onnx` 文件中添加或更新自定义元数据字段，如模型版本、作者和描述等。
- **`reader.cpp` (C++)**: 一个 C++ 函数，演示了如何使用 ONNX Runtime C++ API 来读取 `.onnx` 文件中的这些自定义元数据。

这个工作流程对于构建健壮的 MLOps（机器学习操作）管道至关重要，它允许您将关键信息直接绑定到模型资产中。

## 功能特性

- **嵌入元数据**: 使用 Python 轻松地将版本号、作者、描述等信息写入任何 ONNX 模型。
- **读取元数据**: 使用 ONNX Runtime C++ API 高效地从模型中解析出嵌入的元数据。
- **版本控制**: 通过在模型中嵌入 `required_dll_version` 字段，可以实现客户端与模型版本的自动校验。
- **跨语言工作流**: 演示了 Python (通常用于模型训练和准备) 与 C++ (通常用于高性能推理部署) 之间的无缝协作。
- **内存安全**: C++ 代码示例正确地展示了如何使用 `Ort::AllocatorWithDefaultOptions` 来管理从 ONNX Runtime API 获取数据时所需的内存，避免了内存泄漏。

## 环境依赖

为了运行这些脚本，您需要安装以下库：

### Python 环境

- **onnx**: 用于加载、修改和保存 ONNX 模型。
- **onnxruntime** (可选): 用于在 Python 端验证模型。

您可以使用 pip 来安装它们：
```bash
pip install onnx onnxruntime
```

### C++ 环境

- **ONNX Runtime C++ API**: 您需要下载并链接 ONNX Runtime 预编译的库。
  - 您可以从 [ONNX Runtime GitHub Releases](https://github.com/microsoft/onnxruntime/releases) 页面下载适合您操作系统和架构的最新版本。
  - 在您的 C++ 项目中，需要配置头文件包含路径和库链接路径。

## 使用方法

### 步骤 1: 使用 Python 嵌入元数据

1.  将您的 ONNX 模型文件（例如 `best.onnx`）准备好。
2.  修改 `writer.py` 脚本中的变量：
    - `onnx_file_path`: 指向您的原始 ONNX 模型路径。
    - `onnx_version`, `required_dll_version`, `author`, `description`: 设置您想要嵌入的元数据值。
3.  运行 Python 脚本：

    ```bash
    python writer.py
    ```
4.  脚本会生成一个新的 ONNX 文件（在这个例子中是 `wakuwaku~.onnx`），其中包含了您定义的元数据。

#### `writer.py` 源码
```python
import onnx
from onnx import helper, StringStringEntryProto

# 假设 'your_model' 是你已经定义好的 ONNX 模型对象
# ...

# --- 这是关键部分 ---
# 定义你想要嵌入的自定义元数据
# 强烈建议包含模型自身的版本号，以及它所期望的DLL版本号
onnx_file_path = "E:/project_pycharm/MYMLOPs/comsuption_mid/comsuption/bufferv1.05/weights/best.onnx"
description= 'wakuwaku~'
# 2. 使用 onnx.load() 将文件加载到内存中，得到模型对象
#    现在，`your_model` 就被定义好了！
try:
    your_model = onnx.load(onnx_file_path)
    print(f"模型 '{onnx_file_path}' 加载成功。")
    print(f"模型的原始IR版本是: {your_model.ir_version}")
    print(f"模型的生产者是: {your_model.producer_name}")
except FileNotFoundError:
    print(f"错误: 找不到文件 '{onnx_file_path}'。请检查路径。")
    exit()
except Exception as e:
    print(f"加载模型时出错: {e}")
    exit()

onnx_version = "1.1.0"
required_dll_version = "1.1.0"

# 清除旧的元数据（可选，但推荐）
while len(your_model.metadata_props) > 0:
    your_model.metadata_props.pop()

# 添加新的元数据
prop1 = StringStringEntryProto()
prop1.key = "model_version"
prop1.value = onnx_version

prop2 = StringStringEntryProto()
prop2.key = "required_dll_version"
prop2.value = required_dll_version

prop3 = StringStringEntryProto()
prop3.key = "author"
prop3.value = "Lf"

prop4 = StringStringEntryProto()
prop4.key = "description"
prop4.value = description

your_model.metadata_props.append(prop1)
your_model.metadata_props.append(prop2)
your_model.metadata_props.append(prop3)
your_model.metadata_props.append(prop4)
# 检查模型并保存
onnx.checker.check_model(your_model)
onnx.save(your_model, description+".onnx")


print("\n当前模型的元数据:")
for prop in your_model.metadata_props:
    print(f"  {prop.key}: {prop.value}")

print(f"模型已保存，版本信息已嵌入。")
```

### 步骤 2: 使用 C++ 读取元数据

1.  确保您的 C++ 项目已正确配置，并链接了 ONNX Runtime 库。
2.  使用 `ReadOnnxMetadataFromFile` 函数来加载模型并提取元数据。
3.  函数会返回一个 `ModelMetadata` 结构体，其中包含了从模型中读取到的信息。

**注意**: C++ 代码演示了使用旧版 `GetCustomMetadataMapKeysAllocated` API 的方法，这在处理需要手动进行内存管理的 ONNX Runtime 版本时非常重要。请务必仔细阅读代码注释中关于内存释放的部分。

#### `reader.cpp` 源码
```cpp
#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

// 假设你定义了这样一个结构体来存放结果
struct ModelMetadata {
    std::string model_version;
    std::string required_dll_version;
    std::string author;
    std::string description;
};

ModelMetadata ReadOnnxMetadataFromFile(std::string model_path)
{
	static Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "LegacyMetadataReader");
	Ort::SessionOptions session_options;
	std::wstring widestr = std::wstring(model_path.begin(), model_path.end());
	const wchar_t* widecstr = widestr.c_str();

	try {
		ModelMetadata result;
		Ort::Session session = Ort::Session(env, widecstr, session_options);
		Ort::ModelMetadata model_metadata = session.GetModelMetadata();

		// 在旧版API中，我们需要一个分配器来分配和释放内存
		Ort::AllocatorWithDefaultOptions allocator;

		// --- 这是与新版API的核心区别 ---
		// 1. 获取所有自定义元数据键的列表
		char** keys = nullptr;
		size_t keys_count = 0;
		// 注意：这个函数会使用分配器来创建 `keys` 数组和其中的每个字符串
		model_metadata.GetCustomMetadataMapKeysAllocated(allocator, &keys, &keys_count);

		// 2. 遍历所有键，并为每个键查找对应的值
		for (size_t i = 0; i < keys_count; ++i) {
			const char* key_cstr = keys[i];
			char* value_cstr = nullptr;

			// 使用键来查找值
			model_metadata.LookupCustomMetadataMapAllocated(key_cstr, allocator, &value_cstr);

			// 将C风格字符串转换为std::string以便于比较
			std::string key_str = key_cstr;
			std::string value_str = value_cstr ? value_cstr : ""; // 安全地处理可能的null值

			// 填充我们的结果结构体
			if (key_str == "model_version") {
				result.model_version = value_str;
			}
			else if (key_str == "required_dll_version") {
				result.required_dll_version = value_str;
			}
			else if (key_str == "author") {
				result.author = value_str;
			}
			else if (key_str == "description") {
				result.description = value_str;
			}

			// 3. [至关重要] 释放为值字符串分配的内存
			if (value_cstr) {
				allocator.Free(value_cstr);
			}
		}

		// 4. [至关重要] 释放为键列表和其中每个键字符串分配的内存
		for (size_t i = 0; i < keys_count; ++i) {
			allocator.Free(keys[i]);
		}
		allocator.Free(keys);

		return result;
	}
	catch (const Ort::Exception& e) {
		throw std::runtime_error("Failed to read ONNX model metadata from '" + model_path + "'. Error: " + e.what());
	}
}
```
*注意：为了使C++代码能够编译，我在 `GetCustomMetadataMapKeysAllocated` 和 `LookupCustomMetadataMapAllocated` 的调用中添加了缺失的参数。请根据您使用的 ONNX Runtime 版本核对 API 的确切签名。*

## 贡献

欢迎对本项目进行贡献！如果您发现任何问题或有改进建议，请随时提交 Pull Request 或创建 Issue。

## 许可证

本项目采用 [MIT 许可证](LICENSE)。
