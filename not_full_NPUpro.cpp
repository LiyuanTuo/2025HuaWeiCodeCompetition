// 78409093分
#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]
// 传输时间范围是[10, 20]
int N, g[11], k[11], m[11], M, latency[11][501], a, b, request_size[11], request_id, delay_parameter[501], size_parameter[501];
short NPU_size[11][11][200001];              // 有极端数据，13.5万是安全的
vector<int> receive_process[11][11][200001]; // 内存储请求的id
pair<int, int> server_timecost[501][11];
int which_gpu[11]; // which_gpu[i]用于表示第i个服务器应该让哪一个gpu处理传送至服务器i的请求

struct Plan
{
    int timej, serverj, NPUj, Bj, process_start, sender;
} plan[150001];       // 至多150000个请求
vector<int> ans[501]; // 这个ans的下标是用户的真实id, 存储请求的id

struct User
{
    int id, s, e, cnt;
    User() : id(0), s(0), e(0), cnt(0) {}
    bool operator<(const User &other)
    {                               // 查完成度之后发现在这个样例下完成度是差不多的
        if (cnt != other.cnt)       // 查完成度，必须查！查完之后发现在这个样例下完成度是差不多的
            return cnt < other.cnt; // 一个-0.991136 一个-0.991093，优化后的完成度甚至还要差一些
        else
            return s < other.s;
    }
} user[501];

void get_argument_initial()
{
    cin >> N;
    for (int i = 1; i <= N; i++)
        cin >> g[i] >> k[i] >> m[i];
    cin >> M;
    for (int i = 1; i <= M; i++)
    {
        cin >> user[i].s >> user[i].e >> user[i].cnt;
        user[i].id = i;
    }
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= M; j++)
        {
            cin >> latency[i][j];
        }
    }
    cin >> a >> b;

    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= g[i]; j++) // 初始化
            for (int k = 0; k <= 200000; k++)
                NPU_size[i][j][k] = m[i]; // 确实应该向下取整，

    for (int i = 1; i <= N; i++)
    {
        request_size[i] = min((m[i] - b) / a, 1000); // 表示应该向第i个服务器的NPU放入多大的样本数量
    }
    for (int i = 1; i <= N; i++)
        which_gpu[i] = 1; // initial which_gpu
}

int request_time(int size, int server, int user)
{
    int id = ::user[user].id;
    return latency[server][id] + (int)ceil(sqrt(size) / k[server]);
}

void sort_server()
{
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= N; j++)
        {
            server_timecost[i][j].second = j; // 这里的i是真实id;
        }
    }
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= N; j++)
        {
            int process_time = (int)ceil(sqrt(request_size[j]) / k[j]);
            int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
            if (process_time <= latency[j][i] + 1)
            {
                server_timecost[i][j].first = Ti * (latency[j][i] + 1) - 1 +
                                              (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
            }
            else
            {
                server_timecost[i][j].first = latency[j][i] + (Ti - 1) * (int)ceil(sqrt(request_size[j]) / k[j]) +
                                              (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
            }
        }
    }
    // cout << "Test\n";
    for (int i = 1; i <= M; i++)
    {
        sort(server_timecost[i] + 1, server_timecost[i] + N + 1);
    }
    sort(user + 1, user + 1 + M);
}

bool check(int mid, int j, int k, int process_start, int time_process)
{
    for (int p = 0; p < time_process; p++)
        if (NPU_size[j][k][process_start + p] < mid * a + b)
            return 0;

    return 1;
}

