// 78409093分
#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]
// 传输时间范围是[10, 20]
int N, g[11], k[11], m[11], M, latency[11][501], a, b, request_size[11], request_id;
short NPU_size[11][11][200001];              // 有极端数据，13.5万是安全的
vector<int> receive_process[11][11][200001]; // 内存储请求的id
int which_gpu[11];                           // which_gpu[i]用于表示第i个服务器应该让哪一个gpu处理传送至服务器i的请求
pair<int, int> server_timecost[501][11];
struct Plan
{
    int timej, serverj, NPUj, Bj, process_start, sender;
} plan[80000];        // 至多80000个请求
vector<int> ans[501]; // 这个ans的下标是用户的真实id, 存储请求的id

struct User
{
    int id, s, e, cnt;
    User() : id(0), s(0), e(0), cnt(0) {}
    bool operator<(const User &other)
    { // 查完成度之后发现在这个样例下完成度是差不多的
        // return cnt + 0.08 * s < other.cnt + 0.08 * other.s;

        // if (cnt * double(Request_Time(server_index[id][1], id)) / request_size[server_index[id][1]] != other.cnt * double(Request_Time(server_index[other.id][1], other.id)) / request_size[server_index[other.id][1]])
        //     return cnt * double(Request_Time(server_index[id][1], id)) / request_size[server_index[id][1]] < other.cnt * double(Request_Time(server_index[other.id][1], other.id)) / request_size[server_index[other.id][1]];
        // else
        //     return s < other.s;
        if (cnt != other.cnt)       // 查完成度，必须查！查完之后发现在这个样例下完成度是差不多的
            return cnt < other.cnt; // 一个-0.991136 一个-0.991093，优化后的完成度甚至还要差一些
        else
            return s < other.s;
        // if(server_timecost[id][1] != server_timecost[other.id][1])
        //     return server_timecost[id][1] < server_timecost[other.id][1];
        // else
        //     return s < other.s;
        //    return 1.0 * cnt / (1.0 * e - 1.0 * s) < 1.0 * other.cnt / (1.0 * other.e - 1.0 * other.s);
        // if (s != other.s)
        //     return s < other.s;
        // else
        //     return cnt < other.cnt;
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
        // cout << "user_" << i << ":";
        // for (int j = 1; j <= N; j++)
        // {
        //     cout << server_timecost[i][j] << " ";
        // }
        // cout << endl;
        sort(server_timecost[i] + 1, server_timecost[i] + N + 1);
        // cout << "      :";
        // for (int j = 1; j <= N; j++)
        // {
        //     cout << server_timecost[i][j] << " ";
        // }
        // cout << endl;
    }

    sort(user + 1, user + 1 + M);
    return;
}

bool check(int mid, int server, int npu, int process_start, int time_process)
{
    for (int p = 0; p < time_process; p++)
        if (NPU_size[server][npu][process_start + p] < mid * a + b)
        {
            return 0;
            // process_start = process_start + p + 1;
            // break;
        }

    return 1;
}

