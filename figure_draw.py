import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

# 创建存储图片的文件夹
figure_dir = Path("figure")
figure_dir.mkdir(parents=True, exist_ok=True)

# 读取服务器配置信息
config_path = Path("./sample/data.in")
with open(config_path, 'r') as f:
    n = int(f.readline().strip())
    g = []  # 存储每个服务器的NPU数量
    for _ in range(n):
        gi, ki, mi = map(int, f.readline().split())
        g.append(gi)

# 读取监控数据
monitor_path = Path("monitor.txt")
with open(monitor_path, 'r') as f:
    monitor_data = []
    for line in f:
        data = list(map(int, line.split()))
        monitor_data.append(data)

# 验证数据完整性
total_npus = sum(g)
if len(monitor_data) != total_npus:
    raise ValueError(f"监控数据不匹配！应有{total_npus}个NPU，实际有{len(monitor_data)}行数据")

# 为每个NPU生成可视化图表
npu_index = 0
for server_id in range(n):
    for npu_id in range(g[server_id]):
        data = np.array(monitor_data[npu_index])
        
        # 创建更大的图表（宽度30英寸，高度12英寸）
        fig = plt.figure(figsize=(30, 12), dpi=100)
        ax_main = plt.gca()  # 获取主坐标轴
        
        # 1. 分离空闲点和繁忙点
        indices = np.arange(len(data))
        
        # 找出空闲点 (值 > 0)
        free_points = indices[data > 0]
        free_values = data[data > 0]
        
        # 找出繁忙点 (值 == 0)
        busy_points = indices[data == 0]
        
        # 2. 绘制空闲点 - 使用更大的点
        ax_main.scatter(free_points, free_values, 
                        c='blue', s=3, alpha=0.5, 
                        label='空闲点')
        
        # 3. 绘制繁忙点 - 使用更大的X标记
        ax_main.scatter(busy_points, np.zeros_like(busy_points), 
                        c='red', marker='x', s=25, 
                        label='完全占满')
        
        # 4. 添加变化趋势线（更平滑）
        # 每2000点计算一个平均值
        window_size = 2000
        if len(data) > window_size:
            moving_avg = np.convolve(data, np.ones(window_size)/window_size, mode='valid')
            ax_main.plot(np.arange(window_size-1, len(data)), moving_avg, 
                        'k-', linewidth=3, alpha=0.8,
                        label=f'{window_size}点移动平均')
        
        # 5. 添加关键参考线
        ax_main.axhline(y=0, color='gray', linestyle='-', linewidth=1.5, alpha=0.7)
        median_val = np.median(data)
        ax_main.axhline(y=median_val, color='green', linestyle='--', linewidth=1.5, 
                        alpha=0.7, label=f'中位数: {median_val:.1f}')
        
        # 6. 统计信息
        busy_percent = len(busy_points) / len(data) * 100
        mean_val = np.mean(data)
        
        # 7. 设置图表属性
        ax_main.set_title(f'服务器 {server_id + 1} - NPU {npu_id + 1} 繁忙情况\n'
                        f'完全占满率: {busy_percent:.2f}% | 平均空闲度: {mean_val:.1f} | 范围: {data.min()}-{data.max()}',
                        fontsize=16)
        ax_main.set_xlabel('时间点', fontsize=14)
        ax_main.set_ylabel('空闲度', fontsize=14)
        ax_main.set_ylim(-5, data.max() * 1.1)  # 下方留空白
        
        # 添加网格
        ax_main.grid(alpha=0.3, linestyle=':')
        
        # 添加图例（更大字体）
        ax_main.legend(loc='upper right', fontsize=12)
        
        # 8. 添加统计信息框（移到右上角）
        stats_text = (f'总时间点: {len(data)}\n'
                        f'完全占满点: {len(busy_points)}\n'
                        f'最小空闲度: {data.min()}\n'
                        f'最大空闲度: {data.max()}\n'
                        f'中位空闲度: {median_val:.1f}\n'
                        f'标准差: {np.std(data):.1f}')
        
        ax_main.text(0.98, 0.95, stats_text,
                    transform=ax_main.transAxes, 
                    bbox=dict(facecolor='white', alpha=0.8, edgecolor='gray'),
                    horizontalalignment='right', verticalalignment='top',
                    fontsize=12)
        
        # 9. 添加负载分布直方图（内嵌）- 使用正确的坐标转换
        ax_hist = fig.add_axes([0.75, 0.15, 0.2, 0.2])
        ax_hist.hist(data, bins=50, color='blue', alpha=0.7)
        ax_hist.set_title('空闲度分布', fontsize=10)
        ax_hist.set_xlabel('空闲度')
        ax_hist.set_ylabel('频次')
        ax_hist.grid(alpha=0.2)
        
        # 优化X轴显示
        ax_main.locator_params(axis='x', nbins=20)
        ax_main.locator_params(axis='y', nbins=15)
        
        # 保存图表 - 使用bbox_inches='tight'确保所有元素包含在内
        plt.savefig(figure_dir / f'server_{server_id + 1}_npu_{npu_id + 1}.png', 
                    dpi=100, bbox_inches='tight')
        plt.close(fig)
        
        npu_index += 1

print(f"成功生成 {total_npus} 张图表，已保存至 {figure_dir}/ 目录")