void solution()
{
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id, Fast_Time = 0x3f3f3f3f;
        vector<Plan> Fast_Solu;

        for (int t = 1; t <= N; t++)
        {
            int j = server_timecost[id][t].second;
            int k = which_gpu[j]; // 临时变量
            for (int _ = 1; _ <= g[j]; _++)
            {
                for (int size_range = 10; size_range <= 100; size_range += 7)
                {
                    for (int delay_range = 1; delay_range <= 2; delay_range += 1)
                    {
                        vector<Plan> solu; // 存储的是请求的id
                        int cnt = user[i].cnt;
                        int timej = user[i].s, process_start = timej + latency[j][id];
                        int Ti_count = 0;
                        while (cnt)
                        {
                            int process_time = (int)ceil(sqrt(request_size[j]) / ::k[j]), future_cost = 0;
                            int Ti = (int)ceil(1.0 * cnt / request_size[j]);
                            if (process_time <= latency[j][id] + 1)
                            {
                                future_cost = Ti * (latency[j][id] + 1) - 1 +
                                              (int)ceil(sqrt(cnt - (Ti - 1) * request_size[j]) / ::k[j]);
                            }
                            else
                            {
                                future_cost = latency[j][id] + (Ti - 1) * (int)ceil(sqrt(request_size[j]) / ::k[j]) +
                                              (int)ceil(sqrt(cnt - (Ti - 1) * request_size[j]) / ::k[j]);
                            }
                            if (timej + future_cost >= Fast_Time)
                            {
                                break; // IDA* 优化
                            }

                            if (Ti_count >= 300)
                                break;
                            int r = min(request_size[j], cnt), l = max(r * size_range / 100, 1); // 确定size的大小
                            int time_process = request_time(l, j, i) - latency[j][id];

                            bool flag = 1;
                            for (int p = 0; p < time_process; p++)
                                if (NPU_size[j][k][process_start + p] < l * a + b)
                                {
                                    flag = 0;
                                    process_start = process_start + p + 1;
                                    break;
                                }

                            if (flag)
                            {
                                while (l < r)
                                {
                                    int mid = l + r + 1 >> 1;
                                    time_process = request_time(mid, j, i) - latency[j][id];
                                    if (check(mid, j, k, process_start, time_process))
                                        l = mid;
                                    else
                                        r = mid - 1;
                                }

                                time_process = request_time(l, j, i) - latency[j][id];
                                // process_start存储的是该NPU开始处理当前这个请求的时间
                                // 也即是服务器接收到这个请求时间的上界 timej + latency[j][id] 是接收到这个请求时间的下界
                                for (int q = 0; q <= time_process - 1; q++)
                                {
                                    NPU_size[j][k][process_start + q] -= l * a + b; // 更新这个NPU_size
                                }

                                int receive_time = timej + latency[j][id];
                                for (int q = timej + latency[j][id]; q <= process_start - 1; q++)
                                    for (int &r : receive_process[j][k][q])
                                    {
                                        // if (plan[r].sender > id)
                                        //     receive_time = q + 1; // 确定receive_time
                                        // else
                                        //     receive_time = q;    //目前来看，这部分影响较小
                                        receive_time = q + 1;
                                    }

                                // receive_time 存储的就是这个NPU收到这个请求的时间
                                // 于是timej就是这个请求正确发送的最早时间，并且再次初始化process_start
                                timej = receive_time - latency[j][id];
                                solu.push_back({timej, j, k, l, process_start, id});
                                timej = timej + latency[j][id] + delay_range; // 准备下一次请求的发送时间，这里可以调参
                                process_start = timej + latency[j][id];       // 准备下一次请求的开始处理的事件时间
                                cnt -= l;
                                Ti_count++;
                                // if (solu.back().process_start + request_time(solu.back().Bj, j, i) - latency[j][id] >= Fast_Time)
                                // {
                                //     break;
                                // }
                            }
                        }
                        // 还原
                        for (Plan &p : solu)
                        {
                            int time_process = request_time(p.Bj, j, i) - latency[j][id];
                            for (int q = 0; q <= time_process - 1; q++)
                                NPU_size[j][k][p.process_start + q] += p.Bj * a + b;
                        }
                        // 更新ans[id]

                        if (cnt == 0 && solu.back().process_start + request_time(solu.back().Bj, j, i) - latency[j][id] < Fast_Time)
                        {
                            Fast_Time = solu.back().process_start + request_time(solu.back().Bj, j, i) - latency[j][id];
                            Fast_Solu = solu;
                            size_parameter[id] = size_range;
                            delay_parameter[id] = delay_range;
                        }
                    }
                }
                k = k % g[j] + 1;
            }
        }
        // 采用最优的方案
        for (Plan j : Fast_Solu)
        {
            request_id++;
            for (int k = j.process_start; k <= j.process_start + request_time(j.Bj, j.serverj, i) - latency[j.serverj][id] - 1; k++)
            {
                NPU_size[j.serverj][j.NPUj][k] -= j.Bj * a + b;
            }
            plan[request_id] = j;
            receive_process[j.serverj][j.NPUj][j.timej + latency[j.serverj][id]].push_back(request_id);
            ans[id].push_back(request_id);
        }
        int serv = Fast_Solu[0].serverj;
        which_gpu[serv] = which_gpu[serv] % g[serv] + 1;
        // cerr << "user : " << i << "finished\n";
    }

    for (int i = 1; i <= M; i++)
    {
        cout << ans[i].size() << "\n"; // Ti must less than 300, there won't cost a problem

        for (int j : ans[i])
            cout << plan[j].timej << " " << plan[j].serverj << " " << plan[j].NPUj << " " << plan[j].Bj << " ";
        cout << "\n";
    }
}