int Fast_Time = 0x3f3f3f3f;
vector<Plan> Fast_Solu;
void dfs(int server, int npu, int i, int timej, int cnt, vector<Plan> &solu)
{
    int id = user[i].id;
    if (Fast_Time <= user[i].e)
        return;            // 看来这条优化暂时还不能省啊！
    if (solu.size() > 300) // Ti 超过300 直接返回
        return;
    if (cnt == 0)
    {
        if (solu.back().process_start + (int)ceil(sqrt(solu.back().Bj) / k[server]) <= Fast_Time)
        {
            Fast_Time = solu.back().process_start + (int)ceil(sqrt(solu.back().Bj) / k[server]);
            Fast_Solu = solu;
        }
        return; // 搜索结束，递归边界
    }

    int process_time = (int)ceil(sqrt(request_size[server]) / k[server]), future_cost = 0;
    int Ti = (int)ceil(1.0 * cnt / request_size[server]);
    if (process_time <= latency[server][id] + 1)
    {
        future_cost = Ti * (latency[server][id] + 1) - 1 +
                      (int)ceil(sqrt(cnt - (Ti - 1) * request_size[server]) / k[server]);
    }
    else
    {
        future_cost = latency[server][id] + (Ti - 1) * (int)ceil(sqrt(request_size[server]) / k[server]) +
                      (int)ceil(sqrt(cnt - (Ti - 1) * request_size[server]) / k[server]);
    }
    if (timej + future_cost > Fast_Time)
    {
        return; // IDA* 优化
    }

    // if (!solu.empty())
    //     if (solu.back().process_start + (int)ceil(sqrt(solu.back().Bj) / k[server]) >= Fast_Time)
    //         return; // 最优性剪枝

    // if (server == 5 && npu == 1 && id == 241)
    // {
    //     ofstream out("output.txt", ios::app);
    //     out << "timej   cnt\n"
    //         << timej << "    " << cnt << "    " << dep << "\n";
    // }
    bool flag = 0;
    int process_start = timej + latency[server][id], time_process;
    int r = min(request_size[server], cnt), l = max(r / 4, 1); // 确定size的大小
    time_process = request_time(l, server, i) - latency[server][id];
    while (flag == 0)
    {
        flag = 1;
        for (int p = 0; p < time_process; p++)
            if (NPU_size[server][npu][process_start + p] < l * a + b)
            {
                flag = 0;
                process_start = process_start + p + 1;
                break;
            }
    }

    // while (NPU_size[server][npu][process_start] < a + b)
    // {
    //     process_start++; // 找到一个能处理请求的地方
    // }

    while (l < r)
    {
        int mid = l + r + 1 >> 1;
        time_process = (int)ceil(sqrt(mid) / k[server]);
        if (check(mid, server, npu, process_start, time_process))
            l = mid;
        else
            r = mid - 1; // 确定size的大小
    }
    time_process = (int)ceil(sqrt(l) / k[server]);

    for (int q = 0; q <= time_process - 1; q++)
        NPU_size[server][npu][process_start + q] -= l * a + b; // 更新这个NPU_size

    int receive_time = timej + latency[server][id];
    for (int q = timej + latency[server][id]; q <= process_start - 1; q++)
        for (int &r : receive_process[server][npu][q])
        {
            // if (plan[r].sender > id)
            //     receive_time = q + 1; // 确定receive_time
            // else
            //     receive_time = q;    //目前来看，这部分影响较小
            receive_time = q + 1;
        }

    solu.push_back({receive_time - latency[server][id], server, npu, l, process_start, id});
    dfs(server, npu, i, receive_time + 1, cnt - l, solu); // 第一种情况
    // 还原
    solu.pop_back();
    for (int q = 0; q <= time_process - 1; q++)
        NPU_size[server][npu][process_start + q] += l * a + b;

    // 第二种情况, 看能不能跳过当前这个时间片
    flag = 0;
    process_start++;
    int r2 = min(request_size[server], cnt), l2 = max(r2 / 4, 1); // 确定size的大小
    time_process = request_time(l2, server, i) - latency[server][id];
    while (flag == 0)
    {
        flag = 1;
        for (int p = 0; p < time_process; p++)
            if (NPU_size[server][npu][process_start + p] < l2 * a + b)
            {
                flag = 0;
                process_start = process_start + p + 1;
                break;
            }
    }

    while (l2 < r2)
    {
        int mid = l2 + r2 + 1 >> 1;
        time_process = (int)ceil(sqrt(mid) / k[server]);
        if (check(mid, server, npu, process_start, time_process))
            l2 = mid;
        else
            r2 = mid - 1; // 确定size的大小
    }

    if (l2 > l)
    {
        time_process = (int)ceil(sqrt(l2) / k[server]);

        for (int q = 0; q <= time_process - 1; q++)
            NPU_size[server][npu][process_start + q] -= l2 * a + b; // 更新这个NPU_size

        receive_time = timej + latency[server][id];
        for (int q = timej + latency[server][id]; q <= process_start - 1; q++)
            for (int &r : receive_process[server][npu][q])
            {
                receive_time = q + 1;
            }

        solu.push_back({receive_time - latency[server][id], server, npu, l2, process_start, id});
        dfs(server, npu, i, receive_time + 1, cnt - l2, solu); // 不严谨！！！！

        solu.pop_back();
        for (int q = 0; q <= time_process - 1; q++)
            NPU_size[server][npu][process_start + q] += l2 * a + b; // 还原
    }

    // timej = timej + latency[j][id] + 1; // 准备下一次请求的发送时间，这里可以调参
}

