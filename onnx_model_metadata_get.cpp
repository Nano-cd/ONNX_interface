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
		model_metadata.GetCustomMetadataMapKeysAllocated(allocator);

		// 2. 遍历所有键，并为每个键查找对应的值
		for (size_t i = 0; i < keys_count; ++i) {
			const char* key_cstr = keys[i];
			char* value_cstr = nullptr;

			// 使用键来查找值
			model_metadata.LookupCustomMetadataMapAllocated(key_cstr, allocator);

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
