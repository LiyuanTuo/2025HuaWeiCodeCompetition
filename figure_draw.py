import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

# 配置中文及负号
plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

# 创建存储图片的文件夹
dir_fig = Path("figure")
dir_fig.mkdir(parents=True, exist_ok=True)

# 读取服务器配置信息
def read_config(path):
    with open(path, 'r') as f:
        n = int(f.readline())
        g = []
        for _ in range(n):
            gi, ki, mi = map(int, f.readline().split())
            g.append(gi)
    return g

# 读取监控数据
def read_monitor(path):
    data = []
    with open(path, 'r') as f:
        for line in f:
            data.append(np.fromstring(line, dtype=int, sep=' '))
    return data


def plot_npu_scatter(g_list, monitor_data):
    total = sum(g_list)
    if len(monitor_data) != total:
        raise ValueError(f"数据行 {len(monitor_data)} 与 NPU 数 {total} 不匹配！")

    idx = 0
    for srv, cnt in enumerate(g_list, start=1):
        for npu in range(1, cnt + 1):
            arr = monitor_data[idx]
            x = np.arange(arr.size)

            # 更细致散点图：小点、高分辨率
            plt.figure(figsize=(12, 6), dpi=200)
            plt.scatter(x, arr, s=2 , alpha=0.6)
            plt.title(f"服务器 {srv} - NPU {npu} 繁忙散点图（细化）", fontsize=14)
            plt.xlabel("时间点", fontsize=12)
            plt.ylabel("空闲度 (0=饱和)", fontsize=12)
            plt.grid(linestyle=':', alpha=0.3)

            # 优化刻度：更多分段展示微变动
            plt.locator_params(axis='x', nbins=25)
            plt.locator_params(axis='y', nbins=25)

            out = dir_fig / f"server_{srv}_npu_{npu}_detail.png"
            plt.tight_layout()
            plt.savefig(out)
            plt.close()
            idx += 1
    print(f"已生成 {total} 张精细散点图，保存在 {dir_fig}/")


if __name__ == '__main__':
    cfg = read_config(Path("sample/extra_data.in"))
    mon = read_monitor(Path("test_monitor.txt"))
    plot_npu_scatter(cfg, mon)
