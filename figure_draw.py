import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import matplotlib.colors as mcolors
from matplotlib.colors import LinearSegmentedColormap

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

def main():
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
            
            # 创建图表（增大高度）
            plt.figure(figsize=(30, 10), dpi=100)
            
            # 1. 使用颜色编码替代散点图
            # 创建自定义颜色映射：蓝色→黄色→红色
            colors = ["blue", "cyan", "green", "yellow", "orange", "red"]
            cmap = LinearSegmentedColormap.from_list("busy_cmap", colors)
            norm = plt.Normalize(vmin=0, vmax=data.max())
            
            # 2. 绘制颜色编码的时间序列
            x = np.arange(len(data))
            plt.scatter(x, data, c=data, cmap=cmap, norm=norm, s=1, alpha=0.7)
            
            # 3. 添加颜色条
            cbar = plt.colorbar()
            cbar.set_label('空闲度 (高=空闲, 低=繁忙)')
            
            # 4. 标记完全占满点（红色X标记）
            zero_points = np.where(data == 0)[0]
            plt.scatter(zero_points, np.zeros_like(zero_points), 
                        c='red', marker='x', s=20, label='完全占满')
            
            # 5. 添加变化趋势线
            # 每1000点计算一个平均值
            window_size = 1000
            if len(data) > window_size:
                moving_avg = np.convolve(data, np.ones(window_size)/window_size, mode='valid')
                plt.plot(np.arange(window_size-1, len(data)), moving_avg, 
                        'k-', linewidth=2, label=f'{window_size}点移动平均')
            
            # 6. 统计信息
            busy_percent = len(zero_points) / len(data) * 100
            mean_val = np.mean(data)
            
            # 7. 设置图表属性
            plt.title(f'服务器 {server_id + 1} - NPU {npu_id + 1} 繁忙情况\n'
                     f'完全占满率: {busy_percent:.2f}% | 平均空闲度: {mean_val:.1f} | 范围: {data.min()}-{data.max()}')
            plt.xlabel('时间点')
            plt.ylabel('空闲度')
            plt.ylim(-5, data.max() * 1.1)  # 下方留空白
            
            # 添加网格和参考线
            plt.grid(alpha=0.2)
            plt.axhline(y=0, color='gray', linestyle='--', alpha=0.5)
            
            # 添加图例
            plt.legend(loc='upper right')
            
            # 8. 添加统计信息框（移到左下角）
            plt.text(0.02, 0.05, 
                     f'总时间点: {len(data)}\n'
                     f'完全占满点: {len(zero_points)}\n'
                     f'最小空闲度: {data.min()}\n'
                     f'最大空闲度: {data.max()}',
                     transform=plt.gca().transAxes, 
                     bbox=dict(facecolor='white', alpha=0.7))
            
            # 优化X轴显示
            plt.locator_params(axis='x', nbins=20)
            
            # 保存图表
            plt.tight_layout()
            plt.savefig(figure_dir / f'server_{server_id + 1}_npu_{npu_id + 1}.png', dpi=100)
            plt.close()
            
            npu_index += 1
    
    print(f"成功生成 {total_npus} 张图表，已保存至 {figure_dir}/ 目录")

if __name__ == "__main__":
    main()