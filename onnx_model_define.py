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
