// 78409093分
#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]
// 传输时间范围是[10, 20]
int N, g[11], k[11], m[11], M, latency[11][501], a, b, server_index[501][11], request_size[11], request_id;
short NPU_size[11][11][200001];              // 有极端数据，13.5万是安全的
vector<int> receive_process[11][11][200001]; // 内存储请求的id

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
}

int request_time(int size, int server, int user)
{
    int id = ::user[user].id;
    return latency[server][id] + (int)ceil(sqrt(size) / k[server]);
}

void sort_server()
{
    for (int i = 1; i <= M; i++) // 而这个i则是用户的唯一 id
    {
        for (int j = 1; j <= N; j++)
        {
            server_index[i][j] = j; // 存的是服务器的下标,也即是哪一个服务器
        }
    }
    // cout << endl << "test" << endl;
    for (int i = 1; i <= M; i++)
    {
        sort(server_index[i] + 1, server_index[i] + N + 1, [i](int server_index1, int server_index2)
             { return double(request_time(request_size[server_index1], server_index1, i)) / request_size[server_index1] <
                      double(request_time(request_size[server_index2], server_index2, i)) / request_size[server_index2]; }); // 按照每个对象块的平均处理时间排序
        // cout << "user_" << i << ":";
        // for (int j = 1; j <= N; j++)
        // {
        //     cout << server_index[i][j] << " ";
        // }
        // cout << endl;
    }

    // for (int i = 1; i <= M; i++)
    // {
    //     for (int j = 1; j <= N; j++)
    //     {
    //         int process_time = (int)ceil(sqrt(request_size[j]) / k[j]);
    //         int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
    //         if (process_time <= latency[j][i] + 1)
    //         {
    //             server_timecost[i][j] = Ti * (latency[j][i] + 1) - 1 +
    //                                     (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
    //         }
    //         else
    //         {
    //             server_timecost[i][j] = latency[j][i] + (Ti - 1) * (int)ceil(sqrt(request_size[j]) / k[j]) +
    //                                     (int)ceil(sqrt(user[i].cnt - (Ti - 1) * request_size[j]) / k[j]);
    //         }
    //     }
    // }
    // cout << "Test\n";
    // for (int i = 1; i <= M; i++)
    // {
    //     cout << "user_" << i << ":";
    //     for (int j = 1; j <= N; j++)
    //     {
    //         cout << server_timecost[i][j] << " ";
    //     }
    //     cout << endl;
    //     sort(server_timecost[i] + 1, server_timecost[i] + N + 1);
    //     cout << "      :";
    //     for (int j = 1; j <= N; j++)
    //     {
    //         cout << server_timecost[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    sort(user + 1, user + 1 + M);

    // for(int i = 1; i <= M; i++)
    // {
    //     cout << "user " << user[i].id << ":  ";
    //     cout << user[i].cnt << "\n";
    // }
    // cout << endl;
    return;
}

bool check(int mid, int j, int k, int process_start, int time_process)
{
    for (int p = 0; p < time_process; p++)
        if (NPU_size[j][k][process_start + p] < mid * a + b)
        {
            return 0;
            // process_start = process_start + p + 1;
            // break;
        }

    return 1;
}

void solution()
{
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id, timej = user[i].s;
        while (user[i].cnt)
        {
            double Fast_effiective = INT_MAX;
            int best_size, best_server, best_npu, pro_time;
            for (int j = 1; j <= N; j++)
            {
                for (int k = 1; k <= g[j]; k++)
                {
                    int process_start = timej + latency[j][id];
                    for (int size = 1; size <= min(request_size[j], user[i].cnt); size++)
                    {
                        bool flag = 0;
                        int time_process = request_time(size, j, i) - latency[j][id];
                        while (flag == 0)
                        {
                            flag = 1;
                            for (int p = 0; p < time_process; p++)
                                if (NPU_size[j][k][process_start + p] < size * a + b)
                                {
                                    flag = 0;
                                    process_start = process_start + p + 1;
                                    break;
                                }
                        }

                        if (1.0 * ((ans[id].size() != 0) + process_start + time_process - timej) / pow(1.0 * size, 1.8) < Fast_effiective)
                        {
                            Fast_effiective = 1.0 * (process_start + time_process - timej) / pow(1.0 * size, 1.8);
                            best_size = size, best_server = j, best_npu = k, pro_time = process_start;
                        }
                        else if(1.0 * ((ans[id].size() != 0) + process_start + time_process - timej) / pow(1.0 * size, 1.8) == Fast_effiective && size > best_size)
                        {
                            best_size = size, best_server = j, best_npu = k, pro_time = process_start;
                        }
                    }
                }
            }

            int time_process = request_time(best_size, best_server, i) - latency[best_server][id];
            for (int q = 0; q <= time_process - 1; q++)
            {
                NPU_size[best_server][best_npu][pro_time + q] -= best_size * a + b; // 更新这个NPU_size
            }

            int receive_time = timej + latency[best_server][id];
            for (int q = timej + latency[best_server][id]; q <= pro_time - 1; q++)
                for (int &r : receive_process[best_server][best_npu][q])
                {
                    // if (plan[r].sender > id)
                    //     receive_time = q + 1; // 确定receive_time
                    // else
                    //     receive_time = q;    //目前来看，这部分影响较小
                    receive_time = q + 1;
                }

            timej = receive_time - latency[best_server][id];
            request_id++;
            plan[request_id] = {timej, best_server, best_npu, best_size, pro_time, id};
            receive_process[best_server][best_npu][receive_time].push_back(request_id);
            ans[id].push_back(request_id); // 不过这里可能会有bug， 发送的次数可能会超过300, TO_DO

            timej = timej + latency[best_server][id] + 1; // 准备下一次请求的发送时间，这里可以调参
            user[i].cnt -= best_size;
        }

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
    ofstream out("Migrate_Optimize_monitor.txt");
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
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id;
        Plan &solu = plan[ans[id].back()];
        int endid = solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][id] - user[i].e;
        out << endid << "\n";
    }
    cerr << "Migrate_Optimize:\n\n";
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
        int move = 0;
        for (int j = 1; j <= ans[i].size() - 1; j++)
        {
            int temp_server = plan[ans[i][j]].serverj, last_server = plan[ans[i][j - 1]].serverj;
            int temp_npu = plan[ans[i][j]].NPUj, last_npu = plan[ans[i][j - 1]].NPUj;            
            if(temp_server != last_server || temp_npu != last_npu)
                move++;
        }
        Plan &solu = plan[ans[i].back()];
        int endid = solu.process_start + request_time(solu.Bj, solu.serverj, i) - latency[solu.serverj][i] - user[i].e;
        Score += pow(2, -1.0 * endid / (user[i].e - user[i].s) / 100.0) * 10000.0 * pow(2, -1.0 * move / 200.0);
        // average += 1.0 * endid / (user[i].e - user[i].s);
        Highest_Score += pow(2, 1.0 / 100.0) * 10000.0;
    }
    Score *= pow(2, -1.0 * latenum / 100.0);
    cerr << fixed << setprecision(0) << "Score: " << Score << "\n";
    cerr << fixed << setprecision(0) << "Highest_Score: " << Highest_Score << "\n\n";
    
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