void solution()
{
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id;
        Fast_Time = 0x3f3f3f3f;
        Fast_Solu.clear();

        for (int t = 1; t <= N; t++)
        {
            int j = server_timecost[id][t].second;
            int k = which_gpu[j];           // 临时变量
            for (int _ = 1; _ <= g[j]; _++) // 这里主要起个计数器的作用
            {
                vector<Plan> solu; // 存储的是请求的id   5 1炸了
                dfs(j, k, i, user[i].s, user[i].cnt, solu);
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

        cerr << "user : " << i << "finished\n";
    }

    for (int i = 1; i <= M; i++)
    {
        cout << ans[i].size() << "\n"; // Ti must less than 300, there won't cost a problem

        for (int j : ans[i])
            cout << plan[j].timej << " " << plan[j].serverj << " " << plan[j].NPUj << " " << plan[j].Bj << " ";
        // for (int j = 1; j <= ans[i].size() - 1; j++)
        // {                       danger ans[i].size() - 1
        //     if (ans[i][j].timej - ans[i][j - 1].timej >= latency[ans[i][j - 1].serverj][i] + 1)
        //         cout << "OK " << latency[ans[i][j - 1].serverj][i] + 1  << " ";
        //     else cout <<"Wrong ";
        // }
        cout << "\n";
    }
}

void monitor_NPU_size()
{
    cerr << "search_optimize:\n\n";
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
        cerr << "Right\n";
    }
    else
    {
        cerr << "WRONG\n";
    }

    ofstream out("search_optimize_monitor.txt");
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= g[i]; j++)
        {
            for (int k = 0; k <= 100000; k++)
            {
                out << NPU_size[i][j][k] << " ";
            }
            out << "\n";
        }
    }
    out.close();

    // 这段代码检查任务完成情况
    int latenum = 0;
    for (int i = 1; i <= M; i++)
    {
        Plan &solu = plan[ans[i].back()];
        if (solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] > user[i].e)
            latenum++;
    }
    cerr << "\n"
         << "latenum: " << latenum << "\n";

    double Score = 0.0, Highest_Score = 0.0; // pow(2, -1.0 * latenum / 100.0);
    // double average = 0;
    for (int i = 1; i <= M; i++)
    {
        Plan &solu = plan[ans[i].back()];
        int endid = solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] - user[i].e;
        Score += pow(2, -1.0 * endid / (user[i].e - user[i].s) / 100.0) * 10000.0;
        // average += 1.0 * endid / (user[i].e - user[i].s);
        Highest_Score += pow(2, 1.0 / 100.0) * 10000.0;
    }
    Score *= pow(2, -1.0 * latenum / 100.0);
    cerr << fixed << setprecision(0) << "Score: " << Score << "\n";
    cerr << fixed << setprecision(0) << "Highest_Score: " << Highest_Score << "\n";

    // cout << average / 500.0 << "\n";
}
//-0.992853
int main()
{
    // ofstream out("output.txt");
    // out.close();
    srand(6);
    get_argument_initial();
    sort_server();
    solution();

    monitor_NPU_size();
    return 0;
}
// g++ search_optimize.cpp -std=c++11 -o search_optimize; get-Content .\sample\data.in | search_optimize.exe > search_optimize.txt