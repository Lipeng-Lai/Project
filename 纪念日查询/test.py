import pandas as pd

# 读取文本文件，每两行为一组数据
with open('test.txt', 'r', encoding='utf-8') as file:
    lines = file.readlines()


# 拆分数据为两列
dates = [line.strip() for i, line in enumerate(lines) if i % 2 == 1]
days = [line.strip('天\n') for i, line in enumerate(lines) if i % 2 == 0]

# 创建DataFrame
data = {'天数': days, '日期': dates}
df = pd.DataFrame(data)

# 保存为Excel文件
df.to_excel('Excel.xlsx', index=False)