void monitor_NPU_size()
{
    cerr << "not_full_NPUpro:\n\n";
    long long sumsize = 0;
    for (int i = 1; i <= N; i++)
    {
        cerr << "server " << i << " ";
        for (int j = 1; j <= g[i]; j++)
        {
            int sumsize_i_j = 0;
            for (int k = 0; k <= 200000; k++)
            {
                sumsize_i_j += m[i] - NPU_size[i][j][k];
            }
            cerr << "NPU " << j << ": " << sumsize_i_j << " ";
            sumsize += sumsize_i_j;
        }
        cerr << "\n";
    }
    cerr << "\n";
    long long truesumsize = 0;

    sort(user + 1, user + M + 1, [](User &a, User &b)
         { return a.id < b.id; });

    for (int i = 1; i <= M; i++)
    {
        for (int &id : ans[i])
            truesumsize += (plan[id].Bj * a + b) * (request_time(plan[id].Bj, plan[id].serverj, i) - latency[plan[id].serverj][i]);
    }
    // cerr << "sumsize: " << sumsize << "  truesumsize: " << truesumsize << "\n";
    if (sumsize == truesumsize)
    {
        // cerr << "Right\n";
    }
    else
    {
        cerr << "WRONG\n";
    }

    ofstream out("not_full_NPUpro_monitor.txt");
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= g[i]; j++)
        {
            for (int k = 0; k <= 60000; k++)
            {
                out << NPU_size[i][j][k] << " ";
            }
            out << "\n";
        }
    }

    // 这段代码检查任务完成情况
    int latenum = 0;
    for (int i = 1; i <= M; i++)
    {
        Plan &solu = plan[ans[i].back()];
        if (solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] > user[i].e)
            latenum++;
    }
    cerr
        << "latenum: " << latenum << "\n";

    double Score = 0.0, Highest_Score = 0.0; // pow(2, -1.0 * latenum / 100.0);
    // double average = 0;
    for (int i = 1; i <= M; i++)
    {
        Plan &solu = plan[ans[i].back()];
        int endid = solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] - user[i].e;
        Score += pow(2, -1.0 * endid / (user[i].e - user[i].s) / 100.0) * 10000.0;
        Highest_Score += pow(2, 1.0 / 100.0) * 10000.0;
    }
    Score *= pow(2, -1.0 * latenum / 100.0);
    cerr << fixed << setprecision(0) << "Score: " << Score << "\n";
    cerr << fixed << setprecision(0) << "Highest_Score: " << Highest_Score << "\n\n";

    // cout << average / 500.0 << "\n";
}

void parameter_get()
{
    double best_size_range = 0.0, best_delay_range = 0.0;
    for (int i = 1; i <= M; i++)
    {
        cerr << "delay_parameter_" << i << " is " << delay_parameter[i] << endl;
        best_delay_range += delay_parameter[i];
    }
    cerr << endl;
    for (int i = 1; i <= M; i++)
    {
        cerr << "size_parameter_" << i << " is " << size_parameter[i] << endl;
        best_size_range += size_parameter[i];
    }
    cerr << endl;
    cerr << "best_size_parameter is " << best_size_range / M << endl;
    cerr << "best_delay_parameter is " << best_delay_range / M << endl;
}

//-0.992853
int main()
{
    get_argument_initial();
    sort_server();
    solution();
    monitor_NPU_size();
    // parameter_get();
    return 0;
}
// g++ not_full_NPUpro.cpp -std=c++11 -o not_full_NPUpro; get-Content .\sample\extra_data.in | not_full_NPUpro.exe > not_full_NPUpro.txt