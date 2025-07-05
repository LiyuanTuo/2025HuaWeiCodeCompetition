// 78409093分
#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]
// 传输时间范围是[10, 20]
int N, g[11], k[11], m[11], M, latency[11][501], a, b, server_timecost[501][11], request_size[11], request_id;
short NPU_size[11][11][135001];              // 有极端数据，13.5万是安全的
vector<int> receive_process[11][11][135001]; // 内存储请求的id
int which_gpu[11];                           // which_gpu[i]用于表示第i个服务器应该让哪一个gpu处理传送至服务器i的请求

struct Plan
{
    int timej, serverj, NPUj, Bj, process_start, sender;
} plan[80000];        // 至多80000个请求
vector<int> ans[501]; // 这个ans的下标是用户的真实id, 存储请求的id
// int Request_Time(int server, int user)
// {
//     int size = request_size[server];
//     return latency[server][user] + (int)ceil(sqrt(size) / k[server]);
// }
struct User
{
    int id, s, e, cnt;
    User() : id(0), s(0), e(0), cnt(0) {}
    bool operator<(const User &other)
    { // 查完成度之后发现在这个样例下完成度是差不多的
        // return cnt + 0.08 * s < other.cnt + 0.08 * other.s;

        // if (cnt * double(Request_Time(server_index[id][0], id)) / request_size[server_index[id][0]] != other.cnt * double(Request_Time(server_index[other.id][0], other.id)) / request_size[server_index[other.id][0]])
        //     return cnt * double(Request_Time(server_index[id][0], id)) / request_size[server_index[id][0]] < other.cnt * double(Request_Time(server_index[other.id][0], other.id)) / request_size[server_index[other.id][0]];
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
            for (int k = 0; k <= 135000; k++)
                NPU_size[i][j][k] = (m[i] - b) / a; // 确实应该向下取整，

    for (int i = 1; i <= N; i++)
    {
        request_size[i] = min(NPU_size[i][1][1], short(1000)); // 表示应该向第i个服务器的NPU放入多大的样本数量
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

    // for (int i = 1; i <= M; i++) // 而这个i则是用户的唯一 id
    // {
    //     for (int j = 1; j <= N; j++)
    //     {
    //         server_index[i][j] = j; // 存的是服务器的下标,也即是哪一个服务器
    //     }
    // }
    // // cout << endl << "test" << endl;
    // for (int i = 1; i <= M; i++)
    // {
    //     sort(server_index[i] + 1, server_index[i] + N + 1, [i](int server_index1, int server_index2)
    //          { return double(request_time(request_size[server_index1], server_index1, i)) / request_size[server_index1] <
    //                   double(request_time(request_size[server_index2], server_index2, i)) / request_size[server_index2]; }); // 按照每个对象块的平均处理时间排序
    //     // cout << "user_" << i << ":";
    //     // for (int j = 1; j <= N; j++)
    //     // {
    //     //     cout << server_index[i][j] << " ";
    //     // }
    //     // cout << endl;
    // }

    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= N; j++)
        {
            int process_time = (int)ceil(sqrt(request_size[j]) / k[j]);
            int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
            if (process_time <= latency[j][i] + 1)
            {
                server_timecost[i][j] = Ti * (latency[j][i] + 1) - 1 +
                                        (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
            }
            else
            {
                server_timecost[i][j] = latency[j][i] + (Ti - 1) * (int)ceil(sqrt(request_size[j]) / k[j]) +
                                        (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
            }
        }
    }
    //cout << "Test\n";
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

    // for(int i = 1; i <= M; i++)
    // {
    //     cout << "user " << user[i].id << ":  ";
    //     cout << server_timecost[user[i].id][1] << "\n";
    // }
    // cout << endl;
    return;
}

void solution()
{
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id, Fast_Time = 0x3f3f3f3f;
        vector<Plan> Fast_Solu;

        for (int j = 1; j <= N; j++)
        {
            int k = which_gpu[j];
            // int k = rand() % g[j] + 1;
            int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
            vector<Plan> solu; // 存储的是请求的id
            int cnt = user[i].cnt;
            int timej = user[i].s, process_start = timej + latency[j][id];
            int count = 0;
            while (count != Ti)
            {
                int size = min(request_size[j], cnt);
                int time_process = request_time(size, j, i) - latency[j][id];

                bool flag = 1;
                for (int p = 0; p < time_process; p++)
                    if (NPU_size[j][k][process_start + p] < size)
                    {
                        flag = 0;
                        process_start = process_start + p + 1;
                        break;
                    }

                if (flag)
                { // process_start存储的是该NPU开始处理当前这个请求的时间
                    // 也即是服务器接收到这个请求时间的上界 timej + latency[j][id] 是接收到这个请求时间的下界
                    for (int q = 0; q <= time_process - 1; q++)
                    {
                        NPU_size[j][k][process_start + q] -= size; // 更新这个NPU_size
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
                    solu.push_back({timej, j, k, size, process_start, id});
                    timej = timej + latency[j][id] + 1;     // 准备下一次请求的发送时间
                    process_start = timej + latency[j][id]; // 准备下一次请求的开始处理的事件时间
                    count++;
                    cnt -= size;
                }
            }
            // 更新ans[id]
            if (solu.back().process_start + request_time(solu.back().Bj, j, i) - latency[j][id] <= Fast_Time)
            {
                Fast_Time = solu.back().process_start + request_time(solu.back().Bj, j, i) - latency[j][id];
                Fast_Solu = solu;
            }
            // 还原

            for (Plan &p : solu)
            {
                int time_process = request_time(p.Bj, j, i) - latency[j][id];
                for (int q = 0; q <= time_process - 1; q++)
                    NPU_size[j][k][p.process_start + q] += p.Bj;
            }
        }
        // 采用最优的方案
        for (Plan j : Fast_Solu)
        {
            request_id++;
            for (int k = j.process_start; k <= j.process_start + request_time(j.Bj, j.serverj, i) - latency[j.serverj][id] - 1; k++)
            {
                NPU_size[j.serverj][j.NPUj][k] -= j.Bj;
                // cout << "";
            }
            plan[request_id] = j;
            receive_process[j.serverj][j.NPUj][j.timej + latency[j.serverj][id]].push_back(request_id);
            ans[id].push_back(request_id);
        }
        Plan temp = Fast_Solu[0];
        which_gpu[temp.serverj] = which_gpu[temp.serverj] % g[temp.serverj] + 1;
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
    cout << "\n\n\n";
    int sumsize = 0;
    for (int i = 1; i <= N; i++)
    {
        cout << "server " << i << " ";
        for (int j = 1; j <= g[i]; j++)
        {
            int sumsize_i_j = 0;
            for (int k = 0; k <= 135000; k++)
            {
                sumsize_i_j += (m[i] - b) / a - NPU_size[i][j][k];
            }
            cout << "NPU " << j << ": " << sumsize_i_j << " ";
            sumsize += sumsize_i_j;
        }
        cout << "\n";
    }
    cout << "\n";
    int truesumsize = 0;

    sort(user + 1, user + M + 1, [](User &a, User &b)
         { return a.id < b.id; });

    for (int i = 1; i <= M; i++)
    {
        for (int &id : ans[i])
            truesumsize += plan[id].Bj * (request_time(plan[id].Bj, plan[id].serverj, i) - latency[plan[id].serverj][i]);
    }
    cout << "sumsize: " << sumsize << "  truesumsize: " << truesumsize << "\n\n\n";
    if (sumsize == truesumsize)
    {
        cout << "Right\n";
    }
    else
    {
        cout << "WRONG\n";
    }

    // ofstream out("monitor.txt");
    // for (int i = 1; i <= N; i++)
    // {
    //     for (int j = 1; j <= g[i]; j++)
    //     {
    //         for (int k = 0; k <= 5000; k++)
    //         {
    //             out << NPU_size[i][j][k] << " ";
    //         }
    //         out << "\n";
    //     }
    // }
    // out.close();

    // 这段代码检查任务完成情况
    int latenum = 0;
    for (int i = 1; i <= M; i++)
    {
        Plan &solu = plan[ans[i].back()];
        if (solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] > user[i].e)
            latenum++;
    }
    cout << "\n\n\n"
         << "latenum: " << latenum << "\n\n\n";

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
    cout << fixed << setprecision(0) << "Score: " << Score << "\n";
    cout << fixed << setprecision(0) << "Highest_Score: " << Highest_Score << "\n";

    // cout << average / 500.0 << "\n";
}

//-0.992853
int main()
{
    srand(6);
    get_argument_initial();
    sort_server();
    solution();

    monitor_NPU_size();
    return 0;
}
// g++ main.cpp -std=c++11 -o main; get-Content .\sample\data.in | main.exe > output.txt